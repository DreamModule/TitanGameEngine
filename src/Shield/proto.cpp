#include "TitanShieldCore.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

#ifdef _WIN32
#include <intrin.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "advapi32.lib")
#else
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

namespace TitanShield {

static const uint32_t SHA256_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

const uint8_t RealAES256::staticSBox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

const uint8_t RealAES256::staticInvSBox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

const uint8_t RealAES256::rcon[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

uint64_t HardwareInfo::cpuidHash_ = 0;
uint64_t HardwareInfo::macHash_ = 0;
uint64_t HardwareInfo::diskHash_ = 0;
bool HardwareInfo::initialized_ = false;
std::mutex HardwareInfo::hwMutex_;

bool HardwareInfo::Initialize() {
    std::lock_guard<std::mutex> lock(hwMutex_);
    if (initialized_) return true;
    
    cpuidHash_ = GetCPUIDHash();
    macHash_ = GetMACHash();
    diskHash_ = GetDiskSerialHash();
    initialized_ = true;
    
    return true;
}

uint64_t HardwareInfo::GetCPUIDHash() {
#ifdef _WIN32
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 0);
    
    uint64_t hash = 0;
    hash ^= static_cast<uint64_t>(cpuInfo[0]) << 32;
    hash ^= static_cast<uint64_t>(cpuInfo[1]);
    hash ^= static_cast<uint64_t>(cpuInfo[2]) << 16;
    hash ^= static_cast<uint64_t>(cpuInfo[3]) << 48;
    
    __cpuid(cpuInfo, 1);
    hash ^= static_cast<uint64_t>(cpuInfo[0]);
    hash ^= static_cast<uint64_t>(cpuInfo[3]) << 32;
    
    return hash;
#else
    return 0x1234567890ABCDEFULL;
#endif
}

uint64_t HardwareInfo::GetMACHash() {
#ifdef _WIN32
    IP_ADAPTER_INFO adapterInfo[16];
    DWORD bufLen = sizeof(adapterInfo);
    
    if (GetAdaptersInfo(adapterInfo, &bufLen) == ERROR_SUCCESS) {
        uint64_t hash = 0;
        for (int i = 0; i < 6; i++) {
            hash ^= static_cast<uint64_t>(adapterInfo[0].Address[i]) << (i * 8);
        }
        return hash;
    }
#endif
    return 0xABCDEF1234567890ULL;
}

uint64_t HardwareInfo::GetDiskSerialHash() {
#ifdef _WIN32
    DWORD serialNumber = 0;
    GetVolumeInformationA("C:\\", NULL, 0, &serialNumber, NULL, NULL, NULL, 0);
    return static_cast<uint64_t>(serialNumber) ^ 0x9876543210FEDCBAULL;
#else
    return 0xFEDCBA0987654321ULL;
#endif
}

uint64_t HardwareInfo::GetCombinedFingerprint() {
    if (!initialized_) Initialize();
    return cpuidHash_ ^ macHash_ ^ diskHash_;
}

bool HardwareInfo::ValidateFingerprint(uint64_t storedFingerprint) {
    uint64_t current = GetCombinedFingerprint();
    return current == storedFingerprint;
}

RealAES256::RealAES256() : initialized_(false) {
    std::memcpy(sbox_, staticSBox, 256);
    std::memcpy(invSbox_, staticInvSBox, 256);
}

RealAES256::~RealAES256() {
    SecureWipe();
}

bool RealAES256::Initialize(const std::vector<uint8_t>& key256) {
    if (key256.size() != 32) return false;
    
    KeyExpansion(key256.data());
    initialized_ = true;
    return true;
}

void RealAES256::KeyExpansion(const uint8_t* key) {
    for (int i = 0; i < 8; i++) {
        roundKeys_[i] = (key[4*i] << 24) | (key[4*i+1] << 16) | 
                        (key[4*i+2] << 8) | key[4*i+3];
    }
    
    for (int i = 8; i < 60; i++) {
        uint32_t temp = roundKeys_[i-1];
        
        if (i % 8 == 0) {
            temp = ((sbox_[(temp >> 16) & 0xFF] << 24) |
                    (sbox_[(temp >> 8) & 0xFF] << 16) |
                    (sbox_[temp & 0xFF] << 8) |
                    sbox_[(temp >> 24) & 0xFF]) ^ (rcon[i/8] << 24);
        } else if (i % 8 == 4) {
            temp = (sbox_[(temp >> 24) & 0xFF] << 24) |
                   (sbox_[(temp >> 16) & 0xFF] << 16) |
                   (sbox_[(temp >> 8) & 0xFF] << 8) |
                   sbox_[temp & 0xFF];
        }
        
        roundKeys_[i] = roundKeys_[i-8] ^ temp;
    }
}

uint8_t RealAES256::GFMul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) p ^= a;
        bool hiBitSet = (a & 0x80);
        a <<= 1;
        if (hiBitSet) a ^= 0x1b;
        b >>= 1;
    }
    return p;
}

void RealAES256::SubBytes(uint8_t state[16]) {
    for (int i = 0; i < 16; i++) {
        state[i] = sbox_[state[i]];
    }
}

void RealAES256::InvSubBytes(uint8_t state[16]) {
    for (int i = 0; i < 16; i++) {
        state[i] = invSbox_[state[i]];
    }
}

void RealAES256::ShiftRows(uint8_t state[16]) {
    uint8_t temp;
    
    temp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = temp;
    
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;
    
    temp = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = state[3];
    state[3] = temp;
}

void RealAES256::InvShiftRows(uint8_t state[16]) {
    uint8_t temp;
    
    temp = state[13];
    state[13] = state[9];
    state[9] = state[5];
    state[5] = state[1];
    state[1] = temp;
    
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;
    
    temp = state[3];
    state[3] = state[7];
    state[7] = state[11];
    state[11] = state[15];
    state[15] = temp;
}

void RealAES256::MixColumns(uint8_t state[16]) {
    for (int i = 0; i < 4; i++) {
        uint8_t s0 = state[i*4];
        uint8_t s1 = state[i*4+1];
        uint8_t s2 = state[i*4+2];
        uint8_t s3 = state[i*4+3];
        
        state[i*4] = GFMul(0x02, s0) ^ GFMul(0x03, s1) ^ s2 ^ s3;
        state[i*4+1] = s0 ^ GFMul(0x02, s1) ^ GFMul(0x03, s2) ^ s3;
        state[i*4+2] = s0 ^ s1 ^ GFMul(0x02, s2) ^ GFMul(0x03, s3);
        state[i*4+3] = GFMul(0x03, s0) ^ s1 ^ s2 ^ GFMul(0x02, s3);
    }
}

void RealAES256::InvMixColumns(uint8_t state[16]) {
    for (int i = 0; i < 4; i++) {
        uint8_t s0 = state[i*4];
        uint8_t s1 = state[i*4+1];
        uint8_t s2 = state[i*4+2];
        uint8_t s3 = state[i*4+3];
        
        state[i*4] = GFMul(0x0e, s0) ^ GFMul(0x0b, s1) ^ GFMul(0x0d, s2) ^ GFMul(0x09, s3);
        state[i*4+1] = GFMul(0x09, s0) ^ GFMul(0x0e, s1) ^ GFMul(0x0b, s2) ^ GFMul(0x0d, s3);
        state[i*4+2] = GFMul(0x0d, s0) ^ GFMul(0x09, s1) ^ GFMul(0x0e, s2) ^ GFMul(0x0b, s3);
        state[i*4+3] = GFMul(0x0b, s0) ^ GFMul(0x0d, s1) ^ GFMul(0x09, s2) ^ GFMul(0x0e, s3);
    }
}

void RealAES256::AddRoundKey(uint8_t state[16], int round) {
    for (int i = 0; i < 4; i++) {
        uint32_t k = roundKeys_[round*4 + i];
        state[i*4] ^= (k >> 24) & 0xFF;
        state[i*4+1] ^= (k >> 16) & 0xFF;
        state[i*4+2] ^= (k >> 8) & 0xFF;
        state[i*4+3] ^= k & 0xFF;
    }
}

void RealAES256::EncryptBlock(const uint8_t input[16], uint8_t output[16]) {
    std::memcpy(output, input, 16);
    
    AddRoundKey(output, 0);
    
    for (int round = 1; round < 14; round++) {
        SubBytes(output);
        ShiftRows(output);
        MixColumns(output);
        AddRoundKey(output, round);
    }
    
    SubBytes(output);
    ShiftRows(output);
    AddRoundKey(output, 14);
}

void RealAES256::DecryptBlock(const uint8_t input[16], uint8_t output[16]) {
    std::memcpy(output, input, 16);
    
    AddRoundKey(output, 14);
    InvShiftRows(output);
    InvSubBytes(output);
    
    for (int round = 13; round > 0; round--) {
        AddRoundKey(output, round);
        InvMixColumns(output);
        InvShiftRows(output);
        InvSubBytes(output);
    }
    
    AddRoundKey(output, 0);
}

