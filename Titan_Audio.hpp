#ifndef TITAN_AUDIO_HPP
#define TITAN_AUDIO_HPP
#include "Titan_Core.hpp"
#include <vector>

namespace Titan::Audio {
    using SoundID = u32;
    struct SoundClip {
        SoundID id;
        u32 sampleRate;
        u32 channels;
        std::vector<u8> data; // PCM Data
    };

    struct AudioManager {
        std::vector<SoundClip> clips;
        bool initialized = false;
        void Init(); 
        void Shutdown();
        SoundID LoadSound(const char* path);
        void PlayOneShot(SoundID id, f32 volume = 1.0f);
    };
}
#endif
