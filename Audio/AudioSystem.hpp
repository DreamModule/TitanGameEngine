#ifndef TITAN_AUDIO_SYSTEM_HPP
#define TITAN_AUDIO_SYSTEM_HPP

#define MINIAUDIO_IMPLEMENTATION
#include "../../miniaudio.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>

namespace Titan {
namespace Audio {

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float Length() const { return sqrtf(x * x + y * y + z * z); }
    Vec3 Normalized() const { float l = Length(); return l > 0.0001f ? Vec3{x/l, y/l, z/l} : *this; }
    float Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 Cross(const Vec3& o) const { return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x}; }
};

struct SoundData {
    uint32_t id = 0;
    std::string name;
    std::vector<float> samples;
    uint32_t sampleRate = 44100;
    uint32_t channels = 2;
    float duration = 0;
    bool loaded = false;
};

struct SoundInstance {
    uint32_t id = 0;
    uint32_t soundId = 0;
    float volume = 1.0f;
    float pitch = 1.0f;
    float pan = 0.0f;
    bool loop = false;
    bool paused = false;
    bool stopped = false;
    bool is3D = false;
    Vec3 position;
    float minDistance = 1.0f;
    float maxDistance = 100.0f;
    float rolloff = 1.0f;
    size_t samplePosition = 0;
    float fractionalPosition = 0;
};

struct Listener {
    Vec3 position;
    Vec3 forward = {0, 0, -1};
    Vec3 up = {0, 1, 0};
    Vec3 velocity;
};

class AudioEngine {
public:
    bool Init() {
        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.format = ma_format_f32;
        config.playback.channels = 2;
        config.sampleRate = 44100;
        config.dataCallback = DataCallback;
        config.pUserData = this;
        
        if (ma_device_init(nullptr, &config, &m_device) != MA_SUCCESS) {
            printf("[Audio] Failed to initialize audio device\n");
            return false;
        }
        
        if (ma_device_start(&m_device) != MA_SUCCESS) {
            printf("[Audio] Failed to start audio device\n");
            ma_device_uninit(&m_device);
            return false;
        }
        
        m_initialized = true;
        m_masterVolume = 1.0f;
        m_sfxVolume = 1.0f;
        m_musicVolume = 1.0f;
        
        printf("[Audio] Audio engine initialized (44100 Hz, stereo)\n");
        return true;
    }
    
    void Shutdown() {
        if (!m_initialized) return;
        
        ma_device_uninit(&m_device);
        m_sounds.clear();
        m_instances.clear();
        m_initialized = false;
        
        printf("[Audio] Audio engine shutdown\n");
    }
    
    uint32_t LoadSound(const char* path) {
        std::string key = path;
        for (auto& s : m_sounds) {
            if (s.name == key) return s.id;
        }
        
        SoundData sound;
        sound.id = ++m_nextSoundId;
        sound.name = path;
        
        std::string ext = path;
        size_t dot = ext.find_last_of('.');
        if (dot != std::string::npos) ext = ext.substr(dot + 1);
        
        ma_decoder decoder;
        ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 2, 44100);
        
        if (ma_decoder_init_file(path, &decoderConfig, &decoder) != MA_SUCCESS) {
            printf("[Audio] Failed to load: %s\n", path);
            return 0;
        }
        
