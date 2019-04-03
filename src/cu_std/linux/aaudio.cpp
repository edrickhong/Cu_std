#include "aaudio.h"
#include "libload.h"

#include "dbus/dbus.h"

#define _test(call) if(call < 0){_kill("",1);}


_global void* pcm_avail_update_fptr  = 0;
_global void* pcm_writei_fptr = 0;
_global void* pcm_recover_fptr = 0;




#define								\
snd_pcm_avail_update						\
((snd_pcm_sframes_t (*)(snd_pcm_t*))pcm_avail_update_fptr)

#define									\
snd_pcm_writei							\
((snd_pcm_sframes_t (*)(snd_pcm_t*,const void*,snd_pcm_uframes_t))pcm_writei_fptr)

#define							\
snd_pcm_recover					\
((s32 (*)(snd_pcm_t*,s32,s32))pcm_recover_fptr)

_global LibHandle audiolib = 0;
_global LibHandle dbuslib = 0;


void _ainline InternalLoadAudioLib(){
    
    if(audiolib){
        return;
    }
    
    const s8* audio_libs[] = {
        "libasound.so.2",
        "libasound.so",
    };
    
    for(u32 i = 0; i < _arraycount(audio_libs); i++){
        audiolib = LLoadLibrary(audio_libs[i]);
        if(audiolib){
            break;
        }
    }
    
    _kill("can't load audio lib\n",!audiolib);
    
    {
        pcm_avail_update_fptr =
            LGetLibFunction(audiolib,"snd_pcm_avail_update");
        
        pcm_writei_fptr =
            LGetLibFunction(audiolib,"snd_pcm_writei");
        
        pcm_recover_fptr =
            LGetLibFunction(audiolib,"snd_pcm_recover");  
    }
    
    const s8* dbus_libs[] = {
        "libdbus-1.so.3",
        "libdbus-1.so",
    };
    
    for(u32 i = 0; i < _arraycount(dbus_libs); i++){
        dbuslib = LLoadLibrary(dbus_libs[i]);
        if(dbuslib){
            break;
        }
    }
    
    _kill("can't load dbus lib\n",!dbuslib);
}

_intern _ainline void RegisterDBus(){}

