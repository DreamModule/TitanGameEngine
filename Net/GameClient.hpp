#ifndef TITAN_GAME_CLIENT_HPP
#define TITAN_GAME_CLIENT_HPP

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
#include "NetTypes.hpp"

#pragma comment(lib, "ws2_32.lib")

namespace Titan {
namespace Net {

class GameClient {
public:
    bool Connect(const char* ip, uint16_t port) {
        m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_socket == INVALID_SOCKET) return false;
        
        u_long mode = 1;
        ioctlsocket(m_socket, FIONBIO, &mode);
        
        int rcvBuf = 1024 * 1024;
        setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuf, sizeof(rcvBuf));
        
        m_serverAddr.sin_family = AF_INET;
        m_serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &m_serverAddr.sin_addr);
        
        QueryPerformanceFrequency(&m_freq);
        QueryPerformanceCounter(&m_lastTime);
        
        uint8_t pkt[1] = {1};
        sendto(m_socket, (char*)pkt, 1, 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
        
        m_connecting = true;
        m_connected = false;
        m_localTime = 0;
        m_serverTime = 0;
        m_renderTime = 0;
        m_inputSequence = 0;
        m_ping = 0;
        
        printf("[Client] Connecting to %s:%d\n", ip, port);
        return true;
    }
    
