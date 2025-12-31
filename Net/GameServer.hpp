#ifndef TITAN_GAME_SERVER_HPP
#define TITAN_GAME_SERVER_HPP

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <vector>
#include <deque>
#include <algorithm>
#include "NetTypes.hpp"

#pragma comment(lib, "ws2_32.lib")

namespace Titan {
namespace Net {

enum class ServerPacketType : uint8_t {
    Connect = 1,
    Disconnect = 2,
    Snapshot = 3,
    Ping = 4,
    Pong = 5
};

enum class ClientPacketType : uint8_t {
    Connect = 1,
    Disconnect = 2,
    Input = 3,
    Ping = 4
};

struct ConnectedClient {
    uint32_t id = 0;
    sockaddr_in addr = {};
    float lastPacketTime = 0;
    bool connected = false;
    std::deque<PlayerInput> inputQueue;
    PlayerState state;
    uint32_t lastAck = 0;
};

class GameServer {
public:
    bool Start(uint16_t port) {
        m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_socket == INVALID_SOCKET) return false;
        
        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(m_socket, (sockaddr*)&addr, sizeof(addr)) != 0) {
            closesocket(m_socket);
            return false;
        }
        
        u_long mode = 1;
        ioctlsocket(m_socket, FIONBIO, &mode);
        
        int rcvBuf = 1024 * 1024;
        setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuf, sizeof(rcvBuf));
        int sndBuf = 1024 * 1024;
        setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&sndBuf, sizeof(sndBuf));
        
        QueryPerformanceFrequency(&m_freq);
        QueryPerformanceCounter(&m_lastTime);
        
        m_running = true;
        m_tick = 0;
        m_time = 0;
        m_accumulator = 0;
        m_port = port;
        m_nextClientId = 1;
        
        printf("[Server] Started on port %d (128 tick)\n", port);
        return true;
    }
    
    void Stop() {
        if (!m_running) return;
        
        uint8_t pkt[2] = {(uint8_t)ServerPacketType::Disconnect, 0};
        for (auto& c : m_clients) {
            if (c.connected) {
                sendto(m_socket, (char*)pkt, 2, 0, (sockaddr*)&c.addr, sizeof(c.addr));
            }
        }
        
        closesocket(m_socket);
        m_clients.clear();
        m_running = false;
        printf("[Server] Stopped\n");
    }
    
    void Run() {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        float frameTime = (float)(now.QuadPart - m_lastTime.QuadPart) / m_freq.QuadPart;
        m_lastTime = now;
        
        if (frameTime > 0.25f) frameTime = 0.25f;
        
        m_accumulator += frameTime;
        
        ReceivePackets();
        
        while (m_accumulator >= TICK_INTERVAL) {
            Tick();
            m_accumulator -= TICK_INTERVAL;
        }
        
        m_snapshotTimer += frameTime;
        if (m_snapshotTimer >= SNAPSHOT_INTERVAL) {
            m_snapshotTimer -= SNAPSHOT_INTERVAL;
            BroadcastSnapshot();
        }
        
        m_cleanupTimer += frameTime;
        if (m_cleanupTimer >= 1.0f) {
            m_cleanupTimer = 0;
            CleanupClients();
        }
    }
    
    int GetPlayerCount() const {
        int count = 0;
        for (auto& c : m_clients) if (c.connected) count++;
        return count;
    }
    
    bool IsRunning() const { return m_running; }
    uint32_t GetTick() const { return m_tick; }
    float GetTime() const { return m_time; }