_intern void GetExclusvieAccess(){
    
#define _audiodev(str,dev) str #dev
#define _reserveaudio_dev(str) _audiodev(str,Audio0)
#define _reserveaudio_priority 0x7FFFFFFF
#define _reserveaudio_debug 1 
#define _reserveaudio_timeout 5000
    
#define _dbustest(call) {auto res = call;_kill("",res == -1);}
    
    auto dbus_error_init_fptr = (void (*)(DBusError*))LGetLibFunction(dbuslib,"dbus_error_init");
    
    auto dbus_bus_get_fptr = (DBusConnection* (*)(DBusBusType,DBusError*))LGetLibFunction(dbuslib,"dbus_bus_get");
    
    auto dbus_message_new_method_call_fptr = (DBusMessage* (*)(const s8*,const s8*,const s8*,const s8*))
        LGetLibFunction(dbuslib,"dbus_message_new_method_call");
    
    auto dbus_message_append_args_fptr = (dbus_bool_t (*)(DBusMessage*,s32,...))LGetLibFunction(dbuslib,"dbus_message_append_args");
    
    auto dbus_connection_send_with_reply_and_block_fptr = (DBusMessage* (*)(DBusConnection*,DBusMessage*,s32,DBusError*))LGetLibFunction(dbuslib,"dbus_connection_send_with_reply_and_block");
    
    auto dbus_message_get_args_fptr = (dbus_bool_t (*)(DBusMessage*,DBusError*,s32...))
        LGetLibFunction(dbuslib,"dbus_message_get_args");
    
    auto dbus_bus_request_name_fptr = (s32 (*)(DBusConnection*,const s8*,u32,DBusError*))
        LGetLibFunction(dbuslib,"dbus_bus_request_name");
    
    auto dbus_connection_try_register_object_path_fptr = (dbus_bool_t (*)(DBusConnection*,const s8*,const DBusObjectPathVTable*,void*,DBusError*))LGetLibFunction(dbuslib,"dbus_connection_try_register_object_path");
    
    auto dbus_connection_add_filter_fptr = (dbus_bool_t (*)(DBusConnection*,DBusHandleMessageFunction,void*,DBusFreeFunction))LGetLibFunction(dbuslib,"dbus_connection_add_filter");
    
    DBusConnection* connection = 0;
    
#if _reserveaudio_debug
    DBusError error_s = {};
    DBusError* error = &error_s;
#else
    DBusError* error = 0;
#endif
    
    dbus_error_init_fptr(error);//MARK: need to free if set (on error code)
    connection = dbus_bus_get_fptr(DBUS_BUS_SESSION,error);
    
    auto service_name = _reserveaudio_dev("org.freedesktop.ReserveDevice1.");
    auto object_path = _reserveaudio_dev("/org/freedesktop/ReserveDevice1/");
    auto interface_name = "org.freedesktop.ReserveDevice1";
    
#if (_reserveaudio_priority == 0x7FFFFFFF)
#define _dbus_flag 0
#else
#define _dbus_flag DBUS_NAME_FLAG_ALLOW_REPLACEMENT
#endif
    
    auto ret = dbus_bus_request_name_fptr(connection,service_name,_dbus_flag | DBUS_NAME_FLAG_DO_NOT_QUEUE,error);
    
    _dbustest(ret);
    
    _kill("not a valid return value\n",ret != DBUS_REQUEST_NAME_REPLY_EXISTS && ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER && ret != DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER);
    
    if(ret == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER || ret == DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER){
        //don't have to do shit?? how often do we hit this case??
        RegisterDBus();
        _kill("",1);
        return;
    }
    
    //MARK: need to free if this message
    auto send_msg = dbus_message_new_method_call_fptr(service_name,object_path,interface_name,"RequestRelease");
    
    _kill("cannot create message\n",!send_msg);
    
    auto priority = _reserveaudio_priority;
    ret = dbus_message_append_args_fptr(send_msg,DBUS_TYPE_INT32,&priority,DBUS_TYPE_INVALID);
    
    _kill("failed to append msg\n",!ret);
    
    //MARK: unref this??
    auto reply_msg = dbus_connection_send_with_reply_and_block_fptr(connection,send_msg,_reserveaudio_timeout,error);
    _kill("no reply msg\n",!reply_msg);
    
    b32 name_request = false;
    ret = dbus_message_get_args_fptr(reply_msg,error,DBUS_TYPE_BOOLEAN,&name_request,DBUS_TYPE_INVALID);
    
    _kill("failed to get msg\n",!ret);
    
    //TODO: register our app to dbus
    
    //FIXME: after registering w highest priority, no other app can reconnect to the device
    //It looks like we HAVE to release this right after we are done
    //if we segfault, the device becomes free again!!
    
    RegisterDBus();
    
    exit(0);
    _breakpoint();
}