        ma_uint64 frameCount;
        ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);
        
        sound.samples.resize(frameCount * 2);
        ma_uint64 framesRead;
        ma_decoder_read_pcm_frames(&decoder, sound.samples.data(), frameCount, &framesRead);
        
        ma_decoder_uninit(&decoder);
        
        sound.sampleRate = 44100;
        sound.channels = 2;
        sound.duration = (float)frameCount / 44100.0f;
        sound.loaded = true;
        
        m_sounds.push_back(sound);
        
        printf("[Audio] Loaded: %s (%.2fs)\n", path, sound.duration);
        return sound.id;
    }
    
    uint32_t LoadSoundFromMemory(const char* name, const float* samples, size_t sampleCount, uint32_t sampleRate, uint32_t channels) {
        SoundData sound;
        sound.id = ++m_nextSoundId;
        sound.name = name;
        sound.samples.assign(samples, samples + sampleCount);
        sound.sampleRate = sampleRate;
        sound.channels = channels;
        sound.duration = (float)(sampleCount / channels) / sampleRate;
        sound.loaded = true;
        
        m_sounds.push_back(sound);
        return sound.id;
    }
    
    uint32_t Play(uint32_t soundId, float volume = 1.0f, float pitch = 1.0f, bool loop = false) {
        SoundData* sound = GetSound(soundId);
        if (!sound) return 0;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        SoundInstance instance;
        instance.id = ++m_nextInstanceId;
        instance.soundId = soundId;
        instance.volume = volume;
        instance.pitch = pitch;
        instance.loop = loop;
        instance.is3D = false;
        
        m_instances.push_back(instance);
        return instance.id;
    }
    
    uint32_t Play3D(uint32_t soundId, const Vec3& position, float volume = 1.0f, float pitch = 1.0f, bool loop = false) {
        SoundData* sound = GetSound(soundId);
        if (!sound) return 0;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        SoundInstance instance;
        instance.id = ++m_nextInstanceId;
        instance.soundId = soundId;
        instance.volume = volume;
        instance.pitch = pitch;
        instance.loop = loop;
        instance.is3D = true;
        instance.position = position;
        
        m_instances.push_back(instance);
        return instance.id;
    }
    
    void Stop(uint32_t instanceId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& inst : m_instances) {
            if (inst.id == instanceId) {
                inst.stopped = true;
                return;
            }
        }
    }
    
    void StopAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& inst : m_instances) {
            inst.stopped = true;
        }
    }
    
    void Pause(uint32_t instanceId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& inst : m_instances) {
            if (inst.id == instanceId) {
                inst.paused = true;
                return;
            }
        }
    }
    
    void Resume(uint32_t instanceId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& inst : m_instances) {
            if (inst.id == instanceId) {
                inst.paused = false;
                return;
            }
        }
    }
    
    void SetInstanceVolume(uint32_t instanceId, float volume) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& inst : m_instances) {
            if (inst.id == instanceId) {
                inst.volume = volume;
                return;
            }
        }
    }
    
    void SetInstancePosition(uint32_t instanceId, const Vec3& position) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& inst : m_instances) {
            if (inst.id == instanceId) {
                inst.position = position;
                return;
            }
        }
    }
    
    void SetListener(const Vec3& position, const Vec3& forward, const Vec3& up) {
        m_listener.position = position;
        m_listener.forward = forward.Normalized();
        m_listener.up = up.Normalized();
    }
    
    void SetMasterVolume(float volume) { m_masterVolume = volume; }
    void SetSFXVolume(float volume) { m_sfxVolume = volume; }
    void SetMusicVolume(float volume) { m_musicVolume = volume; }
    
    float GetMasterVolume() const { return m_masterVolume; }
    float GetSFXVolume() const { return m_sfxVolume; }
    float GetMusicVolume() const { return m_musicVolume; }
    
    bool IsPlaying(uint32_t instanceId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& inst : m_instances) {
            if (inst.id == instanceId && !inst.stopped && !inst.paused) {
                return true;
            }
        }
        return false;
    }
    
    static uint32_t GenerateSineWave(AudioEngine& engine, float frequency, float duration, float amplitude = 0.5f) {
        uint32_t sampleRate = 44100;
        size_t totalSamples = (size_t)(duration * sampleRate * 2);
        std::vector<float> samples(totalSamples);
        
        for (size_t i = 0; i < totalSamples / 2; i++) {
            float t = (float)i / sampleRate;
            float value = sinf(2.0f * 3.14159f * frequency * t) * amplitude;
            samples[i * 2] = value;
            samples[i * 2 + 1] = value;
        }
        
        return engine.LoadSoundFromMemory("generated_sine", samples.data(), totalSamples, sampleRate, 2);
    }
    
    static uint32_t GenerateNoise(AudioEngine& engine, float duration, float amplitude = 0.3f) {
        uint32_t sampleRate = 44100;
        size_t totalSamples = (size_t)(duration * sampleRate * 2);
        std::vector<float> samples(totalSamples);
        
        for (size_t i = 0; i < totalSamples; i++) {
            samples[i] = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * amplitude;
        }
        
        return engine.LoadSoundFromMemory("generated_noise", samples.data(), totalSamples, sampleRate, 2);
    }

