#pragma once

/*
  NOTE: We will only support signed 16 for now
*/

#define _48ms2frames(ms) (((f32)(ms) * 48.0f) + 0.5f)
#define _48frames2ms(frames) (((f32)frames)/48.0f)

#define _441ms2frames(ms) (((f32)(ms) * 44.1f) + 0.5f)
#define _441frames2ms(frames) (((f32)frames)/44.1ff)

#define DEFAULT_AUDIO_DEVICE 0

#ifdef _WIN32

#define COBJMACROS
#include "mmdeviceapi.h"
#include "audioclient.h"

#else

#include "alsa/asoundlib.h"

#endif

struct AAudioHandle;
struct AAudioInternalData;

#ifdef _WIN32

struct AAudioContext{
    IMMDevice* device; // we don't really need to keep this
    IAudioClient* audioclient;//MARK: used every frame
    IAudioRenderClient* renderclient;//MARK: used every frame
    u32 channels;
    u32 (*conversion_function)(void*, void*, u32);//MARK: used every frame
};

#else

struct AAudioContext{
    snd_pcm_t* handle; //MARK: used every frame
    u32 reserve_id;//TODO: we can use this to track if we still have exclusive access
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

enum AAudioChannels : u32{
    AAUDIOCHANNELS_MONO = 1,
    AAUDIOCHANNELS_STEREO = 2,
    AAUDIOCHANNELS_5_1 = 6,
    AAUDIOCHANNELS_7_1 = 8,
};

enum AAudioSampleRate : u32{
    AAUDIOSAMPLERATE_44_1_KHZ = 44100,
    AAUDIOSAMPLERATE_48_KHZ = 48000,
    AAUDIOSAMPLERATE_88_2_KHZ = 88100,
    AAUDIOSAMPLERATE_96_KHZ = 96000,
};

enum AAudioFormat : u32{
    
#ifdef _WIN32
    AAUDIOFORMAT_S16 = 0,
    AAUDIOFORMAT_S32 = 1,
    
    AAUDIOFORMAT_F32 = 2,
    AAUDIOFORMAT_F64 = 3,
#else
    
    AAUDIOFORMAT_S16 = SND_PCM_FORMAT_S16_LE,
    AAUDIOFORMAT_S32 = SND_PCM_FORMAT_S32_LE,
    
    AAUDIOFORMAT_F32 = SND_PCM_FORMAT_FLOAT_LE,
    AAUDIOFORMAT_F64 = SND_PCM_FORMAT_FLOAT64_LE,
    
#endif
};

struct AAudioPerformanceProperties{
    // total size of the internal ring buffer
    u32 internal_buffer_size; 
    // minimum number of bytes that need to be written before it is flushed to the sound card
    u32 internal_period_size; 
};

struct AAudioDeviceNames{
    b32 is_default;
    const s8 logical_name[512] = {};
    const s8 device_name[512] = {};
};

struct AAudioDeviceProperties{
    AAudioFormat format_array[8];
    u32 format_count;
    
    AAudioSampleRate min_rate;
    AAudioSampleRate max_rate;
    
    //TODO: apparently this is best expressed by an array
    AAudioChannels min_channels;
    AAudioChannels max_channels;
    
    AAudioPerformanceProperties min_properties;
    AAudioPerformanceProperties max_properties;
};

AAudioPerformanceProperties AMakeDefaultAudioPerformanceProperties();
AAudioPerformanceProperties AMakeAudioPerformanceProperties(u32 period_size,u32 period_count);

AAudioContext ACreateDevice(const s8* logical_name,AAudioFormat format,AAudioChannels channels,AAudioSampleRate rate,AAudioPerformanceProperties prop = AMakeDefaultAudioPerformanceProperties());

void AGetAudioDevices(AAudioDeviceNames* _restrict  array,u32* _restrict  count);


//Reserves the device for exclusive use
b32 AReserveAudioDevice(const s8* logical_name);
//Releases the device for shared use
void AReleaseAudioDevice(const s8* logical_name);
void AReleaseAudioDevice(AAudioContext* _restrict audiocontext);

//valid usage: a default device has to be reserved first before you can get its properties
AAudioDeviceProperties AGetAudioDeviceProperties(const s8* logical_name);


//escalates thread priority so that more time is given to it for audio rendering tasks. for single dedicated audio thread only. recommended for very low latency rendering (sub 2 to 4 ms??). Implementation will do audio specific thread escalation if it is OS supported
void AInitThisAudioThread();//TODO:
void AUninitThisAudioThread();//TODO:

void ADestroyAudioDevice(AAudioContext* _restrict audiocontext);

u32 AAudioDeviceWriteAvailable(AAudioContext* _restrict audiocontext);

void APlayAudioDevice(AAudioContext* _restrict audiocontext,void* _restrict data,u32 write_frames);