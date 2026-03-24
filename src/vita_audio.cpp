/*
 * vita_audio.cpp  –  VitaAudioEngine implementation
 * PSP2 headers are ONLY included here, not in the .h
 */
#include "../third_party/vita_audio.h"
#include "../include/debug_log.h"
#include <psp2/audioout.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/fcntl.h>
#include <cstdlib>
#include <cstring>

static constexpr int AUDIO_CHANNELS = 32;
static constexpr int AUDIO_GRAIN    = 256;
static constexpr int AUDIO_FREQ     = 44100;

struct VitaChannel {
    const VitaSound* sound   = nullptr;
    int    position  = 0;
    float  volume    = 1.f;
    float  pitch     = 1.f;
    bool   looping   = false;
    bool   active    = false;
    bool   is3d      = false;
    float  src_x=0, src_y=0, src_z=0;
};

struct VitaAudioEngine::Impl {
    int       port     = -1;
    SceUID    thread   = -1;
    int16_t*  mix_buf  = nullptr;
    std::atomic<bool> running{false};
    VitaChannel ch[AUDIO_CHANNELS]{};
    float cam_x=0, cam_y=0, cam_z=0;

    static int thread_func(SceSize, void* arg) {
        auto* self = *(Impl**)arg;
        while (self->running) {
            memset(self->mix_buf, 0, AUDIO_GRAIN * 4);
            self->mix(self->mix_buf, AUDIO_GRAIN);
            sceAudioOutOutput(self->port, self->mix_buf);
        }
        return 0;
    }

    void mix(int16_t* out, int grain) {
        for (int i = 0; i < AUDIO_CHANNELS; ++i) {
            auto& c = ch[i];
            if (!c.active || !c.sound || !c.sound->loaded) continue;
            float vol = c.volume;
            if (c.is3d) {
                float dx=c.src_x-cam_x, dy=c.src_y-cam_y, dz=c.src_z-cam_z;
                float dist=sqrtf(dx*dx+dy*dy+dz*dz);
                vol *= std::max(0.f, 1.f - dist/500.f);
            }
            const int16_t* src = c.sound->samples;
            int n = c.sound->n_samples;
            bool mono = (c.sound->channels == 1);
            for (int s = 0; s < grain; ++s) {
                if (c.position >= n) {
                    if (c.looping) c.position = 0;
                    else { c.active = false; break; }
                }
                int16_t L, R;
                if (mono) { L = R = (int16_t)(src[c.position++] * vol); }
                else { L=(int16_t)(src[c.position]*vol); R=(int16_t)(src[c.position+1]*vol); c.position+=2; }
                out[s*2]   = (int16_t)std::max(-32768,std::min(32767,(int)out[s*2]  +L));
                out[s*2+1] = (int16_t)std::max(-32768,std::min(32767,(int)out[s*2+1]+R));
            }
        }
    }
};

VitaAudioEngine::Impl* VitaAudioEngine::s_impl = nullptr;

VitaAudioEngine& VitaAudioEngine::get() {
    static VitaAudioEngine e;
    return e;
}

void VitaAudioEngine::init() {
    if (s_impl) return;
    s_impl = new Impl();
    s_impl->port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN,
        AUDIO_GRAIN, AUDIO_FREQ, SCE_AUDIO_OUT_MODE_STEREO);
    if (s_impl->port < 0) return;
    int vol[2] = {SCE_AUDIO_VOLUME_0DB, SCE_AUDIO_VOLUME_0DB};
    sceAudioOutSetVolume(s_impl->port,
        (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol);
    s_impl->mix_buf = new int16_t[AUDIO_GRAIN * 2];
    s_impl->running = true;
    s_impl->thread = sceKernelCreateThread("vita_audio",
        &Impl::thread_func, 0x10000100, 0x10000, 0, 0, nullptr);
    sceKernelStartThread(s_impl->thread, sizeof(void*), (void*)&s_impl);
}

void VitaAudioEngine::shutdown() {
    if (!s_impl) return;
    s_impl->running = false;
    if (s_impl->port >= 0) sceAudioOutReleasePort(s_impl->port);
    delete[] s_impl->mix_buf;
    delete s_impl;
    s_impl = nullptr;
}

