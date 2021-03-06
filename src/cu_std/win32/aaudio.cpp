#include "mmath.h"
#include "aaudio.h"
#include "aallocator.h"
#include "Functiondiscoverykeys_devpkey.h"
#include "pparse.h"

#include "avrt.h"

_global bool audio_initialized;
_global IMMDeviceEnumerator* device_enum = 0;

#define COBJMACROS
#include "mmdeviceapi.h"
#include "audioclient.h"

_global u64 reserve_hash_array[32] = {};
_global u32 reserve_hash_count = 0;

#define _buffersize 880000

//signed 16 bit range -32,768 to 32,767

#include "audio_util.h"

#include "tthread.h"

void _intern InitWasapi(){
    
    HRESULT res = 0;
    
    if (!audio_initialized) {
        
        res = CoInitializeEx(0,COINIT_MULTITHREADED);
        _kill("", res != S_OK);
        
        CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
        IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
        
        res = CoCreateInstance(
            CLSID_MMDeviceEnumerator, 0,
            CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator,
            (void**)&device_enum);
        
        _kill("", res != S_OK);
        
        
        audio_initialized = true;
    }
    
}

void AGetAudioDevices(AAudioDeviceNames* _restrict  array,u32* _restrict  c){
    
    InitWasapi();
    
    u32 count = 0;
    
    IMMDeviceCollection* collection = 0;
    
    device_enum->EnumAudioEndpoints(eRender,DEVICE_STATE_ACTIVE,&collection);
    
    u32 total = 0;
    collection->GetCount(&total);
    
    //default device
    u64 default_card_hash = 0;
    {
        IMMDevice* default_card = 0;
        device_enum->GetDefaultAudioEndpoint(eRender,eMultimedia,&default_card);
        
        LPWSTR card_id = 0;
        default_card->GetId(&card_id);
        
        s8 buffer[512] = {};
        wcstombs(buffer,card_id,sizeof(buffer));
        
        default_card_hash = PHashString(buffer);
        
        default_card->Release();
        CoTaskMemFree(card_id);
    }
    
    for(u32 i = 0; i < total; i++){
        
        IMMDevice* card = 0;
        collection->Item(i,&card);
        
        //TODO: convert to ascii
        LPWSTR card_id = 0;
        IPropertyStore* property = 0;
        
        card->GetId(&card_id);//this is the logical name
        card->OpenPropertyStore(STGM_READ,&property);
        
        PROPVARIANT var1 = {};
        PROPVARIANT var2 = {};
        
        PropVariantInit(&var1);
        PropVariantInit(&var2);
        
        property->GetValue(PKEY_Device_FriendlyName,&var1);
        property->GetValue(PKEY_DeviceInterface_FriendlyName,&var2);
        
#ifdef DEBUG
        
        printf("%p ID: %S | %S | %S\n",(void*)card,card_id,var1.pwszVal,var2.pwszVal);
        
#endif
        if(array){
            
            auto entry = &array[count];
            
            auto size = wcslen(card_id) * sizeof(WCHAR);
            
            _kill("not enough space in before\n",size > sizeof(entry->
                                                               logical_name));
            memcpy((void*)&entry->
                   logical_name[0],(void*)&card[0],size);
            
            entry->is_default = default_card_hash == PHashString(&entry->logical_name[0]);
            
            s8 buffer1[512] = {};
            s8 buffer2[512] = {};
            
            wcstombs((s8*)&buffer1[0],var1.pwszVal,sizeof(buffer1));
            wcstombs((s8*)&buffer2[0],var2.pwszVal,sizeof(buffer2));
            
            sprintf((s8*)&entry->device_name[0],"%s:%s",&buffer1[0],&buffer2[0]);
            
        }
        count++;
        
        card->Release();
        CoTaskMemFree(card_id);
        property->Release();
    }
    
    
    
    if(c){
        *c = count;
    }
    
    collection->Release();
}

b32 AReserveAudioDevice(const s8* logical_name){
    
    auto hash = PHashString(logical_name);
    b32 found = false;
    
    for(u32 i = 0; i < reserve_hash_count; i++){
        
        if(hash == reserve_hash_array[i]){
            found = true;
            break;
        }
        
    }
    
    if(!found){
        reserve_hash_array[reserve_hash_count] = hash;
        reserve_hash_count++;
    }
    
    return 0;
}

AAudioPerformanceProperties AMakeAudioPerformanceProperties(u32 period_size,u32 period_count){
    u32 buffer_size = period_size * period_count;
    return {buffer_size,period_size};
}

