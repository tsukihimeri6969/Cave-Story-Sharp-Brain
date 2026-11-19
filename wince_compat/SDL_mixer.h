#ifndef WINCE_SDL_MIXER_H
#define WINCE_SDL_MIXER_H

#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MIX_DEFAULT_FREQUENCY 22050
#define MIX_DEFAULT_FORMAT AUDIO_S16SYS
#define MIX_DEFAULT_CHANNELS 2
#define MIX_MAX_VOLUME SDL_MIX_MAXVOLUME

typedef struct Mix_Chunk {
    int allocated;
    Uint8 *abuf;
    Uint32 alen;
    Uint8 volume;
} Mix_Chunk;

typedef struct {
    Mix_Chunk *chunk;
    Uint32 pos;
    int loops;
    int volume;
} wince_mix_slot_t;

static void (*wince_music_hook)(void *, Uint8 *, int) = NULL;
static void *wince_music_udata = NULL;
static wince_mix_slot_t wince_channels[32];

static void wince_audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    memset(stream, 0, len);
    if (wince_music_hook) {
        wince_music_hook(wince_music_udata, stream, len);
    }
    for (int i = 0; i < 32; ++i) {
        Mix_Chunk *c = wince_channels[i].chunk;
        if (!c || !c->abuf) continue;
        Uint32 rem = c->alen - wince_channels[i].pos;
        Uint32 step = ((Uint32)len < rem) ? (Uint32)len : rem;
        int vol = (wince_channels[i].volume * c->volume) / MIX_MAX_VOLUME;
        SDL_MixAudio(stream, c->abuf + wince_channels[i].pos, step, vol);
        wince_channels[i].pos += step;
        if (wince_channels[i].pos >= c->alen) {
            if (wince_channels[i].loops != 0) {
                if (wince_channels[i].loops > 0) wince_channels[i].loops--;
                wince_channels[i].pos = 0;
            } else {
                wince_channels[i].chunk = NULL;
            }
        }
    }
}

static inline int Mix_OpenAudio(int freq, Uint16 format, int channels, int chunksize) {
    SDL_AudioSpec desired;
    memset(&desired, 0, sizeof(desired));
    desired.freq = freq;
    desired.format = format;
    desired.channels = (Uint8)channels;
    desired.samples = (Uint16)chunksize;
    desired.callback = wince_audio_callback;
    for (int i = 0; i < 32; ++i) {
        wince_channels[i].chunk = NULL;
        wince_channels[i].volume = MIX_MAX_VOLUME;
    }
    return SDL_OpenAudio(&desired, NULL);
}

static inline void Mix_CloseAudio(void) { SDL_CloseAudio(); }
static inline int Mix_AllocateChannels(int numchans) { return (numchans > 32) ? 32 : numchans; }

static inline void Mix_HookMusic(void (*mix_func)(void *udata, Uint8 *stream, int len), void *arg) {
    SDL_LockAudio();
    wince_music_hook = mix_func;
    wince_music_udata = arg;
    SDL_UnlockAudio();
}

static inline Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc) {
    SDL_AudioSpec spec;
    Uint8 *buf = NULL;
    Uint32 len = 0;
    if (!SDL_LoadWAV_RW(src, freesrc, &spec, &buf, &len)) return NULL;
    Mix_Chunk *chunk = (Mix_Chunk *)malloc(sizeof(Mix_Chunk));
    if (!chunk) { SDL_FreeWAV(buf); return NULL; }
    chunk->allocated = 1;
    chunk->abuf = buf;
    chunk->alen = len;
    chunk->volume = MIX_MAX_VOLUME;
    return chunk;
}

#define Mix_LoadWAV(file) Mix_LoadWAV_RW(SDL_RWFromFile((file), "rb"), 1)

static inline void Mix_FreeChunk(Mix_Chunk *chunk) {
    if (!chunk) return;
    if (chunk->allocated && chunk->abuf) SDL_FreeWAV(chunk->abuf);
    free(chunk);
}

static inline int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks) {
    (void)ticks;
    if (!chunk) return -1;
    SDL_LockAudio();
    if (channel < 0) {
        for (int i = 0; i < 32; ++i) {
            if (!wince_channels[i].chunk) { channel = i; break; }
        }
    }
    if (channel >= 0 && channel < 32) {
        wince_channels[channel].chunk = chunk;
        wince_channels[channel].pos = 0;
        wince_channels[channel].loops = loops;
    }
    SDL_UnlockAudio();
    return channel;
}

#define Mix_PlayChannel(ch, chunk, loops) Mix_PlayChannelTimed((ch), (chunk), (loops), -1)

static inline int Mix_HaltChannel(int channel) {
    SDL_LockAudio();
    if (channel < 0) {
        for (int i = 0; i < 32; ++i) wince_channels[i].chunk = NULL;
    } else if (channel < 32) {
        wince_channels[channel].chunk = NULL;
    }
    SDL_UnlockAudio();
    return 0;
}

static inline int Mix_Playing(int channel) {
    if (channel >= 0 && channel < 32) return wince_channels[channel].chunk != NULL;
    int count = 0;
    for (int i = 0; i < 32; ++i) if (wince_channels[i].chunk) count++;
    return count;
}

static inline int Mix_Volume(int channel, int volume) {
    if (channel >= 0 && channel < 32) {
        int prev = wince_channels[channel].volume;
        if (volume >= 0) wince_channels[channel].volume = volume;
        return prev;
    }
    return MIX_MAX_VOLUME;
}

static inline int Mix_VolumeChunk(Mix_Chunk *chunk, int volume) {
    if (!chunk) return 0;
    int prev = chunk->volume;
    if (volume >= 0) chunk->volume = (Uint8)volume;
    return prev;
}

static inline const char *Mix_GetError(void) { return SDL_GetError(); }

#ifdef __cplusplus
}
#endif

#endif