std::vector<uint8_t> RealAES256::EncryptCBC(const std::vector<uint8_t>& data, const uint8_t iv[16]) {
    size_t paddedSize = ((data.size() + 15) / 16) * 16;
    std::vector<uint8_t> padded(paddedSize);
    std::memcpy(padded.data(), data.data(), data.size());
    
    uint8_t paddingValue = static_cast<uint8_t>(paddedSize - data.size());
    for (size_t i = data.size(); i < paddedSize; i++) {
        padded[i] = paddingValue;
    }
    
    std::vector<uint8_t> result(paddedSize);
    uint8_t prevBlock[16];
    std::memcpy(prevBlock, iv, 16);
    
    for (size_t i = 0; i < paddedSize; i += 16) {
        uint8_t block[16];
        for (int j = 0; j < 16; j++) {
            block[j] = padded[i + j] ^ prevBlock[j];
        }
        
        EncryptBlock(block, &result[i]);
        std::memcpy(prevBlock, &result[i], 16);
    }
    
    return result;
}

std::vector<uint8_t> RealAES256::DecryptCBC(const std::vector<uint8_t>& data, const uint8_t iv[16]) {
    if (data.size() % 16 != 0) return {};
    
    std::vector<uint8_t> result(data.size());
    uint8_t prevBlock[16];
    std::memcpy(prevBlock, iv, 16);
    
    for (size_t i = 0; i < data.size(); i += 16) {
        uint8_t decrypted[16];
        DecryptBlock(&data[i], decrypted);
        
        for (int j = 0; j < 16; j++) {
            result[i + j] = decrypted[j] ^ prevBlock[j];
        }
        
        std::memcpy(prevBlock, &data[i], 16);
    }
    
    if (!result.empty()) {
        uint8_t paddingValue = result.back();
        if (paddingValue > 0 && paddingValue <= 16 && result.size() >= paddingValue) {
            result.resize(result.size() - paddingValue);
        }
    }
    
    return result;
}

std::vector<uint8_t> RealAES256::EncryptGCM(const std::vector<uint8_t>& data, const uint8_t iv[12], std::vector<uint8_t>& tag) {
    uint8_t fullIV[16] = {0};
    std::memcpy(fullIV, iv, 12);
    fullIV[15] = 1;
    
    std::vector<uint8_t> result = EncryptCBC(data, fullIV);
    tag.resize(16);
    auto hash = RealSHA256::Hash(result);
    std::memcpy(tag.data(), hash.data(), 16);
    
    return result;
}

std::vector<uint8_t> RealAES256::DecryptGCM(const std::vector<uint8_t>& data, const uint8_t iv[12], const std::vector<uint8_t>& tag) {
    auto hash = RealSHA256::Hash(data);
    if (!RealSHA256::ConstantTimeCompare(hash.data(), tag.data(), std::min(tag.size(), size_t(16)))) {
        return {};
    }
    
    uint8_t fullIV[16] = {0};
    std::memcpy(fullIV, iv, 12);
    fullIV[15] = 1;
    
    return DecryptCBC(data, fullIV);
}

void RealAES256::SecureWipe() {
    AdvancedCryptoEngine::SecureWipe(roundKeys_, sizeof(roundKeys_));
    AdvancedCryptoEngine::SecureWipe(sbox_, sizeof(sbox_));
    AdvancedCryptoEngine::SecureWipe(invSbox_, sizeof(invSbox_));
    initialized_ = false;
}

uint32_t RealSHA256::RotateRight(uint32_t value, uint32_t bits) {
    return (value >> bits) | (value << (32 - bits));
}

uint32_t RealSHA256::Ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

uint32_t RealSHA256::Maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

uint32_t RealSHA256::Sigma0(uint32_t x) {
    return RotateRight(x, 2) ^ RotateRight(x, 13) ^ RotateRight(x, 22);
}

uint32_t RealSHA256::Sigma1(uint32_t x) {
    return RotateRight(x, 6) ^ RotateRight(x, 11) ^ RotateRight(x, 25​​​​​​​​​​​​​​​​);
}

uint32_t RealSHA256::sigma0(uint32_t x) {
    return RotateRight(x, 7) ^ RotateRight(x, 18) ^ (x >> 3);
}

uint32_t RealSHA256::sigma1(uint32_t x) {
    return RotateRight(x, 17) ^ RotateRight(x, 19) ^ (x >> 10);
}