private:
    static void DataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
        AudioEngine* engine = (AudioEngine*)device->pUserData;
        engine->ProcessAudio((float*)output, frameCount);
        (void)input;
    }
    
    void ProcessAudio(float* output, uint32_t frameCount) {
        memset(output, 0, frameCount * 2 * sizeof(float));
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto it = m_instances.begin(); it != m_instances.end();) {
            if (it->stopped) {
                it = m_instances.erase(it);
                continue;
            }
            
            if (it->paused) {
                ++it;
                continue;
            }
            
            SoundData* sound = GetSound(it->soundId);
            if (!sound || sound->samples.empty()) {
                it = m_instances.erase(it);
                continue;
            }
            
            float leftVol = it->volume * m_masterVolume * m_sfxVolume;
            float rightVol = leftVol;
            
            if (it->is3D) {
                Calculate3DAudio(*it, leftVol, rightVol);
            } else {
                float panL = fmaxf(0, -it->pan + 1) * 0.5f;
                float panR = fmaxf(0, it->pan + 1) * 0.5f;
                leftVol *= panL;
                rightVol *= panR;
            }
            
            size_t soundFrames = sound->samples.size() / 2;
            float pitchFactor = it->pitch * (float)sound->sampleRate / 44100.0f;
            
            for (uint32_t i = 0; i < frameCount; i++) {
                size_t sampleIdx = it->samplePosition;
                
                if (sampleIdx >= soundFrames) {
                    if (it->loop) {
                        it->samplePosition = 0;
                        sampleIdx = 0;
                    } else {
                        it->stopped = true;
                        break;
                    }
                }
                
                float left = sound->samples[sampleIdx * 2];
                float right = sound->samples[sampleIdx * 2 + 1];
                
                output[i * 2] += left * leftVol;
                output[i * 2 + 1] += right * rightVol;
                
                it->fractionalPosition += pitchFactor;
                while (it->fractionalPosition >= 1.0f) {
                    it->samplePosition++;
                    it->fractionalPosition -= 1.0f;
                }
            }
            
            ++it;
        }
        
        for (uint32_t i = 0; i < frameCount * 2; i++) {
            if (output[i] > 1.0f) output[i] = 1.0f;
            if (output[i] < -1.0f) output[i] = -1.0f;
        }
    }
    
    void Calculate3DAudio(const SoundInstance& instance, float& leftVol, float& rightVol) {
        Vec3 toSound = instance.position - m_listener.position;
        float distance = toSound.Length();
        
        float attenuation = 1.0f;
        if (distance > instance.minDistance) {
            if (distance >= instance.maxDistance) {
                attenuation = 0;
            } else {
                float range = instance.maxDistance - instance.minDistance;
                float t = (distance - instance.minDistance) / range;
                attenuation = 1.0f - powf(t, instance.rolloff);
            }
        }
        
        float pan = 0;
        if (distance > 0.01f) {
            Vec3 dir = toSound.Normalized();
            Vec3 right = m_listener.forward.Cross(m_listener.up).Normalized();
            pan = dir.Dot(right);
        }
        
        float panL = fmaxf(0, -pan + 1) * 0.5f;
        float panR = fmaxf(0, pan + 1) * 0.5f;
        
        leftVol *= attenuation * panL * 2.0f;
        rightVol *= attenuation * panR * 2.0f;
    }
    
    SoundData* GetSound(uint32_t id) {
        for (auto& s : m_sounds) {
            if (s.id == id) return &s;
        }
        return nullptr;
    }
    
    ma_device m_device;
    bool m_initialized = false;
    
    std::vector<SoundData> m_sounds;
    std::vector<SoundInstance> m_instances;
    std::mutex m_mutex;
    
    Listener m_listener;
    
    float m_masterVolume = 1.0f;
    float m_sfxVolume = 1.0f;
    float m_musicVolume = 1.0f;
    
    uint32_t m_nextSoundId = 0;
    uint32_t m_nextInstanceId = 0;
};

}
}

#endif


