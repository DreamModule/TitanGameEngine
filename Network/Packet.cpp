#include "Packet.hpp"
#include <algorithm>

namespace Titan::Network {

PacketWriter::PacketWriter()
    : writePos(0) {
    buffer.reserve(1024);
}

PacketWriter::PacketWriter(size_t initialCapacity)
    : writePos(0) {
    buffer.reserve(initialCapacity);
}

void PacketWriter::EnsureCapacity(size_t additionalBytes) {
    size_t requiredSize = writePos + additionalBytes;
    if (requiredSize > buffer.size()) {
        buffer.resize(std::max(requiredSize, buffer.size() * 2));
    }
}

void PacketWriter::WriteByte(uint8_t value) {
    EnsureCapacity(sizeof(uint8_t));
    buffer[writePos++] = value;
}

void PacketWriter::WriteUInt16(uint16_t value) {
    EnsureCapacity(sizeof(uint16_t));
    std::memcpy(&buffer[writePos], &value, sizeof(uint16_t));
    writePos += sizeof(uint16_t);
}

void PacketWriter::WriteUInt32(uint32_t value) {
    EnsureCapacity(sizeof(uint32_t));
    std::memcpy(&buffer[writePos], &value, sizeof(uint32_t));
    writePos += sizeof(uint32_t);
}

void PacketWriter::WriteUInt64(uint64_t value) {
    EnsureCapacity(sizeof(uint64_t));
    std::memcpy(&buffer[writePos], &value, sizeof(uint64_t));
    writePos += sizeof(uint64_t);
}

void PacketWriter::WriteInt16(int16_t value) {
    WriteUInt16(static_cast<uint16_t>(value));
}

void PacketWriter::WriteInt32(int32_t value) {
    WriteUInt32(static_cast<uint32_t>(value));
}

void PacketWriter::WriteInt64(int64_t value) {
    WriteUInt64(static_cast<uint64_t>(value));
}

void PacketWriter::WriteFloat(float value) {
    EnsureCapacity(sizeof(float));
    std::memcpy(&buffer[writePos], &value, sizeof(float));
    writePos += sizeof(float);
}

void PacketWriter::WriteDouble(double value) {
    EnsureCapacity(sizeof(double));
    std::memcpy(&buffer[writePos], &value, sizeof(double));
    writePos += sizeof(double);
}

void PacketWriter::WriteBool(bool value) {
    WriteByte(value ? 1 : 0);
}

void PacketWriter::WriteString(const std::string& value) {
    uint16_t length = static_cast<uint16_t>(value.length());
    WriteUInt16(length);
    if (length > 0) {
        WriteBytes(value.data(), length);
    }
}

void PacketWriter::WriteBytes(const void* data, size_t size) {
    if (size == 0) return;
    EnsureCapacity(size);
    std::memcpy(&buffer[writePos], data, size);
    writePos += size;
}

void PacketWriter::Clear() {
    writePos = 0;
}

void PacketWriter::Reserve(size_t capacity) {
    buffer.reserve(capacity);
}

PacketReader::PacketReader(const void* inputData, size_t size)
    : data(static_cast<const uint8_t*>(inputData))
    , dataSize(size)
    , readPos(0) {
}

bool PacketReader::ReadByte(uint8_t& value) {
    if (!CanRead(sizeof(uint8_t))) return false;
    value = data[readPos++];
    return true;
}

bool PacketReader::ReadUInt16(uint16_t& value) {
    if (!CanRead(sizeof(uint16_t))) return false;
    std::memcpy(&value, &data[readPos], sizeof(uint16_t));
    readPos += sizeof(uint16_t);
    return true;
}

bool PacketReader::ReadUInt32(uint32_t& value) {
    if (!CanRead(sizeof(uint32_t))) return false;
    std::memcpy(&value, &data[readPos], sizeof(uint32_t));
    readPos += sizeof(uint32_t);
    return true;
}

bool PacketReader::ReadUInt64(uint64_t& value) {
    if (!CanRead(sizeof(uint64_t))) return false;
    std::memcpy(&value, &data[readPos], sizeof(uint64_t));
    readPos += sizeof(uint64_t);
    return true;
}

bool PacketReader::ReadInt16(int16_t& value) {
    uint16_t temp;
    if (!ReadUInt16(temp)) return false;
    value = static_cast<int16_t>(temp);
    return true;
}

bool PacketReader::ReadInt32(int32_t& value) {
    uint32_t temp;
    if (!ReadUInt32(temp)) return false;
    value = static_cast<int32_t>(temp);
    return true;
}

bool PacketReader::ReadInt64(int64_t& value) {
    uint64_t temp;
    if (!ReadUInt64(temp)) return false;
    value = static_cast<int64_t>(temp);
    return true;
}

bool PacketReader::ReadFloat(float& value) {
    if (!CanRead(sizeof(float))) return false;
    std::memcpy(&value, &data[readPos], sizeof(float));
    readPos += sizeof(float);
    return true;
}

bool PacketReader::ReadDouble(double& value) {
    if (!CanRead(sizeof(double))) return false;
    std::memcpy(&value, &data[readPos], sizeof(double));
    readPos += sizeof(double);
    return true;
}

bool PacketReader::ReadBool(bool& value) {
    uint8_t temp;
    if (!ReadByte(temp)) return false;
    value = (temp != 0);
    return true;
}

bool PacketReader::ReadString(std::string& value) {
    uint16_t length;
    if (!ReadUInt16(length)) return false;
    
    if (length == 0) {
        value.clear();
        return true;
    }
    
    if (!CanRead(length)) return false;
    
    value.assign(reinterpret_cast<const char*>(&data[readPos]), length);
    readPos += length;
    return true;
}

bool PacketReader::ReadBytes(void* buffer, size_t size) {
    if (!CanRead(size)) return false;
    std::memcpy(buffer, &data[readPos], size);
    readPos += size;
    return true;
}

size_t PacketReader::GetRemainingBytes() const {
    return dataSize > readPos ? dataSize - readPos : 0;
}

bool PacketReader::CanRead(size_t bytes) const {
    return readPos + bytes <= dataSize;
}

void PacketReader::Reset() {
    readPos = 0;
}

void PacketHeader::Serialize(PacketWriter& writer) const {
    writer.WriteByte(static_cast<uint8_t>(type));
    writer.WriteUInt32(sequence);
    writer.WriteUInt32(ack);
    writer.WriteUInt32(ackBits);
    writer.WriteUInt16(payloadSize);
}

bool PacketHeader::Deserialize(PacketReader& reader) {
    uint8_t typeValue;
    if (!reader.ReadByte(typeValue)) return false;
    type = static_cast<PacketType>(typeValue);
    
    if (!reader.ReadUInt32(sequence)) return false;
    if (!reader.ReadUInt32(ack)) return false;
    if (!reader.ReadUInt32(ackBits)) return false;
    if (!reader.ReadUInt16(payloadSize)) return false;
    
    return true;
}

}