AAudioPerformanceProperties AMakeDefaultAudioPerformanceProperties(){
    
    //assuming 2 channels s16 - contains 4ms of sound
    u32 period_size = _48ms2frames(4.0f) * 2 * sizeof(s16); 
    
    return AMakeAudioPerformanceProperties(period_size,22);
}

b32 _intern IsReserved(s8* logical_name){
    
    auto hash = PHashString(logical_name);
    
    for(u32 i = 0; i < reserve_hash_count; i++){
        
        if(hash == reserve_hash_array[i]){
            return true;
        }
        
    }
    
    return false;
}

u32 _intern GetFormatSize(AAudioFormat format){
    
    switch(format){
        case AAUDIOFORMAT_S16:{
            return sizeof(u16);
        }break;
        case AAUDIOFORMAT_S32:{
            return sizeof(u32);
        }break;
        case AAUDIOFORMAT_F32:{
            return sizeof(f32);
        }break;
        case AAUDIOFORMAT_F64:{
            return sizeof(f64);
        }break;
    }
    
    return 0;
}

AAudioDeviceProperties AGetAudioDeviceProperties(const s8* logical_name){
    
    InitWasapi();
    
#ifdef DEBUG
    {
        
        if(logical_name){
            b32 found = false;
            for(u32 i = 0;  i < reserve_hash_count; i++){
                if(reserve_hash_array[i] == PHashString(logical_name)){
                    found = true;
                    break;
                }
            }
            
            _kill("Device needs to be reserved first\n",!found);
        }
    }
#endif
    
    AAudioDeviceProperties prop = {};
    HRESULT res = 0;
    AUDCLNT_SHAREMODE sharemode = AUDCLNT_SHAREMODE_EXCLUSIVE;
    
    
    IMMDevice* card = 0;
    
    if (logical_name) {
        res = device_enum->GetDevice((WCHAR*)&logical_name[0],&card);
    }
    
    else {
        res = device_enum->GetDefaultAudioEndpoint(eRender,eMultimedia,&card);
        sharemode = AUDCLNT_SHAREMODE_SHARED;
    }
    
    _kill("", res != S_OK);
    
    IAudioClient* client = 0;
    
    res = card->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void**)&client);
    
    _kill("", res != S_OK);
    
    //just get the format that we can test against
    WAVEFORMATEXTENSIBLE* wv_format = 0;
    client->GetMixFormat((WAVEFORMATEX**)&wv_format);
    
#ifdef DEBUG
    {
        if(wv_format->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT){printf("FLOAT\n");}
        
        else if(wv_format->SubFormat == KSDATAFORMAT_SUBTYPE_PCM){printf("PCM\n");}
        
        else{
            _kill("TODO: handle unknown format",1);
        }
    }
