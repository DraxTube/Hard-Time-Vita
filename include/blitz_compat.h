#pragma once
// =============================================================================
//  blitz_compat.h  –  Compatibility layer: Blitz3D API → PS Vita (C++)
//  Maps the original Blitz3D built-ins to vitaGL / SceCtrl / SceAudio equivalents
// =============================================================================

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include "debug_log.h"
#include <algorithm>
#include <memory>
#include <random>

// Vita SDK headers
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/audioout.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/power.h>
#include <psp2/sysmodule.h>
#include <vitaGL.h>

#include "../third_party/vita_audio.h"

// ---------------------------------------------------------------------------
//  Type aliases
// ---------------------------------------------------------------------------
using BBString = std::string;
using BBInt    = int;
using BBFloat  = float;

// ---------------------------------------------------------------------------
//  Screen / Resolution
//  PS Vita native: 960×544
// ---------------------------------------------------------------------------
static constexpr int VITA_SCREEN_W = 960;
static constexpr int VITA_SCREEN_H = 544;

// Blitz3D reference resolution was 800×600; scale helpers
inline float rX(float x) { return x * (VITA_SCREEN_W / 800.0f); }
inline float rY(float y) { return y * (VITA_SCREEN_H / 600.0f); }

// ---------------------------------------------------------------------------
//  Random
// ---------------------------------------------------------------------------
namespace BB {
    static std::mt19937 rng{ static_cast<uint32_t>(sceKernelGetProcessTimeLow()) };

    inline void SeedRnd(int seed) { rng.seed(seed); }

    inline int Rnd(int lo, int hi) {
        if (lo > hi) std::swap(lo, hi);
        return std::uniform_int_distribution<int>(lo, hi)(rng);
    }
    inline int Rnd(int n)       { return Rnd(0, n - 1); }
    inline float RndF(float lo, float hi) {
        return std::uniform_real_distribution<float>(lo, hi)(rng);
    }

    // ---------------------------------------------------------------------------
    //  Math helpers (match Blitz3D naming)
    // ---------------------------------------------------------------------------
    inline float Abs(float v)        { return std::fabs(v); }
    inline float Sgn(float v)        { return (v > 0) - (v < 0); }
    inline int   Int(float v)        { return static_cast<int>(v); }
    inline float Float(int v)        { return static_cast<float>(v); }
    inline float Sqr(float v)        { return std::sqrt(v); }
    inline float Sin(float deg)      { return std::sin(deg * M_PI / 180.0f); }
    inline float Cos(float deg)      { return std::cos(deg * M_PI / 180.0f); }
    inline float ATan2(float y, float x) { return std::atan2(y, x) * 180.0f / M_PI; }
    inline float Floor(float v)      { return std::floor(v); }
    inline float Ceil(float v)       { return std::ceil(v); }
    inline int   MilliSecs()         { return static_cast<int>(sceKernelGetProcessTimeLow() / 1000); }

    // ---------------------------------------------------------------------------
    //  String helpers (match Blitz3D built-ins)
    // ---------------------------------------------------------------------------
    inline BBString Str(int v)       { return std::to_string(v); }
    inline BBString Str(float v)     { return std::to_string(v); }
    inline int      Val(const BBString& s) { return std::stoi(s); }
    inline BBString Left(const BBString& s, int n)  { return s.substr(0, n); }
    inline BBString Right(const BBString& s, int n) { return s.substr(s.size() - n); }
    inline BBString Mid(const BBString& s, int pos, int len = -1) {
        return (len < 0) ? s.substr(pos - 1) : s.substr(pos - 1, len);
    }
    inline int      Len(const BBString& s)  { return static_cast<int>(s.size()); }
    inline BBString Upper(BBString s)        { std::transform(s.begin(), s.end(), s.begin(), ::toupper); return s; }
    inline BBString Lower(BBString s)        { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }
    inline int      Instr(const BBString& s, const BBString& sub) {
        auto pos = s.find(sub);
        return (pos == BBString::npos) ? 0 : static_cast<int>(pos) + 1;
    }
    inline BBString LSet(BBString s, int n) { s.resize(n, ' '); return s; }
    inline char     Chr(int code)   { return static_cast<char>(code); }
    inline int      Asc(const BBString& s) { return s.empty() ? 0 : static_cast<unsigned char>(s[0]); }

    // ---------------------------------------------------------------------------
    //  File I/O  (maps to PSVita sceIo*)
    // ---------------------------------------------------------------------------
    struct BBFile {
        SceUID fd  = -1;
        bool   writing = false;
    };

    inline BBFile* WriteFile(const BBString& path) {
        auto* f = new BBFile();
        f->fd = sceIoOpen(path.c_str(), SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0644);
        f->writing = true;
        return f;
    }
    inline BBFile* ReadFile(const BBString& path) {
        auto* f = new BBFile();
        f->fd = sceIoOpen(path.c_str(), SCE_O_RDONLY, 0);
        f->writing = false;
        return f;
    }
    inline void CloseFile(BBFile* f) {
        if (f && f->fd >= 0) sceIoClose(f->fd);
        delete f;
    }
    inline void WriteInt(BBFile* f, int v) {
        sceIoWrite(f->fd, &v, sizeof(int));
    }
    inline int ReadInt(BBFile* f) {
        int v = 0;
        sceIoRead(f->fd, &v, sizeof(int));
        return v;
    }
    inline void WriteFloat(BBFile* f, float v) {
        sceIoWrite(f->fd, &v, sizeof(float));
    }
    inline float ReadFloat(BBFile* f) {
        float v = 0;
        sceIoRead(f->fd, &v, sizeof(float));
        return v;
    }
    inline void WriteString(BBFile* f, const BBString& s) {
        int len = static_cast<int>(s.size());
        sceIoWrite(f->fd, &len, sizeof(int));
        sceIoWrite(f->fd, s.c_str(), len);
    }
    inline BBString ReadString(BBFile* f) {
        int len = 0;
        sceIoRead(f->fd, &len, sizeof(int));
        BBString s(len, '\0');
        sceIoRead(f->fd, &s[0], len);
        return s;
    }
    inline bool FileExists(const BBString& path) {
        SceIoStat st;
        return sceIoGetstat(path.c_str(), &st) >= 0;
    }

