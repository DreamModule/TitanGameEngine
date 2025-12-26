#ifndef TITAN_CORE_HPP
#define TITAN_CORE_HPP

#include <cstdint>
#include <vector>
#include <cstring>
#include <cmath>
#include <type_traits>

namespace Titan {
    using u32 = uint32_t;
    using i32 = int32_t;
    using u64 = uint64_t;
    using u16 = uint16_t;
    using u8  = uint8_t;
    using f32 = float;
    using usize = size_t;

    namespace Data::Hash {
        constexpr u32 FNV1a_32(const char* s) {
            u32 h = 0x811c9dc5;
            while (*s) {
                h = (h ^ *s) * 0x01000193;
                ++s;
            }
            return h;
        }
    }

    struct SnapshotStorage {
        std::vector<u8> buffer;
        u32 writePos = 0;
        u32 readPos = 0;

        void Reset() { 
            writePos = 0; readPos = 0; 
            std::vector<u8>().swap(buffer); 
        }
        
        template<typename T>
        void Write(const T& data) {
            static_assert(std::is_trivially_copyable<T>::value, "Snapshot: POD types only!");
            usize size = sizeof(T);
            if (writePos + size > buffer.size()) buffer.resize(writePos + size + 1024);
            std::memcpy(&buffer[writePos], &data, size);
            writePos += size;
        }

        template<typename T>
        bool Read(T& outData) {
            static_assert(std::is_trivially_copyable<T>::value, "Snapshot: POD types only!");
            if (readPos + sizeof(T) > writePos) return false;
            std::memcpy(&outData, &buffer[readPos], sizeof(T));
            readPos += sizeof(T);
            return true;
        }
        const u8* GetData() const { return buffer.data(); }
        u32 GetSize() const { return writePos; }
    };
}
#endif
