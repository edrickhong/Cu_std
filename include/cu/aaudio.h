#pragma once

/*
  NOTE: We will only support signed 16 for now
*/

#define _48ms2frames(ms) (((f32)(ms) * 48.0f) + 0.5f)
#define _48frames2ms(frames) (((f32)frames)/48.0f)

#define _441ms2frames(ms) (((f32)(ms) * 44.1f) + 0.5f)
#define _441frames2ms(frames) (((f32)frames)/44.1ff)

#ifdef _WIN32

#define COBJMACROS
#include "mmdeviceapi.h"
#include "audioclient.h"

#define A_FORMAT_S16LE WAVE_FORMAT_PCM
#define A_DEVICE_DEFAULT 0

#else

#include "alsa/asoundlib.h"

#define A_FORMAT_S16LE SND_PCM_FORMAT_S16_LE
#define A_DEVICE_DEFAULT "default"

#endif

#ifdef _WIN32

struct AAudioContext{
    IMMDevice* device;
    IAudioClient* audioclient;//MARK: used every frame
    IAudioRenderClient* renderclient;//MARK: used every frame
    u32 channels;
    u32 (*conversion_function)(void*, void*, u32);//MARK: used every frame
};

#else

struct AAudioContext{
    snd_pcm_t* handle; //MARK: used every frame
    u32 channels;
    u32 sample_size;
};

#endif

struct AAudioBuffer{
    void* data;
    u32 size_frames;
    u32 size;
    u32 cur_pos;
    u32 curpos_frames;
};

enum AAudioChannels{
    AAUDIOCHANNELS_MONO = 1,
    AAUDIOCHANNELS_STEREO = 2,
};

enum AAudioFormat{
    
#ifdef _WIN32
    AAUDIOFORMAT_S16 = WAVE_FORMAT_PCM,
#else
    AAUDIOFORMAT_S16 = SND_PCM_FORMAT_S16_LE,
#endif
};

enum AAudiMode{
    AAUDIOMODE_SHARED,
    AAUDIOMODE_EXCLUSIVE,
};

typedef void (AudioOperation(void* args));


AAudioContext ACreateAudioDevice(const s8* devicename,u32 frequency,u32 channels,u32 format);

void ADestroyAudioDevice(AAudioContext audiocontext);

u32 AAudioDeviceWriteAvailable(AAudioContext audiocontext);

void APlayAudioDevice(AAudioContext audiocontext,void* data,u32 write_frames);