    // ---------------------------------------------------------------------------
    //  Controller Input (replaces Blitz3D keyboard + DirectInput gamepad)
    //
    //  Original button mapping → Vita buttons:
    //    buttAttack  (1) → Cross    (SCE_CTRL_CROSS)
    //    buttDefend  (3) → Circle   (SCE_CTRL_CIRCLE)
    //    buttThrow   (2) → Square   (SCE_CTRL_SQUARE)
    //    buttPickUp  (4) → Triangle (SCE_CTRL_TRIANGLE)
    //
    //  Analogue sticks replace keyboard WASD / arrow movement.
    // ---------------------------------------------------------------------------
    struct Input {
        SceCtrlData pad;
        SceCtrlData prevPad;

        void Update() {
            prevPad = pad;
            sceCtrlPeekBufferPositive(0, &pad, 1);
        }

        bool Held(SceCtrlButtons btn)    const { return (pad.buttons     & btn) != 0; }
        bool Pressed(SceCtrlButtons btn) const { return  (pad.buttons & btn) && !(prevPad.buttons & btn); }
        bool Released(SceCtrlButtons btn)const { return !(pad.buttons & btn) &&  (prevPad.buttons & btn); }

        float LeftX()  const { return (pad.lx  - 128) / 128.0f; }
        float LeftY()  const { return (pad.ly  - 128) / 128.0f; }
        float RightX() const { return (pad.rx  - 128) / 128.0f; }
        float RightY() const { return (pad.ry  - 128) / 128.0f; }
    };

    extern Input gInput;  // global input state defined in main.cpp

    // ---------------------------------------------------------------------------
    // ---------------------------------------------------------------------------
    //  Audio – real SceAudio implementation via VitaAudioEngine
    // ---------------------------------------------------------------------------

    struct BBSound {
        VitaSound* data    = nullptr;
        float      volume  = 1.0f;
        int        pitch   = 100;     // 100 = normal
        bool       is3d    = false;
        int        channel = -1;      // currently playing channel index, -1 if stopped
    };

    inline BBSound* LoadSound(const BBString& path) {
        if (g_debugLog) { fprintf(g_debugLog, "[SND] LoadSound: %s\n", path.c_str()); fflush(g_debugLog); }
        auto* s = new BBSound();
        s->data = VitaAudioEngine::get().load(path.c_str());
        if (g_debugLog) { fprintf(g_debugLog, "[SND] LoadSound OK: %s (loaded=%d)\n", path.c_str(), s->data ? s->data->loaded : -1); fflush(g_debugLog); }
        return s;
    }
    inline BBSound* Load3DSound(const BBString& path) {
        auto* s = LoadSound(path);
        if (s) s->is3d = true;
        return s;
    }
    inline void PlaySound(BBSound* s) {
        if (!s || !s->data) return;
        float pitchMul = s->pitch / 100.0f;
        s->channel = VitaAudioEngine::get().play(s->data, s->volume, pitchMul,
                                                  false, s->is3d);
    }
    inline void LoopSound(BBSound* s) {
        if (!s || !s->data) return;
        s->channel = VitaAudioEngine::get().play(s->data, s->volume, 1.0f, /*loop=*/true, false);
    }
    inline int PlaySoundLoop(BBSound* s, bool loop) {   // play with loop control, returns channel
        if (!s || !s->data) return -1;
        s->channel = VitaAudioEngine::get().play(s->data, s->volume, 1.0f, loop, false);
        return s->channel;
    }
    inline void StopSound(BBSound* s) {
        if (s && s->channel >= 0) {
            VitaAudioEngine::get().stop(s->channel);
            s->channel = -1;
        }
    }
    inline void StopChannel(int ch) { VitaAudioEngine::get().stop(ch); }
    inline bool ChannelPlaying(int ch) { return VitaAudioEngine::get().playing(ch); }
    inline void ChannelVolume(int ch, float v) { VitaAudioEngine::get().set_volume(ch, v); }
    inline void SoundVolume(BBSound* s, float vol) { if(s)s->volume=vol; if(s&&s->channel>=0)VitaAudioEngine::get().set_volume(s->channel,vol); }
    inline void SoundPitch(BBSound* s, int p)  { if(s)s->pitch=p; if(s&&s->channel>=0)VitaAudioEngine::get().set_pitch(s->channel,p/100.0f); }
    inline void EmitSound(BBSound* s, int /*entity*/) {
        if (!s || !s->data) return;
        float pitchMul = s->pitch / 100.0f;
        // 3D position would be set per-entity via set_listener; for now play as 2D
        s->channel = VitaAudioEngine::get().play(s->data, s->volume, pitchMul, false, s->is3d);
    }
    // Init/shutdown audio (called from main.cpp)
    inline void InitAudio() { VitaAudioEngine::get().init(); }
    inline void ShutdownAudio() { VitaAudioEngine::get().shutdown(); }

} // namespace BB

// Convenience macros so we don't have to prefix everything with BB::
using namespace BB;
