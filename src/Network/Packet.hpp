#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

namespace Titan::Network {

enum class PacketType : uint8_t {
    ConnectionRequest = 1,
    ConnectionAccept = 2,
    ConnectionDeny = 3,
    Disconnect = 4,
    Ping = 5,
    Pong = 6,
    Unreliable = 10,
    Reliable = 11,
    ReliableOrdered = 12,
    Ack = 20,
    Snapshot = 30,
    Input = 31,
    RPC = 40
};

class PacketWriter {
public:
    PacketWriter();
    explicit PacketWriter(size_t initialCapacity);

    void WriteByte(uint8_t value);
    void WriteUInt16(uint16_t value);
    void WriteUInt32(uint32_t value);
    void WriteUInt64(uint64_t value);
    void WriteInt16(int16_t value);
    void WriteInt32(int32_t value);
    void WriteInt64(int64_t value);
    void WriteFloat(float value);
    void WriteDouble(double value);
    void WriteBool(bool value);
    void WriteString(const std::string& value);
    void WriteBytes(const void* data, size_t size);

    const uint8_t* GetData() const { return buffer.data(); }
    size_t GetSize() const { return writePos; }
    void Clear();
    void Reserve(size_t capacity);

private:
    std::vector<uint8_t> buffer;
    size_t writePos;

    void EnsureCapacity(size_t additionalBytes);
};

class PacketReader {
public:
    PacketReader(const void* data, size_t size);

    bool ReadByte(uint8_t& value);
    bool ReadUInt16(uint16_t& value);
    bool ReadUInt32(uint32_t& value);
    bool ReadUInt64(uint64_t& value);
    bool ReadInt16(int16_t& value);
    bool ReadInt32(int32_t& value);
    bool ReadInt64(int64_t& value);
    bool ReadFloat(float& value);
    bool ReadDouble(double& value);
    bool ReadBool(bool& value);
    bool ReadString(std::string& value);
    bool ReadBytes(void* buffer, size_t size);

    size_t GetRemainingBytes() const;
    bool CanRead(size_t bytes) const;
    void Reset();

private:
    const uint8_t* data;
    size_t dataSize;
    size_t readPos;
};

struct PacketHeader {
    PacketType type;
    uint32_t sequence;
    uint32_t ack;
    uint32_t ackBits;
    uint16_t payloadSize;

    static constexpr size_t SIZE = sizeof(PacketType) + sizeof(uint32_t) * 3 + sizeof(uint16_t);

    void Serialize(PacketWriter& writer) const;
    bool Deserialize(PacketReader& reader);
};

}