#endif
    
    //find formats
    {
        auto wv = *wv_format;
        
        
        for(u32 i = 0; i < 4; i++){
            
            auto format = (AAudioFormat)i;
            
            {
                switch(format){
                    case AAUDIOFORMAT_S16: //fall thru
                    case AAUDIOFORMAT_S32:{
                        wv.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                    }break;
                    
                    
                    case AAUDIOFORMAT_F32: //fall thru
                    case AAUDIOFORMAT_F64:{
                        wv.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
                    }break;
                }
                
                wv.Format.wBitsPerSample = GetFormatSize(format) << 3;
                wv.Samples.wValidBitsPerSample = wv.Format.wBitsPerSample; // we don't support 24 bit audio so it should be ok
                
                wv.Format.nBlockAlign = (wv.Format.nChannels * wv.Format.wBitsPerSample) >> 3;
                
                wv.Format.nAvgBytesPerSec = wv.Format.nSamplesPerSec * wv.Format.nBlockAlign;
            }
            
            WAVEFORMATEX* closest = 0;
            
            auto ret = client->IsFormatSupported(sharemode,(WAVEFORMATEX*)&wv,&closest);
            
#ifdef DEBUG
            
            printf("%d || %d %d %d\n",ret,S_OK,S_FALSE,AUDCLNT_E_UNSUPPORTED_FORMAT);
            
#endif
            
            if(ret == S_OK){
                
#ifdef DEBUG
                printf("format %d is supported\n",i);
#endif
                prop.format_array[prop.format_count] = format;
                prop.format_count++;
                
            }
            
            if(closest){
                CoTaskMemFree(closest);
            }
            
        }
        
    }
    
    //find rates
    {
        auto format = prop.format_array[0];
        auto wv = *wv_format;
        {
            switch(format){
                case AAUDIOFORMAT_S16: //fall thru
                case AAUDIOFORMAT_S32:{
                    wv.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                }break;
                
                
                case AAUDIOFORMAT_F32: //fall thru
                case AAUDIOFORMAT_F64:{
                    wv.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
                }break;
            }
            
            wv.Format.wBitsPerSample = GetFormatSize(format) << 3;
            wv.Samples.wValidBitsPerSample = wv.Format.wBitsPerSample; // we don't support 24 bit audio so it should be ok
            wv.Format.nBlockAlign = (wv.Format.nChannels * wv.Format.wBitsPerSample) >> 3;
        }
        
        AAudioSampleRate array[5] = {};
        u32 count = 0;
        
        AAudioSampleRate rate_array[] = {
            AAUDIOSAMPLERATE_44_1_KHZ,
            AAUDIOSAMPLERATE_48_KHZ,
            AAUDIOSAMPLERATE_88_2_KHZ,
            AAUDIOSAMPLERATE_96_KHZ,
            AAUDIOSAMPLERATE_192_KHZ
        };
        
        for(u32 i = 0; i < _arraycount(rate_array); i++){
            auto rate = rate_array[i];
            
            wv.Format.nSamplesPerSec = (u32)rate;
            wv.Format.nAvgBytesPerSec = wv.Format.nSamplesPerSec * wv.Format.nBlockAlign;
            
            WAVEFORMATEX* closest = 0;
            
            auto ret = client->IsFormatSupported(sharemode,(WAVEFORMATEX*)&wv,&closest);
            
            if(ret == S_OK){
                array[count] = rate;
                count++;
            }
            
            if(closest){
                CoTaskMemFree(closest);
            }
        }
        
        _kill("no rate supported\n",!count);
        
        prop.min_rate = array[0];
        prop.max_rate = array[count - 1];
    }
    
    //find channels
    {
        
        AAudioChannels channels_array[] = {
            AAUDIOCHANNELS_MONO,
            AAUDIOCHANNELS_STEREO,
            AAUDIOCHANNELS_5_1,
            AAUDIOCHANNELS_7_1
        };
        
        auto wv = *wv_format;
        auto format = prop.format_array[0];
        {
            switch(format){
                case AAUDIOFORMAT_S16: //fall thru
                case AAUDIOFORMAT_S32:{
                    wv.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                }break;
                
                
                case AAUDIOFORMAT_F32: //fall thru
                case AAUDIOFORMAT_F64:{
                    wv.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
                }break;
            }
            
            wv.Format.wBitsPerSample = GetFormatSize(format) << 3;
            wv.Samples.wValidBitsPerSample = wv.Format.wBitsPerSample; // we don't support 24 bit audio so it should be ok
        }
        
        for(u32 i = 0; i < _arraycount(channels_array); i++){
            
            auto channels = channels_array[i];
            
            wv.Format.nChannels = (u32)channels;
            
            wv.Format.nBlockAlign = (wv.Format.nChannels * wv.Format.wBitsPerSample) >> 3;
            
            wv.Format.nAvgBytesPerSec = wv.Format.nSamplesPerSec * wv.Format.nBlockAlign;
            
            switch(channels){
                case AAUDIOCHANNELS_MONO:{
                    wv.dwChannelMask = KSAUDIO_SPEAKER_MONO;
                }break;
                
                case AAUDIOCHANNELS_STEREO:{
                    wv.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
                }break;
                
                
                //apparantly this and KSAUDIO_SPEAKER_5POINT1_SURROUND is different. we should test this when we get the chance
                case AAUDIOCHANNELS_5_1:{
                    wv.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
                }break;
                
                case AAUDIOCHANNELS_7_1:{
                    wv.dwChannelMask = KSAUDIO_SPEAKER_7POINT1;
                }break;
            }
            
            WAVEFORMATEX* closest = 0;
            
            auto ret = client->IsFormatSupported(sharemode,(WAVEFORMATEX*)&wv,&closest);
            if(ret == S_OK){
                prop.channels_array[prop.channels_count] = channels;
                prop.channels_count++;
            }
            
            if(closest){
                CoTaskMemFree(closest);
            }
        }
        
        _kill("no channels supported\n",!prop.channels_count);
        
    }
    
    //find buffer and period limits
    {
        REFERENCE_TIME default_period = {};
        REFERENCE_TIME min_period = {};
        
        res = client->GetDevicePeriod(&default_period,&min_period);
        _kill("", res != S_OK);
        
        auto wv = *wv_format;
        auto format = prop.format_array[0];
        {
            switch(format){
                case AAUDIOFORMAT_S16: //fall thru
                case AAUDIOFORMAT_S32:{
                    wv.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                }break;
                
                
                case AAUDIOFORMAT_F32: //fall thru
                case AAUDIOFORMAT_F64:{
                    wv.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
                }break;
            }
            
            wv.Format.wBitsPerSample = GetFormatSize(format) << 3;
            wv.Samples.wValidBitsPerSample = wv.Format.wBitsPerSample; // we don't support 24 bit audio so it should be ok
            
            wv.Format.nChannels = prop.channels_array[0];
            wv.Format.nSamplesPerSec = (u32)prop.min_rate;
            wv.Format.nBlockAlign = (wv.Format.nChannels * wv.Format.wBitsPerSample) >> 3;
            
            wv.Format.nAvgBytesPerSec = wv.Format.nSamplesPerSec * wv.Format.nBlockAlign;
            
            switch(prop.channels_array[0]){
                case AAUDIOCHANNELS_MONO:{
                    wv.dwChannelMask = KSAUDIO_SPEAKER_MONO;
                }break;
                
                case AAUDIOCHANNELS_STEREO:{
                    wv.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
                }break;
                
                
                //apparantly this and KSAUDIO_SPEAKER_5POINT1_SURROUND is different. we should test this when we get the chance
                case AAUDIOCHANNELS_5_1:{
                    wv.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
                }break;
                
                case AAUDIOCHANNELS_7_1:{
                    wv.dwChannelMask = KSAUDIO_SPEAKER_7POINT1;
                }break;
            }
        }
        
        u32 period = (sharemode == AUDCLNT_SHAREMODE_EXCLUSIVE) ? min_period : 0;
        
        
        
        res = client->Initialize(sharemode,0,0,period,//period size in - 100 nanoseconds. cannot be 0 in exclusive mode
                                 (WAVEFORMATEX*)&wv, 0);
        
        _kill("", res != S_OK);
        
        u32 min_buffer_size = 0;
        
        client->GetBufferSize(&min_buffer_size);
        
        prop.min_properties.internal_buffer_size = min_buffer_size * (u32)prop.channels_array[0] * GetFormatSize(prop.format_array[0]);
        
        
        prop.min_properties.internal_period_size = (u32)ceilf((((f32)min_period/10000.0f) * 
                                                               (f32)((((f32)prop.min_rate/1000.0f) * (f32)prop.channels_array[0] * GetFormatSize(prop.format_array[0])))));
        
        prop.max_properties.internal_buffer_size = 0xFFFFFFFF;
        prop.max_properties.internal_period_size = 0xFFFFFFFF;
    }
    
    /*
    
    AAudioFormat format_array[8];
    u32 format_count;
    
    AAudioSampleRate min_rate;
    AAudioSampleRate max_rate;
    
    AAudioChannels min_channels;
    AAudioChannels max_channels;
    
    AAudioPerformanceProperties min_properties;
    AAudioPerformanceProperties max_properties;
    
    HRESULT IAudioClient::IsFormatSupported(
  AUDCLNT_SHAREMODE  ShareMode,
  const WAVEFORMATEX *pFormat,
  WAVEFORMATEX       **ppClosestMatch
);
*/
    
    card->Release();
    client->Release();
    CoTaskMemFree(wv_format);
    
    return prop;
}