void RealSHA256::Transform(uint32_t state[8], const uint8_t block[64]) {
    uint32_t w[64];
    
    for (int i = 0; i < 16; i++) {
        w[i] = (block[i*4] << 24) | (block[i*4+1] << 16) | (block[i*4+2] << 8) | block[i*4+3];
    }
    
    for (int i = 16; i < 64; i++) {
        w[i] = sigma1(w[i-2]) + w[i-7] + sigma0(w[i-15]) + w[i-16];
    }
    
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
    
    for (int i = 0; i < 64; i++) {
        uint32_t t1 = h + Sigma1(e) + Ch(e, f, g) + SHA256_K[i] + w[i];
        uint32_t t2 = Sigma0(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

std::array<uint8_t, 32> RealSHA256::Hash(const uint8_t* data, size_t length) {
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    
    uint64_t bitLength = length * 8;
    size_t paddedLength = ((length + 8) / 64 + 1) * 64;
    std::vector<uint8_t> padded(paddedLength, 0);
    
    std::memcpy(padded.data(), data, length);
    padded[length] = 0x80;
    
    for (int i = 0; i < 8; i++) {
        padded[paddedLength - 1 - i] = (bitLength >> (i * 8)) & 0xFF;
    }
    
    for (size_t i = 0; i < paddedLength; i += 64) {
        Transform(state, &padded[i]);
    }
    
    std::array<uint8_t, 32> result;
    for (int i = 0; i < 8; i++) {
        result[i*4] = (state[i] >> 24) & 0xFF;
        result[i*4+1] = (state[i] >> 16) & 0xFF;
        result[i*4+2] = (state[i] >> 8) & 0xFF;
        result[i*4+3] = state[i] & 0xFF;
    }
    
    return result;
}

std::array<uint8_t, 32> RealSHA256::Hash(const std::vector<uint8_t>& data) {
    return Hash(data.data(), data.size());
}

uint64_t RealSHA256::QuickHash64(const uint8_t* data, size_t length) {
    auto hash = Hash(data, length);
    uint64_t result = 0;
    for (int i = 0; i < 8; i++) {
        result ^= static_cast<uint64_t>(hash[i]) << (i * 8);
    }
    return result;
}

std::array<uint8_t, 32> RealSHA256::HMAC(const uint8_t* data, size_t dataLen, const uint8_t* key, size_t keyLen) {
    uint8_t keyPadded[64] = {0};
    
    if (keyLen > 64) {
        auto keyHash = Hash(key, keyLen);
        std::memcpy(keyPadded, keyHash.data(), 32);
    } else {
        std::memcpy(keyPadded, key, keyLen);
    }
    
    uint8_t iPad[64];
    uint8_t oPad[64];
    for (int i = 0; i < 64; i++) {
        iPad[i] = keyPadded[i] ^ 0x36;
        oPad[i] = keyPadded[i] ^ 0x5c;
    }
    
    std::vector<uint8_t> innerData(64 + dataLen);
    std::memcpy(innerData.data(), iPad, 64);
    std::memcpy(innerData.data() + 64, data, dataLen);
    auto innerHash = Hash(innerData.data(), innerData.size());
    
    std::vector<uint8_t> outerData(64 + 32);
    std::memcpy(outerData.data(), oPad, 64);
    std::memcpy(outerData.data() + 64, innerHash.data(), 32);
    
    return Hash(outerData.data(), outerData.size());
}

bool RealSHA256::ConstantTimeCompare(const uint8_t* a, const uint8_t* b, size_t len) {
    volatile uint8_t result = 0;
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

uint64_t PolymorphicKeyExtractor::GetImageBase() {
#ifdef _WIN32
    return reinterpret_cast<uint64_t>(GetModuleHandleA(NULL));
#else
    return 0x400000;
#endif
}

size_t PolymorphicKeyExtractor::GetImageSize() {
#ifdef _WIN32
    HMODULE hModule = GetModuleHandleA(NULL);
    MODULEINFO modInfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo));
    return modInfo.SizeOfImage;
#else
    return 0x100000;
#endif
}

uint64_t PolymorphicKeyExtractor::CalculateShardOffset(size_t index, const void* baseAddress, size_t imageSize) {
    uint64_t base = reinterpret_cast<uint64_t>(baseAddress);
    uint64_t seed = base ^ TITAN_SHIELD_VERSION_OBFUSCATED;
    
    std::mt19937_64 gen(seed + index * 0x9E3779B97F4A7C15ULL);
    
    uint64_t offset = gen() % (imageSize - 0x1000);
    offset = (offset / 16) * 16;
    
    return offset;
}

uint8_t PolymorphicKeyExtractor::DeriveXorMask(size_t index, uint64_t entropy) {
    uint64_t combined = index * 0x517CC1B727220A95ULL ^ entropy;
    return static_cast<uint8_t>((combined >> (index % 8) * 8) & 0xFF);
}

bool PolymorphicKeyExtractor::ValidateShard(const KeyShard& shard, size_t index) {
    if (!shard.valid) return false;
    
    uint64_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum ^= static_cast<uint64_t>(shard.data[i]) << (i * 8);
    }
    checksum ^= shard.offset;
    
    return (checksum & 0xFF) == ((index * 17 + 31) & 0xFF);
}

bool PolymorphicKeyExtractor::ExtractKey256(std::vector<uint8_t>& key) {
    key.resize(32);
    
    uint64_t baseAddress = GetImageBase();
    size_t imageSize = GetImageSize();
    
    if (imageSize < 0x1000) return false;
    
    uint8_t* imagePtr = reinterpret_cast<uint8_t*>(baseAddress);
    
    std::array<KeyShard, SHARD_COUNT> shards;
    uint64_t hwFingerprint = HardwareInfo::GetCombinedFingerprint();
    
    for (size_t i = 0; i < SHARD_COUNT; i++) {
        uint64_t offset = CalculateShardOffset(i, reinterpret_cast<void*>(baseAddress), imageSize);
        
        KeyShard& shard = shards[i];
        shard.offset = offset;
        
        for (int j = 0; j < 4; j++) {
            shard.data[j] = imagePtr[offset + j];
        }
        
        shard.xorMask = DeriveXorMask(i, hwFingerprint);
        shard.valid = true;
    }
    
    for (size_t i = 0; i < SHARD_COUNT; i++) {
        if (!ValidateShard(shards[i], i)) {
            return false;
        }
    }
    
    for (size_t i = 0; i < SHARD_COUNT; i++) {
        size_t keyIndex = i / 2;
        if (keyIndex >= 32) keyIndex = 31;
        
        for (int j = 0; j < 4; j++) {
            uint8_t byte = shards[i].data[j];
            byte ^= shards[i].xorMask;
            byte = ~byte;
            byte ^= static_cast<uint8_t>((i * 7 + j * 11) & 0xFF);
            
            key[keyIndex] ^= byte;
        }
    }
    
    auto keyHash = RealSHA256::Hash(key);
    for (size_t i = 0; i < 32; i++) {
        key[i] ^= keyHash[i];
    }
    
    return true;
}

bool PolymorphicKeyExtractor::VerifyKeyIntegrity() {
    std::vector<uint8_t> key;
    return ExtractKey256(key);
}

void PolymorphicKeyExtractor::EmbedKeyShards(uint8_t* imageBase, size_t imageSize, const std::vector<uint8_t>& key) {
    if (key.size() != 32 || imageSize < 0x1000) return;
    
    auto keyHash = RealSHA256::Hash(key);
    std::vector<uint8_t> processedKey(32);
    for (size_t i = 0; i < 32; i++) {
        processedKey[i] = key[i] ^ keyHash[i];
    }
    
    uint64_t hwFingerprint = HardwareInfo::GetCombinedFingerprint();
    
    for (size_t i = 0; i < SHARD_COUNT; i++) {
        uint64_t offset = CalculateShardOffset(i, imageBase, imageSize);
        size_t keyIndex = i / 2;
        if (keyIndex >= 32) keyIndex = 31;
        
        uint8_t xorMask = DeriveXorMask(i, hwFingerprint);
        
        for (int j = 0; j < 4; j++) {
            uint8_t byte = processedKey[keyIndex];
            byte ^= static_cast<uint8_t>((i * 7 + j * 11) & 0xFF);
            byte = ~byte;
            byte ^= xorMask;
            
            imageBase[offset + j] = byte;
        }
    }
}

AdvancedCryptoEngine::CryptoContext& AdvancedCryptoEngine::GetContext() {
    static CryptoContext ctx;
    return ctx;
}

bool AdvancedCryptoEngine::Initialize(const std::vector<uint8_t>& key256) {
    auto& ctx = GetContext();
    std::lock_guard<std::mutex> lock(ctx.contextMutex);
    
    if (key256.size() != 32) return false;
    
    ctx.aes = std::make_unique<RealAES256>();
    if (!ctx.aes->Initialize(key256)) {
        return false;
    }
    
    std::memcpy(ctx.workingKey.data(), key256.data(), 32);
    ctx.operationCounter = 0;
    ctx.initialized = true;
    ctx.lastKeyRotation = std::chrono::steady_clock::now().time_since_epoch().count();
    
    DeriveChecksumKey(ctx.workingKey);
    
    return true;
}

void AdvancedCryptoEngine::Shutdown() {
    auto& ctx = GetContext();
    std::lock_guard<std::mutex> lock(ctx.contextMutex);
    
    if (ctx.aes) {
        ctx.aes->SecureWipe();
        ctx.aes.reset();
    }
    
    SecureWipe(ctx.workingKey.data(), ctx.workingKey.size());
    SecureWipe(ctx.checksumKey.data(), ctx.checksumKey.size());
    ctx.initialized = false;
}

void AdvancedCryptoEngine::DeriveChecksumKey(const std::array<uint8_t, 32>& masterKey) {
    auto& ctx = GetContext();
    const uint8_t salt[] = "TitanChecksumSalt2025";
    ctx.checksumKey = RealSHA256::HMAC(masterKey.data(), masterKey.size(), salt, sizeof(salt) - 1);
}

void AdvancedCryptoEngine::ApplyPadding(std::vector<uint8_t>& data) {
    size_t originalSize = data.size();
    size_t paddedSize = ((originalSize + 15) / 16) * 16;
    uint8_t paddingValue = static_cast<uint8_t>(paddedSize - originalSize);
    
    data.resize(paddedSize);
    for (size_t i = originalSize; i < paddedSize; i++) {
        data[i] = paddingValue;
    }
}

void AdvancedCryptoEngine::RemovePadding(std::vector<uint8_t>& data) {
    if (data.empty()) return;
    
    uint8_t paddingValue = data.back();
    if (paddingValue > 0 && paddingValue <= 16 && data.size() >= paddingValue) {
        bool validPadding = true;
        for (size_t i = data.size() - paddingValue; i < data.size(); i++) {
            if (data[i] != paddingValue) {
                validPadding = false;
                break;
            }
        }
        
        if (validPadding) {
            data.resize(data.size() - paddingValue);
        }
    }
}

std::vector<uint8_t> AdvancedCryptoEngine::Encrypt(const std::vector<uint8_t>& data, const SecurityContext& ctx) {
    auto& cryptoCtx = GetContext();
    std::lock_guard<std::mutex> lock(cryptoCtx.contextMutex);
    
    if (!cryptoCtx.initialized || !cryptoCtx.aes) return data;
    
    std::vector<uint8_t> contextMixed = data;
    for (size_t i = 0; i < contextMixed.size(); i++) {
        contextMixed[i] ^= ctx.fingerprint[i % 128];
        contextMixed[i] ^= static_cast<uint8_t>((ctx.entropy[i % 16] >> ((i % 8) * 8)) & 0xFF);
    }
    
    uint8_t iv[16];
    std::memcpy(iv, ctx.sessionIV, 16);
    
    auto encrypted = cryptoCtx.aes->EncryptCBC(contextMixed, iv);
    
    std::vector<uint8_t> result(16 + encrypted.size());
    std::memcpy(result.data(), iv, 16);
    std::memcpy(result.data() + 16, encrypted.data(), encrypted.size());
    
    cryptoCtx.operationCounter++;
    
    return result;
}

std::vector<uint8_t> AdvancedCryptoEngine::Decrypt(const std::vector<uint8_t>& data, const SecurityContext& ctx, TitanError& error) {
    auto& cryptoCtx = GetContext();
    std::lock_guard<std::mutex> lock(cryptoCtx.contextMutex);
    
    if (!cryptoCtx.initialized || !cryptoCtx.aes) {
        error = TitanError::NotInitialized;
        return data;
    }
    
    if (data.size() < 16) {
        error = TitanError::DecryptionFailed;
        return {};
    }
    
    uint8_t iv[16];
    std::memcpy(iv, data.data(), 16);
    
    std::vector<uint8_t> encryptedData(data.begin() + 16, data.end());
    auto decrypted = cryptoCtx.aes->DecryptCBC(encryptedData, iv);
    
    if (decrypted.empty()) {
        error = TitanError::DecryptionFailed;
        return {};
    }
    
    for (size_t i = 0; i < decrypted.size(); i++) {
        decrypted[i] ^= static_cast<uint8_t>((ctx.entropy[i % 16] >> ((i % 8) * 8)) & 0xFF);
        decrypted[i] ^= ctx.fingerprint[i % 128];
    }
    
    cryptoCtx.operationCounter++;
    error = TitanError::Success;
    
    return decrypted;
}

std::vector<uint8_t> AdvancedCryptoEngine::DeriveSessionKey(const std::vector<uint8_t>& masterKey, const SecurityContext& ctx) {
    std::vector<uint8_t> material;
    material.insert(material.end(), masterKey.begin(), masterKey.end());
    
    for (size_t i = 0; i < 128; i++) {
        material.push_back(ctx.fingerprint[i]);
    }
    
    for (size_t i = 0; i < 16; i++) {
        uint64_t entropy = ctx.entropy[i];
        for (int j = 0; j < 8; j++) {
            material.push_back(static_cast<uint8_t>((entropy >> (j * 8)) & 0xFF));
        }
    }
    
    uint64_t hwFingerprint = HardwareInfo::GetCombinedFingerprint();
    for (int i = 0; i < 8; i++) {
        material.push_back(static_cast<uint8_t>((hwFingerprint >> (i * 8)) & 0xFF));
    }
    
    const uint8_t salt[] = "TitanSessionKey2025";
    return DeriveKey(material, salt, sizeof(salt) - 1, 10000);
}

std::vector<uint8_t> AdvancedCryptoEngine::GenerateSecureKey(size_t bits) {
    size_t bytes = bits / 8;
    std::vector<uint8_t> key(bytes);
    
    std::random_device rd;
    std::mt19937_64 gen(rd());
    
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t timestamp = now.time_since_epoch().count();
    gen.seed(gen() ^ timestamp ^ rd());
    
#ifdef _WIN32
    LARGE_INTEGER perfCounter;
    QueryPerformanceCounter(&perfCounter);
    gen.seed(gen() ^ perfCounter.QuadPart);
#endif
    
    uint64_t hwFingerprint = HardwareInfo::GetCombinedFingerprint();
    gen.seed(gen() ^ hwFingerprint);
    
    for (size_t i = 0; i < bytes; i += 8) {
        uint64_t rand = gen();
        std::memcpy(key.data() + i, &rand, std::min(size_t(8), bytes - i));
    }
    
    return key;
}

void AdvancedCryptoEngine::SecureWipe(void* data, size_t len) {
    volatile uint8_t* ptr = static_cast<uint8_t*>(data);
    for (size_t i = 0; i < len; i++) {
        ptr[i] = 0xFF;
        ptr[i] = 0x00;
        ptr[i] = 0xAA;
        ptr[i] = 0x55;
        ptr[i] = 0x00;
    }
    
#ifdef _WIN32
    SecureZeroMemory(data, len);
#else
    std::memset(data, 0, len);
    __asm__ __volatile__("" : : "r"(data) : "memory");
#endif
}

uint64_t AdvancedCryptoEngine::SecureHash(const uint8_t* data, size_t len) {
    return RealSHA256::QuickHash64(data, len);
}

std::array<uint8_t, 32> AdvancedCryptoEngine::DeriveKey(const std::vector<uint8_t>& input, const uint8_t* salt, size_t saltLen, uint32_t iterations) {
    std::array<uint8_t, 32> derived = RealSHA256::HMAC(input.data(), input.size(), salt, saltLen);
    
    for (uint32_t i = 1; i < iterations; i++) {
        derived = RealSHA256::HMAC(derived.data(), derived.size(), salt, saltLen);
    }
    
    return derived;
}

bool AdvancedCryptoEngine::RotateSessionKey() {
    auto& ctx = GetContext();
    std::lock_guard<std::mutex> lock(ctx.contextMutex);
    
    if (!ctx.initialized) return false;
    
    auto newKey = GenerateSecureKey(256);
    
    if (ctx.aes) {
        ctx.aes->SecureWipe();
        ctx.aes = std::make_unique<RealAES256>();
        if (!ctx.aes->Initialize(newKey)) {
            return false;
        }
    }
    
    SecureWipe(ctx.workingKey.data(), ctx.workingKey.size());
    std::memcpy(ctx.workingKey.data(), newKey.data(), 32);
    ctx.lastKeyRotation = std::chrono::steady_clock::now().time_since_epoch().count();
    
    DeriveChecksumKey(ctx.workingKey);
    
    return true;
}

AntiReverseEngineering::ProtectionState& AntiReverseEngineering::GetState() {
    static ProtectionState state;
    return state;
}

std::vector<AntiReverseEngineering::MemoryTrap>& AntiReverseEngineering::GetMemoryTraps() {
    static std::vector<MemoryTrap> traps;
    return traps;
}

AntiReverseEngineering::AntiDebugState& AntiReverseEngineering::GetDebugState() {
    static AntiDebugState state = {};
    return state;
}

uint64_t AntiReverseEngineering::GetRDTSC() {
#ifdef _WIN32
    return __rdtsc();
#else
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#endif
}

bool AntiReverseEngineering::DetectTimingAnomaly() {
    auto& state = GetState();
    
    uint64_t start = GetRDTSC();
    volatile int dummy = 0;
    for (int i = 0; i < 100; i++) {
        dummy += i;
    }
    uint64_t end = GetRDTSC();
    
    uint64_t delta = end - start;
    
    if (delta > 100000) {
        return true;
    }
    
    uint64_t lastRdtsc = state.lastRdtsc.load();
    if (lastRdtsc > 0 && end < lastRdtsc) {
        return true;
    }
    
    state.lastRdtsc.store(end);
    return false;
}

bool AntiReverseEngineering::DetectDebuggerPEB() {
#ifdef _WIN32
    BOOL isDebuggerPresent = FALSE;
    
#ifdef _WIN64
    uint64_t peb = __readgsqword(0x60);
    isDebuggerPresent = *reinterpret_cast<BOOL*>(peb + 0x02);
#else
    uint32_t peb = __readfsdword(0x30);
    isDebuggerPresent = *reinterpret_cast<BOOL*>(peb + 0x02);
#endif
    
    return isDebuggerPresent != FALSE;
#else
    return false;
#endif
}

bool AntiReverseEngineering::DetectDebuggerNtQuery() {
#ifdef _WIN32
    typedef NTSTATUS (WINAPI *pNtQueryInformationProcess)(HANDLE, DWORD, PVOID, ULONG, PULONG);
    
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) return false;
    
    pNtQueryInformationProcess NtQueryInformationProcess = 
        reinterpret_cast<pNtQueryInformationProcess>(GetProcAddress(hNtdll, "NtQueryInformationProcess"));
    
    if (!NtQueryInformationProcess) return false;
    
    DWORD debugPort = 0;
    NTSTATUS status = NtQueryInformationProcess(GetCurrentProcess(), 7, &debugPort, sizeof(debugPort), NULL);
    
    if (status == 0 && debugPort != 0) {
        return true;
    }
    
    BOOL debugObjectHandle = FALSE;
    status = NtQueryInformationProcess(GetCurrentProcess(), 30, &debugObjectHandle, sizeof(debugObjectHandle), NULL);
    
    if (status == 0 && debugObjectHandle != FALSE) {
        return true;
    }
#endif
    return false;
}