VitaSound* VitaAudioEngine::load(const char* path) {
    if (g_debugLog) { fprintf(g_debugLog, "[AUD] load start: %s\n", path); fflush(g_debugLog); }
    auto* s = new VitaSound();
    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0) {
        if (g_debugLog) { fprintf(g_debugLog, "[AUD] open FAIL: %s\n", path); fflush(g_debugLog); }
        return s;
    }
    if (g_debugLog) { fprintf(g_debugLog, "[AUD] opened fd=%d: %s\n", fd, path); fflush(g_debugLog); }
    long sz = sceIoLseek(fd, 0, SCE_SEEK_END);
    if (sz <= 0 || sz > 10*1024*1024) {
        if (g_debugLog) { fprintf(g_debugLog, "[AUD] bad size %ld: %s\n", sz, path); fflush(g_debugLog); }
        sceIoClose(fd);
        return s;
    }
    if (g_debugLog) { fprintf(g_debugLog, "[AUD] size=%ld: %s\n", sz, path); fflush(g_debugLog); }
    sceIoLseek(fd, 0, SCE_SEEK_SET);
    uint8_t* raw = (uint8_t*)malloc(sz);
    if (!raw) {
        if (g_debugLog) { fprintf(g_debugLog, "[AUD] malloc(%ld) FAIL: %s\n", sz, path); fflush(g_debugLog); }
        sceIoClose(fd);
        return s;
    }
    if (g_debugLog) { fprintf(g_debugLog, "[AUD] malloc OK, reading %ld bytes: %s\n", sz, path); fflush(g_debugLog); }
    sceIoRead(fd, raw, sz);
    sceIoClose(fd);
    if (g_debugLog) { fprintf(g_debugLog, "[AUD] read+close done: %s\n", path); fflush(g_debugLog); }
    if (sz >= 44 && memcmp(raw,"RIFF",4)==0 && memcmp(raw+8,"WAVE",4)==0) {
        if (g_debugLog) { fprintf(g_debugLog, "[AUD] RIFF/WAVE header OK: %s\n", path); fflush(g_debugLog); }
        int off = 12;
        while (off + 8 < sz) {
            uint32_t clen = *(uint32_t*)(raw+off+4);
            if (clen > (uint32_t)(sz - off - 8)) clen = sz - off - 8;
            if (memcmp(raw+off,"fmt ",4)==0 && clen >= 16) {
                s->channels    = *(uint16_t*)(raw+off+10);
                s->sample_rate = *(uint32_t*)(raw+off+12);
                if (g_debugLog) { fprintf(g_debugLog, "[AUD] fmt: ch=%d rate=%d: %s\n", s->channels, s->sample_rate, path); fflush(g_debugLog); }
            } else if (memcmp(raw+off,"data",4)==0) {
                int n = clen / 2;
                if (g_debugLog) { fprintf(g_debugLog, "[AUD] data chunk: clen=%u n=%d: %s\n", clen, n, path); fflush(g_debugLog); }
                if (n > 0) {
                    s->samples   = new(std::nothrow) int16_t[n];
                    if (s->samples) {
                        s->n_samples = n;
                        memcpy(s->samples, raw+off+8, clen);
                        s->loaded = true;
                    } else {
                        if (g_debugLog) { fprintf(g_debugLog, "[AUD] sample alloc FAIL n=%d: %s\n", n, path); fflush(g_debugLog); }
                    }
                }
                break;
            }
            off += 8 + clen;
            if (off < 0) break;
        }
    } else {
        if (g_debugLog) { fprintf(g_debugLog, "[AUD] not RIFF/WAVE: %s\n", path); fflush(g_debugLog); }
    }
    free(raw);
    if (g_debugLog) { fprintf(g_debugLog, "[AUD] load done %s: sz=%ld loaded=%d\n", path, sz, s->loaded); fflush(g_debugLog); }
    return s;
}

void VitaAudioEngine::free_sound(VitaSound* s) {
    if (!s) return;
    delete[] s->samples;
    delete s;
}

int VitaAudioEngine::play(const VitaSound* s, float vol, float pitch,
                           bool loop, bool is3d, float x, float y, float z) {
    if (!s || !s->loaded || !s_impl) return -1;
    for (int i = 0; i < AUDIO_CHANNELS; ++i) {
        if (!s_impl->ch[i].active) {
            s_impl->ch[i] = {s, 0, vol, pitch, loop, true, is3d, x, y, z};
            return i;
        }
    }
    return -1;
}

void VitaAudioEngine::stop(int ch) {
    if (s_impl && ch>=0 && ch<AUDIO_CHANNELS) s_impl->ch[ch].active=false;
}
bool VitaAudioEngine::playing(int ch) const {
    return s_impl && ch>=0 && ch<AUDIO_CHANNELS && s_impl->ch[ch].active;
}
void VitaAudioEngine::set_volume(int ch, float v) {
    if (s_impl && ch>=0 && ch<AUDIO_CHANNELS) s_impl->ch[ch].volume=v;
}
void VitaAudioEngine::set_pitch(int ch, float p) {
    if (s_impl && ch>=0 && ch<AUDIO_CHANNELS) s_impl->ch[ch].pitch=p;
}
void VitaAudioEngine::set_listener(float x, float y, float z) {
    if (s_impl) { s_impl->cam_x=x; s_impl->cam_y=y; s_impl->cam_z=z; }
}