    void Disconnect() {
        if (!m_connected && !m_connecting) return;
        
        uint8_t pkt[1] = {2};
        sendto(m_socket, (char*)pkt, 1, 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
        
        closesocket(m_socket);
        m_connected = false;
        m_connecting = false;
        m_snapshots.Clear();
        m_pendingInputs.Clear();
        printf("[Client] Disconnected\n");
    }
    
    void Update(float dt) {
        m_localTime += dt;
        
        ReceivePackets();
        
        if (m_connected) {
            float interpDelay = m_ping < 30.0f ? INTERP_DELAY_LAN : INTERP_DELAY_ONLINE;
            float targetRenderTime = m_serverTime - interpDelay;
            
            float interpSpeed = 20.0f;
            m_renderTime = m_renderTime + (targetRenderTime - m_renderTime) * dt * interpSpeed;
            
            if (m_renderTime > m_serverTime) m_renderTime = m_serverTime;
            if (m_renderTime < m_serverTime - 0.5f) m_renderTime = m_serverTime - interpDelay;
            
            m_pingTimer += dt;
            if (m_pingTimer >= 0.25f) {
                m_pingTimer = 0;
                SendPing();
            }
        }
    }
    
    void SetInterpDelay(float delay) { m_customInterpDelay = delay; }
    float GetInterpDelay() const { 
        if (m_customInterpDelay > 0) return m_customInterpDelay;
        return m_ping < 30.0f ? INTERP_DELAY_LAN : INTERP_DELAY_ONLINE; 
    }
    
    void SendInput(const PlayerInput& input) {
        if (!m_connected) return;
        
        InputCommand cmd;
        cmd.input = input;
        cmd.input.sequence = ++m_inputSequence;
        cmd.input.tick = m_serverTick;
        
        m_localState.ApplyInput(cmd.input, TICK_INTERVAL);
        cmd.predictedState = m_localState;
        
        m_pendingInputs.Push(cmd);
        
        uint8_t buffer[512];
        buffer[0] = 3;
        uint16_t size = 1;
        
        uint8_t inputCount = 0;
        auto& inputs = m_pendingInputs.GetBuffer();
        size_t start = inputs.size() > MAX_INPUTS_PER_PACKET ? inputs.size() - MAX_INPUTS_PER_PACKET : 0;
        
        uint8_t countPos = (uint8_t)size++;
        for (size_t i = start; i < inputs.size() && inputCount < MAX_INPUTS_PER_PACKET; i++) {
            WritePlayerInput(buffer, size, inputs[i].input);
            inputCount++;
        }
        buffer[countPos] = inputCount;
        
        sendto(m_socket, (char*)buffer, size, 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
    }
    
    bool GetInterpolatedWorld(WorldSnapshot& out) const {
        return m_snapshots.GetInterpolated(m_renderTime, out);
    }
    
    const PlayerState& GetLocalState() const { return m_localState; }
    PlayerState& GetLocalState() { return m_localState; }
    
    bool IsConnected() const { return m_connected; }
    bool IsConnecting() const { return m_connecting; }
    uint32_t GetClientId() const { return m_clientId; }
    float GetPing() const { return m_ping; }
    float GetServerTime() const { return m_serverTime; }
    float GetRenderTime() const { return m_renderTime; }
    uint32_t GetServerTick() const { return m_serverTick; }

private:
    void ReceivePackets() {
        uint8_t buffer[1500];
        sockaddr_in from;
        int fromLen = sizeof(from);
        
        int maxPackets = 100;
        while (maxPackets-- > 0) {
            int received = recvfrom(m_socket, (char*)buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromLen);
            if (received <= 0) break;
            
            ProcessPacket(buffer, received);
        }
    }
    
    void ProcessPacket(uint8_t* data, int size) {
        if (size < 1) return;
        
        uint8_t type = data[0];
        
        switch (type) {
            case 1:
                if (size >= 9) {
                    memcpy(&m_clientId, data + 1, 4);
                    memcpy(&m_serverTick, data + 5, 4);
                    m_connected = true;
                    m_connecting = false;
                    m_localState.clientId = m_clientId;
                    m_localState.position = {0, 1, 0};
                    printf("[Client] Connected as %d\n", m_clientId);
                }
                break;
                
            case 2:
                m_connected = false;
                printf("[Client] Disconnected by server\n");
                break;
                
            case 3:
                ProcessSnapshot(data, size);
                break;
                
            case 5:
                if (size >= 9) {
                    uint32_t serverTick;
                    float serverTime;
                    memcpy(&serverTick, data + 1, 4);
                    memcpy(&serverTime, data + 5, 4);
                    
                    m_ping = (m_localTime - m_lastPingTime) * 1000.0f;
                    m_serverTick = serverTick;
                    m_serverTime = serverTime;
                    
                    if (m_renderTime < m_serverTime - 1.0f) {
                        m_renderTime = m_serverTime - INTERP_DELAY_ONLINE;
                    }
                }
                break;
        }
    }
    
    void ProcessSnapshot(uint8_t* data, int size) {
        if (size < 10) return;
        
        WorldSnapshot snap;
        uint16_t pos = 1;
        
        memcpy(&snap.tick, data + pos, 4); pos += 4;
        memcpy(&snap.serverTime, data + pos, 4); pos += 4;
        memcpy(&snap.playerCount, data + pos, 1); pos += 1;
        
        for (int i = 0; i < snap.playerCount && pos + 42 <= size; i++) {
            ReadPlayerState(data, pos, snap.players[i]);
        }
        
        m_snapshots.Push(snap);
        m_serverTime = snap.serverTime;
        m_serverTick = snap.tick;
        
        for (int i = 0; i < snap.playerCount; i++) {
            if (snap.players[i].clientId == m_clientId) {
                Reconcile(snap.players[i]);
                break;
            }
        }
    }
    
    void Reconcile(const PlayerState& serverState) {
        m_pendingInputs.RemoveUpTo(serverState.lastProcessedInput);
        
        Vec3 error = serverState.position - m_localState.position;
        float errorMag = error.Length();
        
        if (errorMag > 0.01f) {
            m_localState = serverState;
            
            auto& inputs = m_pendingInputs.GetBuffer();
            for (const auto& cmd : inputs) {
                m_localState.ApplyInput(cmd.input, TICK_INTERVAL);
            }
            
            if (errorMag > 0.5f) {
                printf("[Client] Reconcile: error=%.2f inputs=%zu\n", errorMag, inputs.size());
            }
        }
    }
    
    void SendPing() {
        m_lastPingTime = m_localTime;
        uint8_t pkt[1] = {4};
        sendto(m_socket, (char*)pkt, 1, 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
    }

private:
    SOCKET m_socket = INVALID_SOCKET;
    sockaddr_in m_serverAddr = {};
    bool m_connected = false;
    bool m_connecting = false;
    uint32_t m_clientId = 0;
    uint32_t m_serverTick = 0;
    float m_localTime = 0;
    float m_serverTime = 0;
    float m_renderTime = 0;
    float m_ping = 0;
    float m_pingTimer = 0;
    float m_lastPingTime = 0;
    float m_customInterpDelay = 0;
    uint32_t m_inputSequence = 0;
    
    PlayerState m_localState;
    SnapshotBuffer m_snapshots;
    InputBuffer m_pendingInputs;
    
    LARGE_INTEGER m_freq;
    LARGE_INTEGER m_lastTime;
};

}
}

#endif
