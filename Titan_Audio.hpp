/**
 * Titan Audio Header
 * 
 * Audio playback system
 */

#ifndef TITAN_AUDIO_HPP
#define TITAN_AUDIO_HPP

#include "Titan_Core.hpp"
#include <vector>
#include <string>

namespace Titan::Audio {

// ============================================================================
// Types
// ============================================================================

using SoundID = uint32;

constexpr SoundID INVALID_SOUND = 0;

struct SoundClip
{
    SoundID ID = INVALID_SOUND;
    uint32 SampleRate = 44100;
    uint32 Channels = 2;
    std::vector<uint8> Data;
    std::string Name;
};

// ============================================================================
// Audio Manager
// ============================================================================

struct AudioManager
{
    std::vector<SoundClip> Clips;
    bool bInitialized = false;

    /**
     * Initialize the audio system
     */
    void Init();

    /**
     * Shutdown the audio system
     */
    void Shutdown();

    /**
     * Load a sound from file
     * @param Path - Path to audio file (WAV, MP3, etc.)
     * @return Sound ID or INVALID_SOUND on failure
     */
    SoundID LoadSound(const char* Path);

    /**
     * Unload a sound
     */
    void UnloadSound(SoundID ID);

    /**
     * Play a sound once
     * @param ID - Sound to play
     * @param Volume - Volume (0.0 to 1.0)
     */
    void PlayOneShot(SoundID ID, float Volume = 1.0f);

    /**
     * Play a sound with looping
     */
    void PlayLooped(SoundID ID, float Volume = 1.0f);

    /**
     * Stop all sounds
     */
    void StopAll();

    /**
     * Set master volume
     */
    void SetMasterVolume(float Volume);
};

} // namespace Titan::Audio

#endif // TITAN_AUDIO_HPP