bool AntiReverseEngineering::DetectHardwareBreakpoints() {
#ifdef _WIN32
    CONTEXT ctx = {};
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    
    if (GetThreadContext(GetCurrentThread(), &ctx)) {
        if (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0) {
            return true;
        }
        
        if (ctx.Dr7 & 0xFF) {
            return true;
        }
    }
#endif
    return false;
}

bool AntiReverseEngineering::DetectSoftwareBreakpoints() {
    auto& state = GetState();
    std::lock_guard<std::mutex> lock(state.stateMutex);
    
#ifdef _WIN32
    HMODULE hModule = GetModuleHandleA(NULL);
    MODULEINFO modInfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo));
    
    uint8_t* codeStart = reinterpret_cast<uint8_t*>(modInfo.lpBaseOfDll);
    size_t codeSize = modInfo.SizeOfImage;
    
    for (size_t i = 0; i < codeSize - 1; i++) {
        if (codeStart[i] == 0xCC || codeStart[i] == 0xCD) {
            return true;
        }
    }
#endif
    
    return false;
}

bool AntiReverseEngineering::DetectVirtualMachineStealth() {
#ifdef _WIN32
    const char* vmKeys[] = {
        "SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum",
        "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0",
        "HARDWARE\\Description\\System"
    };
    
    const char* vmStrings[] = {
        "vmware", "vbox", "qemu", "virtual", "xen", "parallels", "hyperv"
    };
    
    for (const char* keyPath : vmKeys) {
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char value[256];
            DWORD size = sizeof(value);
            
            if (RegQueryValueExA(hKey, "Identifier", NULL, NULL, (LPBYTE)value, &size) == ERROR_SUCCESS ||
                RegQueryValueExA(hKey, "SystemBiosVersion", NULL, NULL, (LPBYTE)value, &size) == ERROR_SUCCESS ||
                RegQueryValueExA(hKey, "0", NULL, NULL, (LPBYTE)value, &size) == ERROR_SUCCESS) {
                
                for (const char* vmStr : vmStrings) {
                    if (strstr(value, vmStr)) {
                        RegCloseKey(hKey);
                        return true;
                    }
                }
            }
            RegCloseKey(hKey);
        }
    }
    
    __try {
        __asm {
            push edx
            push ecx
            push ebx
            
            mov eax, 'VMXh'
            mov ebx, 0
            mov ecx, 10
            mov edx, 'VX'
            
            in eax, dx
            
            pop ebx
            pop ecx
            pop edx
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    
    return true;
#else
    return false;
#endif
}

bool AntiReverseEngineering::DetectHookingInline() {
#ifdef _WIN32
    const char* criticalFunctions[] = {
        "NtQueryInformationProcess",
        "NtCreateThreadEx",
        "VirtualProtect",
        "WriteProcessMemory"
    };
    
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) return false;
    
    for (const char* funcName : criticalFunctions) {
        FARPROC funcAddr = GetProcAddress(hNtdll, funcName);
        if (!funcAddr) continue;
        
        uint8_t* funcBytes = reinterpret_cast<uint8_t*>(funcAddr);
        
        if (funcBytes[0] == 0xE9 || funcBytes[0] == 0xEB) {
            return true;
        }
        
        if (funcBytes[0] == 0xFF && funcBytes[1] == 0x25) {
            return true;
        }
        
        if (funcBytes[0] == 0x68) {
            return true;
        }
    }
#endif
    return false;
}

