#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Titan::Network {

enum class SocketType {
    UDP,
    TCP
};

enum class SocketResult {
    Success,
    WouldBlock,
    Disconnected,
    Error
};

class Socket {
public:
    Socket();
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    bool Create(SocketType type);
    bool Bind(uint16_t port);
    bool SetNonBlocking(bool nonBlocking);
    bool SetReuseAddress(bool reuse);
    
    SocketResult SendTo(const void* data, size_t size, const std::string& address, uint16_t port, size_t& bytesSent);
    SocketResult ReceiveFrom(void* buffer, size_t bufferSize, size_t& bytesReceived, std::string& senderAddress, uint16_t& senderPort);
    
    void Close();
    bool IsValid() const;
    
    uint16_t GetBoundPort() const;

private:
    void* socketHandle;
    SocketType type;
    bool isValid;
    uint16_t boundPort;

    void Cleanup();
};

}
