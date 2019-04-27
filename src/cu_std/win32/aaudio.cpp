
#include "aaudio.h"
#include "aallocator.h"
#include "Functiondiscoverykeys_devpkey.h"
#include "pparse.h"

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
            
            wcstombs((s8*)&entry->logical_name[0],card_id,sizeof(entry->logical_name));
            
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
        
        WCHAR buffer[512] = {};
        mbstowcs(&buffer[0],logical_name,sizeof(buffer));
        res = device_enum->GetDevice(&buffer[0],&card);
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
        
        AAudioSampleRate array[4] = {};
        u32 count = 0;
        
        AAudioSampleRate rate_array[] = {
            AAUDIOSAMPLERATE_44_1_KHZ,
            AAUDIOSAMPLERATE_48_KHZ,
            AAUDIOSAMPLERATE_88_2_KHZ,
            AAUDIOSAMPLERATE_96_KHZ
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
        
        AAudioChannels array[8] = {};
        u32 count = 0;
        
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
                array[count] = channels;
                count++;
            }
            
            if(closest){
                CoTaskMemFree(closest);
            }
        }
        
        _kill("no channels supported\n",!count);
        
        prop.min_channels = array[0];
        prop.max_channels = array[count - 1];
        
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
            
            wv.Format.nChannels = prop.min_channels;
            wv.Format.nSamplesPerSec = (u32)prop.min_rate;
            wv.Format.nBlockAlign = (wv.Format.nChannels * wv.Format.wBitsPerSample) >> 3;
            
            wv.Format.nAvgBytesPerSec = wv.Format.nSamplesPerSec * wv.Format.nBlockAlign;
            
            switch(prop.min_channels){
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
        
        res = client->Initialize(sharemode,0,
                                 0,
                                 period,//period size in - 100 nanoseconds. cannot be 0 in exclusive mode
                                 (WAVEFORMATEX*)&wv, 0);
        _kill("", res != S_OK);
        
        u32 min_buffer_size = 0;
        
        client->GetBufferSize(&min_buffer_size);
        
        prop.min_properties.internal_buffer_size = min_buffer_size * (u32)prop.min_channels * GetFormatSize(prop.format_array[0]);
        
        prop.min_properties.internal_period_size = ((f32)min_period/10000.0f) * (u32)((f32)prop.min_rate/1000.0f) *  (u32)prop.min_channels * GetFormatSize(prop.format_array[0]);
        
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
    
    _breakpoint();
    
    
    AAudioContext context = {};
    
    HRESULT res = 0;
    
    InitWasapi();
    
    auto sharemode = AUDCLNT_SHAREMODE_SHARED;
    
    if (logical_name) {
        
        if(IsReserved((s8*)logical_name)){
            sharemode = AUDCLNT_SHAREMODE_EXCLUSIVE;
        }
        
        WCHAR buffer[512] = {};
        mbtowc(&buffer[0],logical_name,sizeof(buffer));
        res = device_enum->GetDevice(&buffer[0],&context.device);
    }
    
    else {
        res = device_enum->GetDefaultAudioEndpoint(eRender,eMultimedia, &context.device);
    }
    
    //TODO: handle exclusive audio
    _breakpoint();
    
    _kill("", res != S_OK);
    
    res = context.device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void**)&context.audioclient);
    
    _kill("", res != S_OK);
    
    WAVEFORMATEX* wv_format = 0;
    context.audioclient->GetMixFormat(&wv_format);
    
    
    {
        _kill("do not support this format\n", (((WAVEFORMATEXTENSIBLE*)wv_format)->SubFormat != KSDATAFORMAT_SUBTYPE_PCM) && ((WAVEFORMATEXTENSIBLE*)wv_format)->SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
        
        if (format == AAUDIOFORMAT_S16 && ((WAVEFORMATEXTENSIBLE*)wv_format)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
            context.conversion_function = Convert_NONE_SLE16;
        }
        
        if (format == AAUDIOFORMAT_S16 && ((WAVEFORMATEXTENSIBLE*)wv_format)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
            context.conversion_function = Convert_SLE16_TO_F32;
        }
        
        _kill("for now\n", wv_format->nChannels != channels);
        
        context.channels = wv_format->nChannels;
    }
    
    _kill("", res != S_OK);
    
    //AUDCLNT_STREAMFLAGS_RATEADJUST  must be in shared mode only. lets you set the sample rate
    //AUDCLNT_SHAREMODE_EXCLUSIVE Windows only
    res = context.audioclient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_RATEADJUST,
                                          _buffersize,
                                          0,//period size in - 100 nanoseconds. cannot be 0 in exclusive mode
                                          wv_format, 0);
    
    _kill("", res != S_OK);
    
    if (wv_format->nSamplesPerSec != rate) {
        IAudioClockAdjustment* clockadj = 0;
        IID IID_IAudioClockAdjustment = __uuidof(IAudioClockAdjustment);
        
        res = context.audioclient->GetService(IID_IAudioClockAdjustment, (void**)&clockadj);
        _kill("", res != S_OK);
        
        res = clockadj->SetSampleRate(rate);
        _kill("", res != S_OK);
    }
    
    IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
    
    res = context.audioclient->GetService(IID_IAudioRenderClient, (void**)&context.renderclient);
    _kill("", res != S_OK);
    
    
    //maybe we should start only after we do our first submission
    res = context.audioclient->Start();
    _kill("", res != S_OK);
    
    CoTaskMemFree(wv_format);
    
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

void _intern ConvertAndWrite(AAudioContext* _restrict context, void* data, u32 frame_count, void* dst_buffer){
    
#define _reserved_frames (u32)_48ms2frames(36)
    
    auto sample_count = frame_count * context->channels;
    
    u32 conversion_buffer[sizeof(u32) * _reserved_frames] = {};
    _kill("exceeded conversion reserved conversion buffer\n", frame_count > _reserved_frames);
    
    auto samplesize = context->conversion_function(conversion_buffer,data,sample_count);
    
    memcpy(dst_buffer, conversion_buffer, samplesize * sample_count);
}

void APlayAudioDevice(AAudioContext* _restrict  context,void* data,u32 write_frames){
    
    s8* dst_buffer = 0;
    context->renderclient->GetBuffer(write_frames, (BYTE**)&dst_buffer);
    
    ConvertAndWrite(context,data,write_frames,dst_buffer);
    
    context->renderclient->ReleaseBuffer(write_frames, 0);
}