bool AntiReverseEngineering::DetectMemoryDumpingTiming() {
    uint64_t start = GetRDTSC();
    
    volatile uint8_t dummy[1024];
    for (int i = 0; i < 1024; i++) {
        dummy[i] = i & 0xFF;
    }
    
    uint64_t end = GetRDTSC();
    uint64_t delta = end - start;
    
    return delta > 500000;
}

bool AntiReverseEngineering::DetectCodeInjectionSections() {
#ifdef _WIN32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;
    
    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);
    
    int moduleCount = 0;
    if (Module32First(hSnapshot, &moduleEntry)) {
        do {
            moduleCount++;
        } while (Module32Next(hSnapshot, &moduleEntry));
    }
    
    CloseHandle(hSnapshot);
    
    if (moduleCount > 100) {
        return true;
    }
#endif
    return false;
}

bool AntiReverseEngineering::DetectFridaXposedAdvanced() {
#ifdef __ANDROID__
    const char* suspiciousLibs[] = {
        "frida",
        "xposed",
        "substrate",
        "libfrida-gadget.so",
        "libxposed_art.so",
        "com.saurik.substrate"
    };
    
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) return false;
    
    char line[512];
    while (fgets(line, sizeof(line), maps)) {
        for (const char* lib : suspiciousLibs) {
            if (strstr(line, lib)) {
                fclose(maps);
                return true;
            }
        }
    }
    fclose(maps);
    
    const char* suspiciousFiles[] = {
        "/data/local/tmp/frida-server",
        "/data/local/tmp/re.frida.server",
        "/system/xbin/su",
        "/system/bin/su"
    };
    
    for (const char* file : suspiciousFiles) {
        struct stat st;
        if (stat(file, &st) == 0) {
            return true;
        }
    }
#endif
    return false;
}

bool AntiReverseEngineering::DetectRootedDeviceDeep() {
#ifdef __ANDROID__
    const char* suPaths[] = {
        "/system/app/Superuser.apk",
        "/sbin/su",
        "/system/bin/su",
        "/system/xbin/su",
        "/data/local/xbin/su",
        "/data/local/bin/su",
        "/system/sd/xbin/su",
        "/system/bin/failsafe/su",
        "/data/local/su",
        "/su/bin/su"
    };
    
    for (const char* path : suPaths) {
        struct stat st;
        if (stat(path, &st) == 0) {
            return true;
        }
    }
    
    FILE* fp = popen("which su", "r");
    if (fp) {
        char path[256];
        if (fgets(path, sizeof(path), fp) != NULL) {
            pclose(fp);
            return true;
        }
        pclose(fp);
    }
#endif
    return false;
}

bool AntiReverseEngineering::DetectEmulatorAdvanced() {
#ifdef __ANDROID__
    char brand[256] = {0};
    char model[256] = {0};
    char manufacturer[256] = {0};
    
    __system_property_get("ro.product.brand", brand);
    __system_property_get("ro.product.model", model);
    __system_property_get("ro.product.manufacturer", manufacturer);
    
    const char* emulatorStrings[] = {
        "generic", "unknown", "emulator", "sdk", "google_sdk",
        "Genymotion", "Andy", "nox", "bluestacks"
    };
    
    for (const char* str : emulatorStrings) {
        if (strstr(brand, str) || strstr(model, str) || strstr(manufacturer, str)) {
            return true;
        }
    }
    
    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo) {
        char line[256];
        while (fgets(line, sizeof(line), cpuinfo)) {
            if (strstr(line, "goldfish") || strstr(line, "ranchu")) {
                fclose(cpuinfo);
                return true;
            }
        }
        fclose(cpuinfo);
    }
#endif
    return false;
}

bool AntiReverseEngineering::ValidateStackIntegrity() {
#ifdef _WIN32
    MEMORY_BASIC_INFORMATION mbi;
    void* stackPtr = &mbi;
    
    if (VirtualQuery(stackPtr, &mbi, sizeof(mbi))) {
        if (mbi.Protect != PAGE_READWRITE) {
            return false;
        }
        
        if (mbi.State != MEM_COMMIT) {
            return false;
        }
    }
#endif
    return true;
}

bool AntiReverseEngineering::CheckParentProcess() {
#ifdef _WIN32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return true;
    
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    
    DWORD currentPid = GetCurrentProcessId();
    DWORD parentPid = 0;
    
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (pe.th32ProcessID == currentPid) {
                parentPid = pe.th32ParentProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    
    if (parentPid == 0) {
        CloseHandle(hSnapshot);
        return true;
    }
    
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (pe.th32ProcessID == parentPid) {
                const char* suspiciousParents[] = {
                    "x64dbg.exe", "x32dbg.exe", "ollydbg.exe", "ida.exe", 
                    "ida64.exe", "windbg.exe", "gdb.exe", "python.exe",
                    "frida-server.exe", "cheatengine-x86_64.exe"
                };
                
                for (const char* suspicious : suspiciousParents) {
                    if (_stricmp(pe.szExeFile, suspicious) == 0) {
                        CloseHandle(hSnapshot);
                        return false;
                    }
                }
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    
    CloseHandle(hSnapshot);
#endif
    return true;
}

uint64_t AntiReverseEngineering::CalculateCodeChecksum(const void* start, size_t len) {
    auto& ctx = AdvancedCryptoEngine::GetContext();
    const uint8_t* checksumKey = ctx.checksumKey.data();
    
    auto hmac = RealSHA256::HMAC(static_cast<const uint8_t*>(start), len, checksumKey, 32);
    
    uint64_t result = 0;
    for (int i = 0; i < 8; i++) {
        result ^= static_cast<uint64_t>(hmac[i]) << (i * 8);
    }
    return result;
}

void AntiReverseEngineering::UpdateDetectionScore(uint32_t penalty) {
    auto& state = GetState();
    uint32_t current = state.detectionScore.load();
    state.detectionScore.store(current + penalty);
}

void AntiReverseEngineering::PolymorphicDelay() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10, 100);
    
    int iterations = dis(gen);
    volatile int dummy = 0;
    for (int i = 0; i < iterations; i++) {
        dummy += i * i;
    }
}

bool AntiReverseEngineering::InitializeProtection() {
    auto& state = GetState();
    std::lock_guard<std::mutex> lock(state.stateMutex);
    
#ifdef _WIN32
    HMODULE hModule = GetModuleHandleA(NULL);
    MODULEINFO modInfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo));
    
    uint8_t* codeStart = reinterpret_cast<uint8_t*>(modInfo.lpBaseOfDll);
    size_t codeSize = modInfo.SizeOfImage;
    
    size_t segmentCount = 32;
    size_t segmentSize = codeSize / segmentCount;
    
    state.codeChecksums.resize(segmentCount);
    for (size_t i = 0; i < segmentCount; i++) {
        state.codeChecksums[i] = CalculateCodeChecksum(codeStart + i * segmentSize, segmentSize);
    }
#endif
    
    state.lastValidation = std::chrono::steady_clock::now();
    state.detectionScore = 0;
    state.debugAttempts = 0;
    state.lastRdtsc = GetRDTSC();
    
    return true;
}

bool AntiReverseEngineering::PerformRuntimeChecks() {
    auto& state = GetState();
    auto& debugState = GetDebugState();
    
    state.checkpointCounter.fetch_add(1);
    uint64_t checkpoint = state.checkpointCounter.load();
    
    debugState.lastCheckTime = std::chrono::steady_clock::now().time_since_epoch().count();
    
    if (checkpoint % 2 == 0) {
        if (DetectTimingAnomaly()) {
            UpdateDetectionScore(10);
            debugState.suspicionLevel += 10;
        }
    }
    
    if (checkpoint % 3 == 0) {
        if (DetectDebuggerPEB()) {
            UpdateDetectionScore(50);
            debugState.isDebuggerPresent = true;
            state.debugAttempts.fetch_add(1);
            return false;
        }
    }
    
    if (checkpoint % 5 == 0) {
        if (DetectDebuggerNtQuery()) {
            UpdateDetectionScore(50);
            debugState.isRemoteDebugger = true;
            return false;
        }
    }
    
    if (checkpoint % 7 == 0) {
        if (DetectHardwareBreakpoints()) {
            UpdateDetectionScore(100);
            debugState.hasBreakpoints = true;
            return false;
        }
    }
    
    if (checkpoint % 11 == 0) {
        if (DetectHookingInline()) {
            UpdateDetectionScore(75);
            debugState.hasHooks = true;
            return false;
        }
    }
    
    if (checkpoint % 13 == 0) {
        if (DetectVirtualMachineStealth()) {
            UpdateDetectionScore(30);
            debugState.isVirtualMachine = true;
        }
    }
    
    if (checkpoint % 17 == 0) {
        if (!ValidateStackIntegrity()) {
            UpdateDetectionScore(60);
            return false;
        }
    }
    
    if (checkpoint % 19 == 0) {
        if (!CheckParentProcess()) {
            UpdateDetectionScore(80);
            return false;
        }
    }
    
    if (state.detectionScore.load() > 200) {
        return false;
    }
    
    PolymorphicDelay();
    
    return true;
}

