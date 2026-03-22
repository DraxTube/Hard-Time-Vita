#pragma once
/*
 * vita_audio.h  –  Audio engine interface (implementation in src/vita_audio.cpp)
 * No PSP2 headers here - they're only needed in the .cpp implementation.
 */
#include <cstdint>
#include <array>
#include <atomic>
#include <algorithm>
#include <cstring>
#include <cmath>

struct VitaSound {
    int16_t* samples     = nullptr;
    int      n_samples   = 0;
    int      sample_rate = 44100;
    int      channels    = 1;
    bool     loaded      = false;
};

class VitaAudioEngine {
public:
    static VitaAudioEngine& get();

    void init();
    void shutdown();

    VitaSound* load(const char* path);
    void       free_sound(VitaSound* s);

    int  play(const VitaSound* s, float vol=1.f, float pitch=1.f,
              bool loop=false, bool is3d=false,
              float x=0, float y=0, float z=0);
    void stop(int ch);
    bool playing(int ch) const;
    void set_volume(int ch, float v);
    void set_pitch(int ch, float p);
    void set_listener(float x, float y, float z);

private:
    // Forward-declared - implementation in vita_audio.cpp
    struct Impl;
    static Impl* s_impl;
};