//WASAPI allows us to change the sample rate of a stream but not the stream format
AAudioContext ACreateDevice(const s8* logical_name,AAudioFormat format,AAudioChannels channels,AAudioSampleRate rate,AAudioPerformanceProperties prop){
    
    InitWasapi();
    
    
    AAudioContext context = {};
    
    HRESULT res = 0;
    
    auto sharemode = AUDCLNT_SHAREMODE_SHARED;
    u32 flags = 0;
    
    //TODO: make the divide part f32 to handle 44_1
    REFERENCE_TIME buffer_time = (prop.internal_buffer_size/(GetFormatSize(format) * (u32)channels * (u32)(rate/1000.0f))) * 10000;
    REFERENCE_TIME period_time = 0;
    
    if (logical_name) {
        
        if(IsReserved((s8*)logical_name)){
            sharemode = AUDCLNT_SHAREMODE_EXCLUSIVE;
        }
        
        res = device_enum->GetDevice((WCHAR*)&logical_name[0],&context.device);
        
        period_time = (prop.internal_period_size/(GetFormatSize(format) * (u32)channels * (u32)(rate/1000.0f))) * 10000;
    }
    
    else {
        res = device_enum->GetDefaultAudioEndpoint(eRender,eMultimedia, &context.device);
        //TODO: set flags
    }
    
    _kill("", res != S_OK);
    
    res = context.device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void**)&context.audioclient);
    
    _kill("", res != S_OK);
    
    WAVEFORMATEXTENSIBLE wv = {};
    {
        wv.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        wv.Format.wBitsPerSample = GetFormatSize(format) << 3;
        wv.Format.nChannels = (u32)channels;
        wv.Format.nSamplesPerSec = (u32)rate;
        wv.Format.nBlockAlign = (wv.Format.nChannels * wv.Format.wBitsPerSample) >> 3;
        wv.Format.nAvgBytesPerSec = wv.Format.nSamplesPerSec * wv.Format.nBlockAlign;
        
        wv.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
        
        
        wv.Samples.wValidBitsPerSample = wv.Format.wBitsPerSample;
        
        switch(channels){
            case AAUDIOCHANNELS_MONO:{
                wv.dwChannelMask = KSAUDIO_SPEAKER_MONO;
            }break;
            
            case AAUDIOCHANNELS_STEREO:{
                wv.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
            }break;
            
            
            //apparantly this and KSAUDIO_SPEAKER_5POINT1_SURROUND is different. we should test this when we get the chance
            case AAUDIOCHANNELS_5_1:{
                wv.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
            }break;
            
            case AAUDIOCHANNELS_7_1:{
                wv.dwChannelMask = KSAUDIO_SPEAKER_7POINT1;
            }break;
        }
        
        
        switch(format){
            case AAUDIOFORMAT_S16: //fall thru
            case AAUDIOFORMAT_S32:{
                wv.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            }break;
            
            
            case AAUDIOFORMAT_F32: //fall thru
            case AAUDIOFORMAT_F64:{
                wv.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
            }break;
        }
        
    }
    
    //AUDCLNT_STREAMFLAGS_RATEADJUST  must be in shared mode only. lets you set the sample rate
    //AUDCLNT_SHAREMODE_EXCLUSIVE Windows only
    res = context.audioclient->Initialize(sharemode,flags,
                                          buffer_time,
                                          period_time,//period size in - 100 nanoseconds. cannot be 0 in exclusive mode
                                          (WAVEFORMATEX*)&wv, 0);
    
    _kill("", res != S_OK);
    