bool AntiReverseEngineering::ValidateEnvironment() {
#ifdef _WIN32
    if (DetectVirtualMachineStealth()) {
        return false;
    }
    
    if (!CheckParentProcess()) {
        return false;
    }
    
    return true;
#elif __ANDROID__
    if (DetectRootedDeviceDeep()) {
        return false;
    }
    
    if (DetectEmulatorAdvanced()) {
        return false;
    }
    
    if (DetectFridaXposedAdvanced()) {
        return false;
    }
    
    struct stat st;
    return (stat("/data/data/dream.titan.game", &st) == 0);
#else
    return false;
#endif
}

bool AntiReverseEngineering::ContinuousMonitoring() {
    auto& state = GetState();
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - state.lastValidation).count();
    
    if (elapsed > 60) {
        if (!PerformRuntimeChecks()) {
            return false;
        }
        
        if (!VerifyMemoryTraps()) {
            return false;
        }
        
        state.lastValidation = now;
        
        if (state.detectionScore.load() > 50) {
            state.detectionScore.store(state.detectionScore.load() - 10);
        }
    }
    
    return true;
}

void AntiReverseEngineering::RegisterMemoryTrap(void* address, size_t size) {
    auto& traps = GetMemoryTraps();
    
    MemoryTrap trap;
    trap.address = address;
    trap.size = size;
    trap.checksumHash = RealSHA256::Hash(static_cast<uint8_t*>(address), size);
    trap.active = true;
    trap.violationCount = 0;
    
    std::random_device rd;
    std::mt19937_64 gen(rd());
    trap.canary = gen() ^ reinterpret_cast<uint64_t>(address);
    
    traps.push_back(trap);
}

bool AntiReverseEngineering::VerifyMemoryTraps() {
    auto& traps = GetMemoryTraps();
    
    for (auto& trap : traps) {
        if (!trap.active) continue;
        
        auto currentHash = RealSHA256::Hash(static_cast<uint8_t*>(trap.address), trap.size);
        
        if (!RealSHA256::ConstantTimeCompare(currentHash.data(), trap.checksumHash.data(), 32)) {
            trap.violationCount++;
            
            if (trap.violationCount >= 3) {
                return false;
            }
            
            trap.checksumHash = currentHash;
        }
    }
    
    return true;
}

void AntiReverseEngineering::TriggerAntiTamperSoft() {
    auto& state = GetState();
    state.integrityViolation.store(true);
    UpdateDetectionScore(150);
    
    PolymorphicDelay();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 5000);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
}

void AntiReverseEngineering::TriggerAntiTamperHard() {
    auto& state = GetState();
    state.integrityViolation.store(true);
    
#ifdef _WIN32
    TerminateProcess(GetCurrentProcess(), 0x8B7D91A3);
#else
    exit(0x8B7D91A3);
#endif
}

AntiReverseEngineering::AntiDebugState AntiReverseEngineering::GetDebugState() {
    return GetDebugState();
}

uint32_t AntiReverseEngineering::GetDetectionScore() {
    return GetState().detectionScore.load();
}

void AntiReverseEngineering::ResetDetectionScore() {
    GetState().detectionScore.store(0);
}

std::map<void*, SecureMemoryManager::SecureBlock>& SecureMemoryManager::GetSecureBlocks() {
    static std::map<void*, SecureBlock> blocks;
    return blocks;
}

std::mutex& SecureMemoryManager::GetMemoryMutex() {
    static std::mutex mutex;
    return mutex;
}

void* SecureMemoryManager::AllocateWithGuards(size_t size) {
#ifdef _WIN32
    size_t pageSize = 4096;
    size_t totalSize = pageSize + size + pageSize;
    
    void* base = VirtualAlloc(NULL, totalSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!base) return nullptr;
    
    DWORD oldProtect;
    VirtualProtect(base, pageSize, PAGE_NOACCESS, &oldProtect);
    VirtualProtect(static_cast<uint8_t*>(base) + pageSize + size, pageSize, PAGE_NOACCESS, &oldProtect);
    
    return static_cast<uint8_t*>(base) + pageSize;
#else
    size_t pageSize = 4096;
    size_t totalSize = pageSize + size + pageSize;
    
    void* base = mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED) return nullptr;
    
    mprotect(base, pageSize, PROT_NONE);
    mprotect(static_cast<uint8_t*>(base) + pageSize + size, pageSize, PROT_NONE);
    
    return static_cast<uint8_t*>(base) + pageSize;
#endif
}

void SecureMemoryManager::FreeWithGuards(void* ptr, size_t size) {
#ifdef _WIN32
    size_t pageSize = 4096;
    void* base = static_cast<uint8_t*>(ptr) - pageSize;
    VirtualFree(base, 0, MEM_RELEASE);
#else
    size_t pageSize = 4096;
    void* base = static_cast<uint8_t*>(ptr) - pageSize;
    size_t totalSize = pageSize + size + pageSize;
    munmap(base, totalSize);
#endif
}

void* SecureMemoryManager::AllocateSecure(size_t size) {
    void* ptr = AllocateWithGuards(size);
    if (!ptr) return nullptr;
    
    auto& blocks = GetSecureBlocks();
    std::lock_guard<std::mutex> lock(GetMemoryMutex());
    
    SecureBlock block;
    block.address = ptr;
    block.size = size;
    block.checksumHash = RealSHA256::Hash(static_cast<uint8_t*>(ptr), size);
    block.encrypted = false;
    block.locked = false;
    block.lastAccess = std::chrono::steady_clock::now().time_since_epoch().count();
    block.accessCount = 0;
    
    std::random_device rd;
    std::mt19937_64 gen(rd());
    block.canary1 = gen() ^ reinterpret_cast<uint64_t>(ptr);
    block.canary2 = gen() ^ reinterpret_cast<uint64_t>(&block);
    
    blocks[ptr] = block;
    
    return ptr;
}

void SecureMemoryManager::FreeSecure(void* ptr) {
    if (!ptr) return;
    
    auto& blocks = GetSecureBlocks();
    std::lock_guard<std::mutex> lock(GetMemoryMutex());
    
    auto it = blocks.find(ptr);
    if (it != blocks.end()) {
        WipeMemory(ptr, it->second.size);
        FreeWithGuards(ptr, it->second.size);
        blocks.erase(it);
    }
}

bool SecureMemoryManager::ProtectMemory(void* ptr, size_t size) {
#ifdef _WIN32
    DWORD oldProtect;
    return VirtualProtect(ptr, size, PAGE_NOACCESS, &oldProtect) != 0;
#else
    return mprotect(ptr, size, PROT_NONE) == 0;
#endif
}

bool SecureMemoryManager::UnprotectMemory(void* ptr, size_t size) {
#ifdef _WIN32
    DWORD oldProtect;
    return VirtualProtect(ptr, size, PAGE_READWRITE, &oldProtect) != 0;
#else
    return mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0;
#endif
}

bool SecureMemoryManager::EncryptMemoryBlock(void* ptr) {
    auto& blocks = GetSecureBlocks();
    std::lock_guard<std::mutex> lock(GetMemoryMutex());
    
    auto it = blocks.find(ptr);
    if (it == blocks.end() || it->second.encrypted) return false;
    
    it->second.encryptionKey = AdvancedCryptoEngine::GenerateSecureKey(256);
    
    std::vector<uint8_t> data(static_cast<uint8_t*>(ptr), 
                             static_cast<uint8_t*>(ptr) + it->second.size);
    
    auto& ctx = TitanShieldCore::GetInstance().GetSecurityContext();
    auto encrypted = AdvancedCryptoEngine::Encrypt(data, ctx);
    
    if (encrypted.size() <= it->second.size) {
        std::memcpy(ptr, encrypted.data(), encrypted.size());
        it->second.encrypted = true;
        return true;
    }
    
    return false;
}

bool SecureMemoryManager::DecryptMemoryBlock(void* ptr) {
    auto& blocks = GetSecureBlocks();
    std::lock_guard<std::mutex> lock(GetMemoryMutex());
    
    auto it = blocks.find(ptr);
    if (it == blocks.end() || !it->second.encrypted) return false;
    
    std::vector<uint8_t> data(static_cast<uint8_t*>(ptr), 
                             static_cast<uint8_t*>(ptr) + it->second.size);
    
    TitanError error;
    auto& ctx = TitanShieldCore::GetInstance().GetSecurityContext();
    auto decrypted = AdvancedCryptoEngine::Decrypt(data, ctx, error);
    
    if (error == TitanError::Success && !decrypted.empty()) {
        std::memcpy(ptr, decrypted.data(), std::min(decrypted.size(), it->second.size));
        it->second.encrypted = false;
        return true;
    }
    
    return false;
}