AAudioContext ACreateAudioDevice(const s8* device_string,u32 frequency,u32 channels,
                                 u32 format){
    
    /*
      Now it only supports 48kHz. We should calculate the period length and period count according
      to the frequency passed to us
    */
    
    InternalLoadAudioLib();
    
    GetExclusvieAccess();
    
    auto snd_pcm_open_fptr = (s32 (*)(snd_pcm_t**,const s8*,snd_pcm_stream_t,s32 mode))
        LGetLibFunction(audiolib,"snd_pcm_open");
    
    auto snd_pcm_hw_params_malloc_fptr  = (s32 (*)(snd_pcm_hw_params_t**))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_malloc");
    
    auto snd_pcm_hw_params_any_fptr  = (s32 (*)(snd_pcm_t *,snd_pcm_hw_params_t *))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_any");
    
    
    auto snd_pcm_hw_params_set_access_fptr =
        (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,snd_pcm_access_t))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_set_access");
    
    
    auto snd_pcm_hw_params_set_format_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,snd_pcm_format_t))LGetLibFunction(audiolib,"snd_pcm_hw_params_set_format");
    
    
    auto snd_pcm_hw_params_set_channels_fptr =
        (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,u32))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_set_channels");
    
    auto snd_pcm_hw_params_set_rate_near_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,u32*,s32*))LGetLibFunction(audiolib,"snd_pcm_hw_params_set_rate_near");
    
    auto snd_pcm_hw_params_set_periods_near_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,u32*,s32*))LGetLibFunction(audiolib,"snd_pcm_hw_params_set_periods_near");
    
    auto snd_pcm_hw_params_set_buffer_size_near_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,snd_pcm_uframes_t*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_set_buffer_size_near");
    
    auto snd_pcm_hw_params_fptr  = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params");
    
    auto snd_pcm_hw_params_set_period_size_near_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,snd_pcm_uframes_t*,s32*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_set_period_size_near");
    
    
    auto snd_pcm_sw_params_malloc_fptr  = (s32 (*)(snd_pcm_sw_params_t**))
        LGetLibFunction(audiolib,"snd_pcm_sw_params_malloc");
    
    auto snd_pcm_sw_params_current_fptr =
        (s32 (*)(snd_pcm_t*,snd_pcm_sw_params_t*))
        LGetLibFunction(audiolib,"snd_pcm_sw_params_current");
    
    auto snd_pcm_sw_params_set_avail_min_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_sw_params_t*,snd_pcm_uframes_t))LGetLibFunction(audiolib,"snd_pcm_sw_params_set_avail_min");
    
    
    auto snd_pcm_sw_params_set_start_threshold_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_sw_params_t*,snd_pcm_uframes_t))LGetLibFunction(audiolib,"snd_pcm_sw_params_set_start_threshold");
    
    auto snd_pcm_sw_params_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_sw_params_t*))
        LGetLibFunction(audiolib,"snd_pcm_sw_params");
    
    auto snd_pcm_sw_params_free_fptr = (void (*)(snd_pcm_sw_params_t*))
        LGetLibFunction(audiolib,"snd_pcm_sw_params_free");
    
    auto snd_pcm_nonblock_fptr = (s32 (*)(snd_pcm_t*,s32))
        LGetLibFunction(audiolib,"snd_pcm_nonblock");
    
    auto snd_pcm_hw_params_free_fptr = (void (*)(snd_pcm_hw_params_t *))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_free");
    