private:
    void ReceivePackets() {
        uint8_t buffer[1500];
        sockaddr_in from;
        int fromLen = sizeof(from);
        
        int maxPackets = 1000;
        while (maxPackets-- > 0) {
            int received = recvfrom(m_socket, (char*)buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromLen);
            if (received <= 0) break;
            
            ProcessPacket(buffer, received, from);
        }
    }
    
    void ProcessPacket(uint8_t* data, int size, const sockaddr_in& from) {
        if (size < 1) return;
        
        ClientPacketType type = (ClientPacketType)data[0];
        
        ConnectedClient* client = FindClient(from);
        
        switch (type) {
            case ClientPacketType::Connect:
                if (!client && m_clients.size() < MAX_PLAYERS) {
                    ConnectedClient newClient;
                    newClient.id = m_nextClientId++;
                    newClient.addr = from;
                    newClient.connected = true;
                    newClient.lastPacketTime = m_time;
                    newClient.state.clientId = newClient.id;
                    newClient.state.position = {0, 1, 0};
                    newClient.state.alive = true;
                    newClient.state.health = 100;
                    m_clients.push_back(newClient);
                    
                    uint8_t resp[16];
                    resp[0] = (uint8_t)ServerPacketType::Connect;
                    memcpy(resp + 1, &newClient.id, 4);
                    memcpy(resp + 5, &m_tick, 4);
                    sendto(m_socket, (char*)resp, 9, 0, (sockaddr*)&from, sizeof(from));
                    
                    printf("[Server] Client %d connected\n", newClient.id);
                }
                break;
                
            case ClientPacketType::Disconnect:
                if (client) {
                    printf("[Server] Client %d disconnected\n", client->id);
                    client->connected = false;
                }
                break;
                
            case ClientPacketType::Input:
                if (client && size >= 26) {
                    client->lastPacketTime = m_time;
                    
                    uint16_t pos = 1;
                    uint8_t inputCount = data[pos++];
                    
                    for (int i = 0; i < inputCount && pos + 25 <= size; i++) {
                        PlayerInput input;
                        ReadPlayerInput(data, pos, input);
                        
                        if (input.sequence > client->lastAck) {
                            client->inputQueue.push_back(input);
                            if (client->inputQueue.size() > 32) {
                                client->inputQueue.pop_front();
                            }
                        }
                    }
                }
                break;
                
            case ClientPacketType::Ping:
                if (client) {
                    client->lastPacketTime = m_time;
                    uint8_t resp[16];
                    resp[0] = (uint8_t)ServerPacketType::Pong;
                    memcpy(resp + 1, &m_tick, 4);
                    memcpy(resp + 5, &m_time, 4);
                    sendto(m_socket, (char*)resp, 9, 0, (sockaddr*)&from, sizeof(from));
                }
                break;
        }
    }
    
    void Tick() {
        m_tick++;
        m_time += TICK_INTERVAL;
        
        for (auto& client : m_clients) {
            if (!client.connected) continue;
            
            while (!client.inputQueue.empty()) {
                PlayerInput& input = client.inputQueue.front();
                client.state.ApplyInput(input, TICK_INTERVAL);
                client.state.lastProcessedInput = input.sequence;
                client.lastAck = input.sequence;
                client.inputQueue.pop_front();
            }
            
            if (client.state.position.x < -50) client.state.position.x = -50;
            if (client.state.position.x > 50) client.state.position.x = 50;
            if (client.state.position.z < -50) client.state.position.z = -50;
            if (client.state.position.z > 50) client.state.position.z = 50;
        }
    }
    
    void BroadcastSnapshot() {
        WorldSnapshot snap;
        snap.tick = m_tick;
        snap.serverTime = m_time;
        snap.playerCount = 0;
        
        for (auto& c : m_clients) {
            if (c.connected && snap.playerCount < MAX_PLAYERS) {
                snap.players[snap.playerCount++] = c.state;
            }
        }
        
        uint8_t buffer[1400];
        buffer[0] = (uint8_t)ServerPacketType::Snapshot;
        uint16_t size = 1;
        
        memcpy(buffer + size, &snap.tick, 4); size += 4;
        memcpy(buffer + size, &snap.serverTime, 4); size += 4;
        memcpy(buffer + size, &snap.playerCount, 1); size += 1;
        
        for (int i = 0; i < snap.playerCount; i++) {
            WritePlayerState(buffer, size, snap.players[i]);
        }
        
        for (auto& c : m_clients) {
            if (c.connected) {
                sendto(m_socket, (char*)buffer, size, 0, (sockaddr*)&c.addr, sizeof(c.addr));
            }
        }
    }
    
    void CleanupClients() {
        for (auto it = m_clients.begin(); it != m_clients.end();) {
            if (!it->connected || m_time - it->lastPacketTime > 10.0f) {
                printf("[Server] Client %d timed out\n", it->id);
                it = m_clients.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    ConnectedClient* FindClient(const sockaddr_in& addr) {
        for (auto& c : m_clients) {
            if (c.addr.sin_addr.s_addr == addr.sin_addr.s_addr && 
                c.addr.sin_port == addr.sin_port) {
                return &c;
            }
        }
        return nullptr;
    }

private:
    SOCKET m_socket = INVALID_SOCKET;
    uint16_t m_port = 0;
    bool m_running = false;
    uint32_t m_tick = 0;
    float m_time = 0;
    float m_accumulator = 0;
    float m_snapshotTimer = 0;
    float m_cleanupTimer = 0;
    uint32_t m_nextClientId = 1;
    std::vector<ConnectedClient> m_clients;
    LARGE_INTEGER m_freq;
    LARGE_INTEGER m_lastTime;
};

}
}

#endif