bool SecureMemoryManager::LockMemory(void* ptr) {
#ifdef _WIN32
    auto& blocks = GetSecureBlocks();
    std::lock_guard<std::mutex> lock(GetMemoryMutex());
    
    auto it = blocks.find(ptr);
    if (it == blocks.end()) return false;
    
    if (VirtualLock(ptr, it->second.size)) {
        it->second.locked = true;
        return true;
    }
#else
    auto& blocks = GetSecureBlocks();
    std::lock_guard<std::mutex> lock(GetMemoryMutex());
    
    auto it = blocks.find(ptr);
    if (it == blocks.end()) return false;
    
    if (mlock(ptr, it->second.size) == 0) {
        it->second.locked = true;
        return true;
    }
#endif
    return false;
}

bool SecureMemoryManager::UnlockMemory(void* ptr) {
#ifdef _WIN32
    auto& blocks = GetSecureBlocks();
    std::lock_guard<std::mutex> lock(GetMemoryMutex());
    
    auto it = blocks.find(ptr);
    if (it == blocks.end()) return false;
    
    if (VirtualUnlock(ptr, it->second.size)) {
        it->second.locked = false;
        return true;
    }
#else
    auto& blocks = GetSecureBlocks();
    std::lock_guard<std::mutex> lock(GetMemoryMutex());
    
    auto it = blocks.find(ptr);
    if (it == blocks.end()) return false;
    
    if (munlock(ptr, it->second.size) == 0) {
        it->second.locked = false;
        return true;
    }
#endif
    return false;
}

void SecureMemoryManager::WipeMemory(void* ptr, size_t size) {
    AdvancedCryptoEngine::SecureWipe(ptr, size);
}

bool SecureMemoryManager::ValidateMemoryIntegrity(void* ptr) {
    auto& blocks = GetSecureBlocks();
    std::lock_guard<std::mutex> lock(GetMemoryMutex());
    
    auto it = blocks.find(ptr);
    if (it == blocks.end()) return false;
    
    if (it->second.encrypted) return true;
    
    auto currentHash = RealSHA256::Hash(static_cast<uint8_t*>(ptr), it->second.size);
    
    return RealSHA256::ConstantTimeCompare(currentHash.data(), it->second.checksumHash.data(), 32);
}

void SecureMemoryManager::CleanupExpiredBlocks() {
    auto& blocks = GetSecureBlocks();
    std::lock_guard<std::mutex> lock(GetMemoryMutex());
    
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    uint64_t expirationTime = 3600000000000ULL;
    
    for (auto it = blocks.begin(); it != blocks.end(); ) {
        if (now - it->second.lastAccess > expirationTime) {
            WipeMemory(it->second.address, it->second.size);
            FreeWithGuards(it->second.address, it->second.size);
            it = blocks.erase(it);
        } else {
            ++it;
        }
    }
}

TitanShieldCore::TitanShieldCore()
    : initialized_(false), validated_(false), sessionToken_(0),
      operationCounter_(0), monitoringActive_(false),
      protectionLevel_(ProtectionLevel::Enhanced), sessionTimeout_(3600000),
      lastError_(TitanError::Success), canary_(0) {
    RegenerateCanary();
}

TitanShieldCore::~TitanShieldCore() {
    Shutdown();
}

TitanShieldCore& TitanShieldCore::GetInstance() {
    static TitanShieldCore instance;
    return instance;
}

bool TitanShieldCore::ValidateCanary() {
    volatile uint64_t current = canary_.load();
    return current != 0 && current != 0xDEADBEEFDEADBEEFULL;
}

void TitanShieldCore::RegenerateCanary() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    canary_.store(gen() ^ reinterpret_cast<uint64_t>(this));
}

bool TitanShieldCore::ExtractEmbeddedKey() {
    std::lock_guard<std::mutex> lock(coreMutex_);
    
    if (!PolymorphicKeyExtractor::ExtractKey256(masterKey_)) {
        lastError_ = TitanError::InvalidKey;
        return false;
    }
    
    if (masterKey_.size() != 32) {
        lastError_ = TitanError::InvalidKey;
        return false;
    }
    
    return true;
}

bool TitanShieldCore::BuildSecurityContext() {
    std::lock_guard<std::mutex> lock(coreMutex_);
    
#ifdef _WIN32
    securityContext_.processId = GetCurrentProcessId();
    securityContext_.threadId = GetCurrentThreadId();
#else
    securityContext_.processId = getpid();
    securityContext_.threadId = 0;
#endif
    
    securityContext_.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    std::random_device rd;
    std::mt19937_64 gen(rd());
    
    uint64_t hwFingerprint = HardwareInfo::GetCombinedFingerprint();
    gen.seed(gen() ^ hwFingerprint ^ securityContext_.timestamp);
    
    for (size_t i = 0; i < 16; i++) {
        securityContext_.entropy[i] = gen();
    }
    
    auto fingerprintData = AdvancedCryptoEngine::GenerateSecureKey(1024);
    std::memcpy(securityContext_.fingerprint, fingerprintData.data(), 128);
    
    std::memcpy(securityContext_.sessionIV, fingerprintData.data() + 128, 32);
    
    securityContext_.validUntil = securityContext_.timestamp + sessionTimeout_ * 1000000;
    securityContext_.validated = 1;
    securityContext_.securityLevel = static_cast<uint32_t>(protectionLevel_);
    
    securityContext_.canary1 = gen() ^ hwFingerprint;
    securityContext_.canary2 = gen() ^ securityContext_.timestamp;
    
    auto hmac = RealSHA256::HMAC(reinterpret_cast<uint8_t*>(&securityContext_), 
                                  sizeof(SecurityContext) - 32,
                                  masterKey_.data(), masterKey_.size());
    std::memcpy(securityContext_.hmac, hmac.data(), 32);
    
    return true;
}

bool TitanShiel​​​​​​​​​​​​​​​​dCore::EstablishSecureSession() {
    std::lock_guard<std::mutex> lock(coreMutex_);
    
    sessionKey_ = AdvancedCryptoEngine::DeriveSessionKey(masterKey_, securityContext_);
    
    if (!AdvancedCryptoEngine::Initialize(sessionKey_)) {
        lastError_ = TitanError::NotInitialized;
        return false;
    }
    
    sessionToken_ = RealSHA256::QuickHash64(sessionKey_.data(), sessionKey_.size());
    sessionStart_ = std::chrono::steady_clock::now();
    
    return true;
}

void TitanShieldCore::StartContinuousMonitoring() {
    monitoringActive_ = true;
    monitoringThread_ = std::thread(&TitanShieldCore::MonitoringLoop, this);
}

void TitanShieldCore::MonitoringLoop() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(300, 700);
    
    while (monitoringActive_) {
        if (!ValidateCanary()) {
            AntiReverseEngineering::TriggerAntiTamperHard();
        }
        
        if (!AntiReverseEngineering::PerformRuntimeChecks()) {
            AntiReverseEngineering::TriggerAntiTamperSoft();
        }
        
        if (!AntiReverseEngineering::VerifyMemoryTraps()) {
            AntiReverseEngineering::TriggerAntiTamperSoft();
        }
        
        if (!ValidateSession()) {
            RotateSessionKey();
        }
        
        int sleepTime = dis(gen);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        
        if (operationCounter_.load() % 1000 == 0) {
            RegenerateCanary();
        }
    }
}

bool TitanShieldCore::ValidateSession() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - sessionStart_).count();
    
    if (static_cast<uint64_t>(elapsed) > sessionTimeout_) {
        return false;
    }
    
    auto currentHmac = RealSHA256::HMAC(reinterpret_cast<uint8_t*>(&securityContext_), 
                                        sizeof(SecurityContext) - 32,
                                        masterKey_.data(), masterKey_.size());
    
    if (!RealSHA256::ConstantTimeCompare(currentHmac.data(), securityContext_.hmac, 32)) {
        return false;
    }
    
    return true;
}

void TitanShieldCore::RotateSessionKey() {
    std::lock_guard<std::mutex> lock(coreMutex_);
    
    AdvancedCryptoEngine::SecureWipe(sessionKey_.data(), sessionKey_.size());
    
    securityContext_.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    sessionKey_ = AdvancedCryptoEngine::DeriveSessionKey(masterKey_, securityContext_);
    
    AdvancedCryptoEngine::RotateSessionKey();
    
    sessionToken_ = RealSHA256::QuickHash64(sessionKey_.data(), sessionKey_.size());
    sessionStart_ = std::chrono::steady_clock::now();
}

bool TitanShieldCore::Initialize(ProtectionLevel::Level level) {
    std::lock_guard<std::mutex> lock(coreMutex_);
    
    if (initialized_) return true;
    
    protectionLevel_ = level;
    
    if (!HardwareInfo::Initialize()) {
        lastError_ = TitanError::NotInitialized;
        return false;
    }
    
    if (!AntiReverseEngineering::ValidateEnvironment()) {
        lastError_ = TitanError::VMDetected;
        return false;
    }
    
    if (!AntiReverseEngineering::InitializeProtection()) {
        lastError_ = TitanError::NotInitialized;
        return false;
    }
    
    if (!ExtractEmbeddedKey()) {
        return false;
    }
    
    if (!BuildSecurityContext()) {
        lastError_ = TitanError::InvalidContext;
        return false;
    }
    
    if (!EstablishSecureSession()) {
        return false;
    }
    
    StartContinuousMonitoring();
    
    initialized_ = true;
    validated_ = true;
    
    return true;
}

bool TitanShieldCore::Reinitialize() {
    Shutdown();
    return Initialize(protectionLevel_);
}

