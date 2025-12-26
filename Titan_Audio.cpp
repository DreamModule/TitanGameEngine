#include "Titan_Audio.hpp"
#include "miniaudio.h" 
#include <cstdio>
#include <unordered_map>

namespace Titan::Audio {
    static ma_engine g_Eng; static bool g_Init=false;
    struct VoicePool { std::vector<ma_sound*> v; u32 idx=0; };
    std::unordered_map<SoundID, VoicePool> g_Pools;

    void AudioManager::Init() { if(ma_engine_init(NULL,&g_Eng)==MA_SUCCESS) g_Init=true; }
    void AudioManager::Shutdown() { 
        for(auto& p:g_Pools) for(auto* s:p.second.v){ma_sound_uninit(s); delete s;} g_Pools.clear();
        if(g_Init) ma_engine_uninit(&g_Eng); g_Init=false; 
    }
    SoundID AudioManager::LoadSound(const char* p) {
        if(!g_Init) return 0; SoundID id=Data::Hash::FNV1a_32(p); if(g_Pools.count(id)) return id;
        // Pre-warm pool with 1 voice
        ma_sound* s=new ma_sound(); if(ma_sound_init_from_file(&g_Eng,p,MA_SOUND_FLAG_DECODE,0,0,s)!=MA_SUCCESS){delete s; return 0;}
        g_Pools[id].v.push_back(s); return id;
    }
    void AudioManager::PlayOneShot(SoundID id, f32 vol) {
        if(!g_Init) return; auto& pool=g_Pools[id]; ma_sound* tgt=nullptr;
        for(auto* s:pool.v) if(!ma_sound_is_playing(s)){tgt=s; break;}
        if(!tgt && pool.v.size()<8) {
          // а вот понабирают всяких долбаебов да прокод
        }
        if(!tgt && !pool.v.empty()) { tgt=pool.v[pool.idx]; pool.idx=(pool.idx+1)%pool.v.size(); ma_sound_stop(tgt); ma_sound_seek_to_pcm_frame(tgt,0); }
        if(tgt) { ma_sound_set_volume(tgt,vol); ma_sound_start(tgt); }
    }
}