#ifdef DEBUG
    
    auto snd_pcm_hw_params_get_buffer_size_fptr = (s32 (*)(const snd_pcm_hw_params_t*,snd_pcm_uframes_t*))LGetLibFunction(audiolib,"snd_pcm_hw_params_get_buffer_size");
    
    auto snd_pcm_hw_params_get_period_size_fptr = (s32 (*)(const snd_pcm_hw_params_t*,snd_pcm_uframes_t*,s32*))LGetLibFunction(audiolib,"snd_pcm_hw_params_get_period_size");
    
    auto snd_pcm_hw_params_get_periods_fptr = (s32 (*)(const snd_pcm_hw_params_t*,u32*,s32*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_get_periods");
    
#endif
    
    AAudioContext context = {};
    
    snd_pcm_t* handle;
    
    snd_pcm_hw_params_t* hwparams;
    snd_pcm_sw_params_t* swparams;
    
    _test(snd_pcm_open_fptr(&handle,device_string,SND_PCM_STREAM_PLAYBACK,
                            SND_PCM_NONBLOCK));
    
    snd_pcm_hw_params_malloc_fptr(&hwparams);
    
    //hw params
    {
        
        
        _test(snd_pcm_hw_params_any_fptr(handle, hwparams));
        
        _test(snd_pcm_hw_params_set_access_fptr(handle, hwparams,
                                                SND_PCM_ACCESS_RW_INTERLEAVED));
        
        snd_pcm_hw_params_set_format_fptr(handle,hwparams,
                                          (snd_pcm_format_t)format);
        
        //MARK: Figure this out programatically for other values
        context.sample_size = sizeof(s16);
        
        _test(snd_pcm_hw_params_set_channels_fptr(handle, hwparams,channels));
        
        context.channels = channels;
        
        
        _test(snd_pcm_hw_params_set_rate_near_fptr(handle, hwparams,&frequency, 0));
    }
    
    snd_pcm_uframes_t period_frames = _48ms2frames(4.0f);
    
    //internal buffer configuration
    {
        
#define _periodcount 22
        
        _test(snd_pcm_hw_params_set_period_size_near_fptr(handle,hwparams,
                                                          (snd_pcm_uframes_t *)&period_frames,0));
        
        u32 periods = _periodcount;
        
        _test(snd_pcm_hw_params_set_periods_near_fptr(handle, hwparams, &periods, 0));
        
        
        snd_pcm_uframes_t bufferframes = period_frames * _periodcount;
        
        _test(snd_pcm_hw_params_set_buffer_size_near_fptr(handle, hwparams, &bufferframes));
        
        /* "set" the hardware with the desired parameters */
        snd_pcm_hw_params_fptr(handle, hwparams);
        
        
        
#ifdef DEBUG
        
        snd_pcm_uframes_t bufsize;
        
        /* Get samples for the actual buffer size */
        snd_pcm_hw_params_get_buffer_size_fptr(hwparams, &bufsize);
        
        snd_pcm_uframes_t persize = 0;
        periods = 0;
        
        snd_pcm_hw_params_get_period_size_fptr(hwparams, &persize, NULL);
        snd_pcm_hw_params_get_periods_fptr(hwparams, &periods, NULL);
        
        fprintf(stderr,
                "ALSA: period size = %ld, periods = %u, buffer size = %lu\n",
                persize, periods, bufsize);//convert these to miliseconds
        
#endif
        
    }
    
    //sw params
    {
        snd_pcm_sw_params_malloc_fptr(&swparams);
        
        
        _test(snd_pcm_sw_params_current_fptr(handle, swparams));
        
        _test(snd_pcm_sw_params_set_avail_min_fptr(handle, swparams,period_frames));
        
        _test(snd_pcm_sw_params_set_start_threshold_fptr(handle,swparams,1));
        
        _test(snd_pcm_sw_params_fptr(handle, swparams));
        
        snd_pcm_sw_params_free_fptr(swparams);
    }
    
    snd_pcm_nonblock_fptr(handle,1);
    
    snd_pcm_hw_params_free_fptr(hwparams);
    
    context.handle = handle;
    
    return context;
}


u32 AAudioDeviceWriteAvailable(AAudioContext audiocontext){
    return snd_pcm_avail_update(audiocontext.handle);
}

void APlayAudioDevice(AAudioContext audiocontext,void* data,u32 write_frames){
    
    s32 err = snd_pcm_writei(audiocontext.handle,data,write_frames);
    
    if(err == -EPIPE){
        
        printf("failed consistent write\n");
        
#ifdef DEBUG
        snd_pcm_recover(audiocontext.handle,err,0);//set to 1 to not print errors
#else
        snd_pcm_recover(audiocontext.handle,err,1);//set to 1 to not print errors
#endif
        
        
    }
    
}


void APlayAudioDevice(AAudioContext audiocontext,void* data,u32 write_frames,
                      AudioOperation op,void* args){
    
    u32 avail_frames =  snd_pcm_avail_update(audiocontext.handle);
    
    if(avail_frames >= write_frames){
        
        if(op){
            op(args);
        }
        
        s32 err = snd_pcm_writei(audiocontext.handle,data,write_frames);
        
        if(err == -EPIPE){
            
            printf("failed consistent write\n");
            // exit(1);
            //underrun occured
            
#ifdef DEBUG
            snd_pcm_recover(audiocontext.handle,err,0);//set to 1 to not print errors
#else
            snd_pcm_recover(audiocontext.handle,err,1);//set to 1 to not print errors
#endif
            
            
        }
        
    }
    
}