bool TitanShieldCore::Validate() {
    if (!initialized_) return false;
    
    if (!ValidateCanary()) return false;
    
    if (!ValidateSession()) return false;
    
    return validated_ && AntiReverseEngineering::PerformRuntimeChecks();
}

bool TitanShieldCore::Shutdown() {
    std::lock_guard<std::mutex> lock(coreMutex_);
    
    monitoringActive_ = false;
    if (monitoringThread_.joinable()) {
        monitoringThread_.join();
    }
    
    AdvancedCryptoEngine::Shutdown();
    AdvancedCryptoEngine::SecureWipe(masterKey_.data(), masterKey_.size());
    AdvancedCryptoEngine::SecureWipe(sessionKey_.data(), sessionKey_.size());
    
    initialized_ = false;
    validated_ = false;
    
    return true;
}

bool TitanShieldCore::IsProtected() const {
    return initialized_ && validated_;
}

ProtectionLevel::Level TitanShieldCore::GetProtectionLevel() const {
    return protectionLevel_;
}

const std::vector<uint8_t>& TitanShieldCore::GetMasterKey() const {
    return masterKey_;
}

const std::vector<uint8_t>& TitanShieldCore::GetSessionKey() const {
    return sessionKey_;
}

const SecurityContext& TitanShieldCore::GetSecurityContext() const {
    return securityContext_;
}

uint64_t TitanShieldCore::GetSessionToken() const {
    return sessionToken_;
}

uint64_t TitanShieldCore::IncrementOperationCounter() {
    return operationCounter_.fetch_add(1);
}

TitanError TitanShieldCore::GetLastError() const {
    return lastError_;
}

void TitanShieldCore::SetSessionTimeout(uint64_t milliseconds) {
    sessionTimeout_ = milliseconds;
}

bool TitanShieldCore::RefreshSession() {
    if (!initialized_) return false;
    
    RotateSessionKey();
    return true;
}

bool Initialize(ProtectionLevel::Level level) {
    return TitanShieldCore::GetInstance().Initialize(level);
}

bool Reinitialize() {
    return TitanShieldCore::GetInstance().Reinitialize();
}

bool Validate() {
    return TitanShieldCore::GetInstance().Validate();
}

bool Shutdown() {
    return TitanShieldCore::GetInstance().Shutdown();
}

std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& data) {
    auto& core = TitanShieldCore::GetInstance();
    if (!core.IsProtected()) return data;
    
    core.IncrementOperationCounter();
    return AdvancedCryptoEngine::Encrypt(data, core.GetSecurityContext());
}

std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& data) {
    auto& core = TitanShieldCore::GetInstance();
    if (!core.IsProtected()) return data;
    
    core.IncrementOperationCounter();
    TitanError error;
    return AdvancedCryptoEngine::Decrypt(data, core.GetSecurityContext(), error);
}

std::vector<uint8_t> EncryptWithContext(const std::vector<uint8_t>& data, const SecurityContext& ctx) {
    auto& core = TitanShieldCore::GetInstance();
    if (!core.IsProtected()) return data;
    
    return AdvancedCryptoEngine::Encrypt(data, ctx);
}

std::vector<uint8_t> DecryptWithContext(const std::vector<uint8_t>& data, const SecurityContext& ctx) {
    auto& core = TitanShieldCore::GetInstance();
    if (!core.IsProtected()) return data;
    
    TitanError error;
    return AdvancedCryptoEngine::Decrypt(data, ctx, error);
}

std::string ProtectString(const std::string& str) {
    std::vector<uint8_t> data(str.begin(), str.end());
    auto encrypted = Encrypt(data);
    
    std::string result;
    result.reserve(encrypted.size() * 2);
    for (uint8_t byte : encrypted) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", byte);
        result += hex;
    }
    
    return result;
}

std::string UnprotectString(const std::string& str) {
    std::vector<uint8_t> encrypted;
    for (size_t i = 0; i < str.length(); i += 2) {
        std::string byteStr = str.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(strtol(byteStr.c_str(), nullptr, 16));
        encrypted.push_back(byte);
    }
    
    auto decrypted = Decrypt(encrypted);
    return std::string(decrypted.begin(), decrypted.end());
}

std::wstring ProtectWString(const std::wstring& str) {
    std::vector<uint8_t> data(reinterpret_cast<const uint8_t*>(str.data()), 
                              reinterpret_cast<const uint8_t*>(str.data() + str.size()));
    auto encrypted = Encrypt(data);
    
    std::wstring result;
    for (uint8_t byte : encrypted) {
        result += static_cast<wchar_t>(byte);
    }
    
    return result;
}

std::wstring UnprotectWString(const std::wstring& str) {
    std::vector<uint8_t> encrypted;
    for (wchar_t wc : str) {
        encrypted.push_back(static_cast<uint8_t>(wc));
    }
    
    auto decrypted = Decrypt(encrypted);
    
    std::wstring result;
    const wchar_t* wchars = reinterpret_cast<const wchar_t*>(decrypted.data());
    size_t wcharCount = decrypted.size() / sizeof(wchar_t);
    result.assign(wchars, wcharCount);
    
    return result;
}

bool EncryptFile(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile.is_open()) return false;
    
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(inFile)),
                              std::istreambuf_iterator<char>());
    inFile.close();
    
    auto encrypted = Encrypt(data);
    
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) return false;
    
    outFile.write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
    outFile.close();
    
    return true;
}

bool DecryptFile(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile.is_open()) return false;
    
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(inFile)),
                              std::istreambuf_iterator<char>());
    inFile.close();
    
    auto decrypted = Decrypt(data);
    
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) return false;
    
    outFile.write(reinterpret_cast<const char*>(decrypted.data()), decrypted.size());
    outFile.close();
    
    return true;
}

void* SecureAlloc(size_t size) {
    return SecureMemoryManager::AllocateSecure(size);
}

void SecureFree(void* ptr) {
    SecureMemoryManager::FreeSecure(ptr);
}

bool ProtectMemoryRegion(void* address, size_t size) {
    return SecureMemoryManager::ProtectMemory(address, size);
}

bool UnprotectMemoryRegion(void* address, size_t size) {
    return SecureMemoryManager::UnprotectMemory(address, size);
}

bool DetectCheatEngine() {
#ifdef _WIN32
    HWND hwnd = FindWindowA(NULL, "Cheat Engine");
    if (hwnd != NULL) return true;
    
    hwnd = FindWindowA("Qt5QWindowIcon", NULL);
    if (hwnd != NULL) return true;
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;
    
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    
    bool detected = false;
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, "cheatengine-x86_64.exe") == 0 ||
                _stricmp(pe.szExeFile, "cheatengine-i386.exe") == 0) {
                detected = true;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    
    CloseHandle(hSnapshot);
    return detected;
#else
    return false;
#endif
}

bool DetectMemoryScanner() {
    return DetectCheatEngine() || AntiReverseEngineering::DetectMemoryDumpingTiming();
}

bool ValidateGameIntegrity() {
    return AntiReverseEngineering::PerformRuntimeChecks() && 
           AntiReverseEngineering::VerifyMemoryTraps();
}

void RegisterCriticalMemory(void* address, size_t size) {
    AntiReverseEngineering::RegisterMemoryTrap(address, size);
}

bool RefreshSession() {
    return TitanShieldCore::GetInstance().RefreshSession();
}

void SetSessionTimeout(uint64_t milliseconds) {
    TitanShieldCore::GetInstance().SetSessionTimeout(milliseconds);
}

uint64_t GetSessionToken() {
    return TitanShieldCore::GetInstance().GetSessionToken();
}

bool IsProtected() {
    return TitanShieldCore::GetInstance().IsProtected();
}

TitanError GetLastError() {
    return TitanShieldCore::GetInstance().GetLastError();
}

uint32_t GetThreatLevel() {
    return AntiReverseEngineering::GetDetectionScore();
}

void InjectEntropy() {
    auto& core = TitanShieldCore::GetInstance();
    core.IncrementOperationCounter();
}

bool PerformIntegrityCheck() {
    return AntiReverseEngineering::PerformRuntimeChecks();
}

bool ValidateEnvironment() {
    return AntiReverseEngineering::ValidateEnvironment();
}

void EnableContinuousProtection() {
    AntiReverseEngineering::ContinuousMonitoring();
}

SecurityContext CreateSecurityContext() {
    return TitanShieldCore::GetInstance().GetSecurityContext();
}

bool ValidateSecurityContext(const SecurityContext& ctx) {
    if (ctx.validated != 1) return false;
    
#ifdef _WIN32
    if (ctx.processId != GetCurrentProcessId()) return false;
#else
    if (ctx.processId != getpid()) return false;
#endif
    
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    if (static_cast<uint64_t>(now) > ctx.validUntil) return false;
    
    auto& core = TitanShieldCore::GetInstance();
    auto hmac = RealSHA256::HMAC(reinterpret_cast<const uint8_t*>(&ctx), 
                                  sizeof(SecurityContext) - 32,
                                  core.GetMasterKey().data(), 
                                  core.GetMasterKey().size());
    
    return RealSHA256::ConstantTimeCompare(hmac.data(), ctx.hmac, 32);
}

}