#if 0
    
    if (wv.nSamplesPerSec != rate) {
        IAudioClockAdjustment* clockadj = 0;
        IID IID_IAudioClockAdjustment = __uuidof(IAudioClockAdjustment);
        
        res = context.audioclient->GetService(IID_IAudioClockAdjustment, (void**)&clockadj);
        _kill("", res != S_OK);
        
        res = clockadj->SetSampleRate(rate);
        _kill("", res != S_OK);
    }
    
#endif
    
    IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
    
    res = context.audioclient->GetService(IID_IAudioRenderClient, (void**)&context.renderclient);
    _kill("", res != S_OK);
    
    
    //maybe we should start only after we do our first submission
    res = context.audioclient->Start();
    _kill("", res != S_OK);
    
    //TODO: run to test exclusive audio and do flags and test shared. try to get the audio engine to do the resampling for us
    //_breakpoint();
    
    context.frame_size = GetFormatSize(format) * (u32)channels;
    
    return context;
}

u32 AAudioDeviceWriteAvailable(AAudioContext* _restrict context){
    
    u32 buffer_size_frames = 0;
    u32 frames_locked = 0;
    
    auto res = context->audioclient->GetBufferSize(&buffer_size_frames);
    _kill("", res != S_OK);
    
    
    res = context->audioclient->GetCurrentPadding(&frames_locked);
    _kill("", res != S_OK);
    
    
    return buffer_size_frames - frames_locked;
}

void APlayAudioDevice(AAudioContext* _restrict  context,void* data,u32 write_frames){
    
    s8* dst_buffer = 0;
    context->renderclient->GetBuffer(write_frames,(BYTE**)&dst_buffer);
    
    memcpy(dst_buffer,data,write_frames * context->frame_size);
    
    context->renderclient->ReleaseBuffer(write_frames, 0);
}

_global HANDLE thread_priority = 0;

#ifdef DEBUG
_global TThreadID init_thread = {};
#endif


void AInitThisAudioThread(){
    
    _kill("only one thread can have be prioritised at a time\n",thread_priority);
    
    DWORD index = 0;
    
    thread_priority = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &index);
    
#ifdef DEBUG
    init_thread = TGetThisThreadID();
#endif
}

void AUninitThisAudioThread(){
    
    _kill("the init thread has to call this\n",TGetThisThreadID() != init_thread);
    
    AvRevertMmThreadCharacteristics(thread_priority);
}
