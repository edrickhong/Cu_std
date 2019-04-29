#include "aaudio.h"
#include "libload.h"

#include "dbus/dbus.h"

#include "pparse.h"


struct AAudioInternalData{
    
};

#define _test(call) {auto call_res = call;if(call_res < 0){_kill("",1);}}


_global void* pcm_avail_update_fptr  = 0;
_global void* pcm_writei_fptr = 0;
_global void* pcm_recover_fptr = 0;
_global void* pcm_close_fptr = 0;

#define								\
snd_pcm_avail_update						\
((snd_pcm_sframes_t (*)(snd_pcm_t*))pcm_avail_update_fptr)

#define									\
snd_pcm_writei							\
((snd_pcm_sframes_t (*)(snd_pcm_t*,const void*,snd_pcm_uframes_t))pcm_writei_fptr)

#define							\
snd_pcm_recover					\
((s32 (*)(snd_pcm_t*,s32,s32))pcm_recover_fptr)

#define snd_pcm_close ((s32 (*)(snd_pcm_t*))pcm_close_fptr)

_global LibHandle audiolib = 0;
_global LibHandle dbuslib = 0;


//TODO: move this to its own file and clean up exclusvie implementation
#define _reserveaudio_priority 0x7FFFFFFF
#define _reserveaudio_debug 1 
#define _reserveaudio_timeout 5000

#if (_reserveaudio_priority == 0x7FFFFFFF)
#define _dbus_flag 0
#else
#define _dbus_flag DBUS_NAME_FLAG_ALLOW_REPLACEMENT
#endif

#define _dbus_reserve_name "CU_AUDIO_APP"
#define _dbus_reserve_devicename "CU_AUDIODEV"

_global void (*dbus_error_init_fptr)(DBusError*) = 0;
_global dbus_bool_t (*dbus_message_is_method_call_fpr)(DBusMessage*,const s8*,const s8*) = 0;
_global dbus_bool_t (*dbus_message_get_args_fptr)(DBusMessage*,DBusError*,s32...) = 0;
_global DBusMessage* (*dbus_message_new_method_return_fptr)(DBusMessage*) = 0;
_global dbus_bool_t (*dbus_message_append_args_fptr)(DBusMessage*,s32,...) = 0;
_global dbus_bool_t (*dbus_connection_send_fptr)(DBusConnection*,DBusMessage*,u32*) = 0;
_global void (*dbus_message_unref_fptr)(DBusMessage*) = 0;
_global void (*dbus_message_iter_init_append_fptr)(DBusMessage*,DBusMessageIter*) = 0;
_global dbus_bool_t (*dbus_message_iter_open_container_fptr)(DBusMessageIter*,s32,const s8*,DBusMessageIter*) = 0;
_global dbus_bool_t (*dbus_message_iter_append_basic_fptr)(DBusMessageIter*,s32,const void*) = 0;
_global dbus_bool_t (*dbus_message_iter_close_container_fptr)(DBusMessageIter*,DBusMessageIter*) = 0;
_global void (*dbus_error_free_fptr)(DBusError*) = 0;
_global dbus_bool_t (*dbus_message_is_signal_fptr)(DBusMessage*,const s8*,const s8*) = 0;


_global DBusDispatchStatus (*dbus_connection_dispatch_fptr)(DBusConnection*) = 0;
_global DBusDispatchStatus (*dbus_connection_get_dispatch_status_fptr)(DBusConnection*) = 0;

#define dbus_error_init dbus_error_init_fptr
#define dbus_message_is_method_call dbus_message_is_method_call_fpr
#define dbus_message_get_args dbus_message_get_args_fptr
#define dbus_error_free dbus_error_free_fptr
#define dbus_message_new_method_return dbus_message_new_method_return_fptr
#define dbus_message_append_args dbus_message_append_args_fptr
#define dbus_connection_send dbus_connection_send_fptr
#define dbus_message_unref dbus_message_unref_fptr
#define dbus_message_iter_init_append dbus_message_iter_init_append_fptr
#define dbus_message_iter_open_container dbus_message_iter_open_container_fptr
#define dbus_message_iter_append_basic dbus_message_iter_append_basic_fptr
#define dbus_message_iter_close_container dbus_message_iter_close_container_fptr
#define dbus_message_is_signal dbus_message_is_signal_fptr
#define dbus_connection_dispatch dbus_connection_dispatch_fptr
#define dbus_connection_get_dispatch_status dbus_connection_get_dispatch_status_fptr

_global DBusConnection* dbus_connection = 0;
_global const s8* dbus_interface_name = "org.freedesktop.ReserveDevice1";_global DBusObjectPathVTable dbus_vtable = {};
_global s32 dbus_priority = 0;

_global void (*dbus_handler)() = 0;


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
        
        pcm_close_fptr =
            LGetLibFunction(audiolib,"snd_pcm_close");
    }
}


_global u32 reserved_card_no_array[16] = {};
_global u32 reserved_card_no_count = 0;

u32 _intern GetAudioCardNo(const s8* logical_name){
    
    s8 num_buffer[16] = {};
    u32 num_count = 0;
    
    {
        u32 len = strlen(logical_name);
        
        b32 record = false;
        for(u32 i = 0; i < len; i++){
            
            auto c = logical_name[i];
            
            if(record){
                num_buffer[num_count] = c;
                num_count++;
            }
            
            if(c == ':'){
                record = true;
            }
            
            if(c == ','){
                break;
            }
        }
    }
    
    return (u32)atoi(num_buffer);
}

void _intern AReleaseAudioDevice(u32 device_no,b32 to_remove_entry = true){
    
#if _reserveaudio_debug
    DBusError error_s = {};
    DBusError* dbus_error = &error_s;
    dbus_error_init(dbus_error);//MARK: need to free if set (on error code)
#else
    DBusError* dbus_error = 0;
#endif
    
    auto dbus_bus_release_name_fptr = (s32 (*)(DBusConnection*,const s8*,DBusError*))
        LGetLibFunction(dbuslib,"dbus_bus_release_name");
    
    s8 buffer[512] = {};
    sprintf(buffer,"org.freedesktop.ReserveDevice1.Audio%d",device_no);
    
    auto ret = dbus_bus_release_name_fptr(dbus_connection,buffer,dbus_error);
    _kill("failed to release dbus\n",ret == -1);
    
    //remove card from release list
    if(to_remove_entry){
        for(u32 i = 0; i < reserved_card_no_count; i++){
            
            if(reserved_card_no_array[i] == device_no){
                reserved_card_no_count--;
                reserved_card_no_array[i] = reserved_card_no_array[reserved_card_no_count];
                break;
            }
        }
    }
    
}



void AReleaseAudioDevice(const s8* logical_name){
    
    _kill("cannot pass the default \n",!logical_name);
    
    u32 dev_no = GetAudioCardNo(logical_name);
    
    AReleaseAudioDevice(dev_no);
}

void AReleaseAudioDevice(AAudioContext* _restrict audiocontext){
    
    if(audiocontext->reserve_id != (u32)-1){
        AReleaseAudioDevice(audiocontext->reserve_id);
    }
}

_intern void FreeDeviceAccess(){
    
    for(u32 i = 0; i < reserved_card_no_count; i++){
        AReleaseAudioDevice(reserved_card_no_array[i],false);
    }
}

_intern DBusHandlerResult DBusFilterFunction(DBusConnection* connection,DBusMessage* message,void* data){
    
    
    //events loop
    
#if _reserveaudio_debug
    DBusError error_s = {};
    DBusError* dbus_error = &error_s;
    dbus_error_init(dbus_error);//MARK: need to free if set (on error code)
#else
    DBusError* dbus_error = 0;
#endif
    
    auto dbus_service_name = (s8*)data;
    
    //NOTE:this is to handle lost of device where the device is forcibly taken from us
    
    if(dbus_message_is_signal(message,"org.freedesktop.DBus", "NameLost")){
        
        const s8* name = 0;
        
        auto ret = dbus_message_get_args(message,dbus_error,DBUS_TYPE_STRING,&name,DBUS_TYPE_INVALID);
        
        if(!ret){
            dbus_error_free(dbus_error);
            _kill("failed to get args\n",!ret);
        }
        
        if(PHashString(name) == PHashString(dbus_service_name)){
            //TODO: add a way to tell the user that exclusive access is lost so they can handle it
            FreeDeviceAccess();
        }
        
    }
    
    return DBUS_HANDLER_RESULT_HANDLED;
}

_intern void DBusHandleEvents(){
    
    while(dbus_connection_get_dispatch_status(dbus_connection) != DBUS_DISPATCH_COMPLETE){
        dbus_connection_dispatch(dbus_connection);
    }
    
}

_intern DBusHandlerResult DBusObjHandler(DBusConnection* connection,DBusMessage* message,void* data){
    
    //others calls us
    
    //NOTE:this handles requests to give the device back and assigning object device info
    
#if _reserveaudio_debug
    DBusError error_s = {};
    DBusError* dbus_error = &error_s;
    dbus_error_init(dbus_error);//MARK: need to free if set (on error code)
#else
    DBusError* dbus_error = 0;
#endif
    
    //We have been asked to release our device
    if(dbus_message_is_method_call(message,dbus_interface_name,"RequestRelease")){
        
        printf("REQUEST RELEASE\n");
        
        s32 requester_priority;
        
        auto ret = dbus_message_get_args(message,dbus_error,DBUS_TYPE_INT32,&requester_priority,DBUS_TYPE_INVALID);
        
        if(!ret){
            dbus_error_free(dbus_error);
            _kill("can't get args\n",!ret);
        }
        
        
        
        dbus_bool_t to_return = false;
        
        if(requester_priority > dbus_priority){
            //TODO: add a way to tell the user that exclusive access is lost so they can handle it
            FreeDeviceAccess();
            to_return = true;
        }
        
        auto reply = dbus_message_new_method_return(message);
        _kill("cannot generate a reply\n",!reply);
        
        ret = dbus_message_append_args(reply,DBUS_TYPE_BOOLEAN,&to_return,DBUS_TYPE_INVALID);
        _kill("cannot append message\n",!ret);
        
        ret = dbus_connection_send(connection,reply,0);
        _kill("cannot send message\n",!ret);
        
        dbus_message_unref(reply);
    }
    
    
    else if(dbus_message_is_method_call(message,"org.freedesktop.DBus.Properties","Get")){
        
        const s8* interface = 0;
        const s8* property = 0;
        
        auto ret = dbus_message_get_args(message,dbus_error,DBUS_TYPE_STRING,&interface,DBUS_TYPE_STRING,&property,DBUS_TYPE_INVALID);
        
        if(!ret){
            dbus_error_free(dbus_error);
            _kill("can't get args\n",!ret);
        }
        
        if(PHashString(interface) == PHashString(dbus_interface_name)){
            
            
            auto add_prop = [](DBusMessage* msg,s32 type,void* data) -> void{
                
                DBusMessageIter iter = {};
                DBusMessageIter sub = {};
                
                type = type ^ 0xFFFFFF00;
                
                dbus_message_iter_init_append(msg,&iter);
                
                auto ret = dbus_message_iter_open_container(&iter,DBUS_TYPE_VARIANT,(const s8*)&type,&sub);
                _kill("failed to open container\n",!ret);
                
                ret = dbus_message_iter_append_basic(&sub,type,data);
                _kill("failed to append\n",!ret);
                
                ret = dbus_message_iter_close_container(&iter,&sub);
                _kill("failed to close\n",!ret);
            };
            
            switch(PHashString(property)){
                
                case PHashString("ApplicationName"):{
                    
                    printf("SET APP NAME\n");
                    
                    auto reply = dbus_message_new_method_return(message);
                    _kill("cannot generate a reply\n",!reply);
                    
                    add_prop(reply,DBUS_TYPE_STRING,(void*)_dbus_reserve_name);
                    
                    ret = dbus_connection_send(connection,reply,0);
                    _kill("failed to send reply\n",!ret);
                    
                    dbus_message_unref(reply);
                    
                }break;
                
                case PHashString("ApplicationDeviceName"):{
                    
                    printf("SET DEVICE NAME\n");
                    
                    auto reply = dbus_message_new_method_return(message);
                    _kill("cannot generate a reply\n",!reply);
                    
                    add_prop(reply,DBUS_TYPE_STRING,(void*)_dbus_reserve_devicename);
                    
                    ret = dbus_connection_send(connection,reply,0);
                    _kill("failed to send reply\n",!ret);
                    
                    dbus_message_unref(reply);
                    
                }break;
                
                case PHashString("Priority"):{
                    
                    printf("SET PRIORITY\n");
                    
                    auto reply = dbus_message_new_method_return(message);
                    _kill("cannot generate a reply\n",!reply);
                    
                    add_prop(reply,DBUS_TYPE_INT32,(void*)&dbus_priority);
                    
                    ret = dbus_connection_send(connection,reply,0);
                    _kill("failed to send reply\n",!ret);
                    
                    dbus_message_unref(reply);
                    
                }break;
                
#ifdef DEBUG
                default:{
                    _kill("weird message received\n",1);
                }break;
#endif
                
            }
            
        }
        
    }
    
    
    else if(dbus_message_is_method_call(message,"org.freedesktop.DBus.Introspectable","Introspect")){
        
        printf("INTROSPECT\n");
        
        _persist s8 introspect[] = {
            DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
                "<node>"
                " <!-- If you are looking for documentation make sure to check out\n"
                "      http://git.0pointer.de/?p=reserve.git;a=blob;f=reserve.txt -->\n"
                " <interface name=\"org.freedesktop.ReserveDevice1\">"
                "  <method name=\"RequestRelease\">"
                "   <arg name=\"priority\" type=\"i\" direction=\"in\"/>"
                "   <arg name=\"result\" type=\"b\" direction=\"out\"/>"
                "  </method>"
                "  <property name=\"Priority\" type=\"i\" access=\"read\"/>"
                "  <property name=\"ApplicationName\" type=\"s\" access=\"read\"/>"
                "  <property name=\"ApplicationDeviceName\" type=\"s\" access=\"read\"/>"
                " </interface>"
                " <interface name=\"org.freedesktop.DBus.Properties\">"
                "  <method name=\"Get\">"
                "   <arg name=\"interface\" direction=\"in\" type=\"s\"/>"
                "   <arg name=\"property\" direction=\"in\" type=\"s\"/>"
                "   <arg name=\"value\" direction=\"out\" type=\"v\"/>"
                "  </method>"
                " </interface>"
                " <interface name=\"org.freedesktop.DBus.Introspectable\">"
                "  <method name=\"Introspect\">"
                "   <arg name=\"data\" type=\"s\" direction=\"out\"/>"
                "  </method>"
                " </interface>"
                "</node>"
        };
        
        auto reply = dbus_message_new_method_return(message);
        _kill("cannot generate a reply\n",!reply);
        
        auto ret = dbus_message_append_args(reply,DBUS_TYPE_STRING,introspect,DBUS_TYPE_INVALID);
        _kill("failed to send reply\n",!ret);
        
        ret = dbus_connection_send(connection,reply,0);
        _kill("failed to send reply\n",!ret);
        
        dbus_message_unref(reply);
    }
    
    
    
    return DBUS_HANDLER_RESULT_HANDLED;
}


_intern _ainline b32 RegisterDBus(s8* dbus_service_name,s8* dbus_object_path){
    
    
    atexit(FreeDeviceAccess);
    
    auto dbus_bus_request_name_fptr = (s32 (*)(DBusConnection*,const s8*,u32,DBusError*))
        LGetLibFunction(dbuslib,"dbus_bus_request_name");
    
    auto dbus_connection_try_register_object_path_fptr = (dbus_bool_t (*)(DBusConnection*,const s8*,const DBusObjectPathVTable*,void*,DBusError*))LGetLibFunction(dbuslib,"dbus_connection_try_register_object_path");
    
    auto dbus_connection_add_filter_fptr = (dbus_bool_t (*)(DBusConnection*,DBusHandleMessageFunction,void*,DBusFreeFunction))LGetLibFunction(dbuslib,"dbus_connection_add_filter");
    
#if _reserveaudio_debug
    DBusError error_s = {};
    DBusError* dbus_error = &error_s;
    dbus_error_init(dbus_error);//MARK: need to free if set (on error code)
#else
    DBusError* dbus_error = 0;
#endif
    
    auto ret = dbus_bus_request_name_fptr(dbus_connection,dbus_service_name,_dbus_flag | DBUS_NAME_FLAG_DO_NOT_QUEUE | DBUS_NAME_FLAG_REPLACE_EXISTING,dbus_error);
    
    if(ret == -1){
        dbus_error_free(dbus_error);
        _kill("Failed to request name",ret == -1);
    }
    
    _kill("Failed to become the primary owner\n",ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER && ret != DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER);
    
    if(ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER && ret != DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER){
        return (u32)-1;
    }
    
    dbus_vtable.message_function = DBusObjHandler;
    
    ret = dbus_connection_try_register_object_path_fptr(dbus_connection,dbus_object_path,&dbus_vtable,dbus_service_name,dbus_error);
    
    if(!ret){
        dbus_error_free(dbus_error);
        _kill("failed to register handler\n",!ret);
    }
    
    ret = dbus_connection_add_filter_fptr(dbus_connection,DBusFilterFunction,dbus_service_name,0);
    
    _kill("failed to add filter\n",!ret);
    
    dbus_handler = DBusHandleEvents;
    
    return 0;
}

void _intern InternalLoadDBus(){
    
    if(dbuslib){
        return;
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
    
    {
        dbus_error_init = (void (*)(DBusError*))LGetLibFunction(dbuslib,"dbus_error_init");
        
        dbus_message_is_method_call = 
            (dbus_bool_t (*)(DBusMessage*,const s8*,const s8*))
            LGetLibFunction(dbuslib,"dbus_message_is_method_call");
        
        dbus_message_get_args = (dbus_bool_t (*)(DBusMessage*,DBusError*,s32...))
            LGetLibFunction(dbuslib,"dbus_message_get_args");
        
        
        dbus_message_append_args = (dbus_bool_t (*)(DBusMessage*,s32,...))LGetLibFunction(dbuslib,"dbus_message_append_args");
        
        dbus_message_new_method_return = (DBusMessage* (*)(DBusMessage*))LGetLibFunction(dbuslib,"dbus_message_new_method_return");
        
        dbus_connection_send = (dbus_bool_t (*)(DBusConnection*,DBusMessage*,u32*))LGetLibFunction(dbuslib,
                                                                                                   "dbus_connection_send"
                                                                                                   );
        
        dbus_message_unref = (void (*)(DBusMessage*))LGetLibFunction(dbuslib,"dbus_message_unref");
        
        dbus_message_iter_init_append = (void (*)(DBusMessage*,DBusMessageIter*))LGetLibFunction(dbuslib,"dbus_message_iter_init_append"
                                                                                                 );
        dbus_message_iter_open_container = 
            (dbus_bool_t (*)(DBusMessageIter*,s32,const s8*,DBusMessageIter*))LGetLibFunction(dbuslib,
                                                                                              "dbus_message_iter_open_container"
                                                                                              );
        
        dbus_message_iter_append_basic = (dbus_bool_t (*)(DBusMessageIter*,s32,const void*))LGetLibFunction(dbuslib,
                                                                                                            "dbus_message_iter_append_basic"
                                                                                                            );
        
        dbus_message_iter_close_container = (dbus_bool_t (*)(DBusMessageIter*,DBusMessageIter*))LGetLibFunction(dbuslib,
                                                                                                                "dbus_message_iter_close_container"
                                                                                                                );
        
        dbus_error_free = (void (*)(DBusError*))LGetLibFunction(dbuslib,"dbus_error_free");
        
        dbus_message_is_signal = (dbus_bool_t (*)(DBusMessage*,const s8*,const s8*))
            LGetLibFunction(dbuslib,"dbus_message_is_signal");
        
        
        dbus_connection_dispatch = (DBusDispatchStatus (*)(DBusConnection*))
            LGetLibFunction(dbuslib,"dbus_connection_dispatch");
        
        dbus_connection_get_dispatch_status = (DBusDispatchStatus (*)(DBusConnection*))
            LGetLibFunction(dbuslib,"dbus_connection_get_dispatch_status");
    }
}


b32 AReserveAudioDevice(const s8* logical_name){
    
    _kill("cannot pass the default \n",!logical_name);
    
    InternalLoadDBus();
    
    u32 dev_no = GetAudioCardNo(logical_name);
    
    
    //add this to the list of cards to be released
    {
        b32 found = false;
        for(u32 i = 0; i < reserved_card_no_count; i++){
            if(reserved_card_no_array[i] == dev_no){
                found = true;
                break;
            }
        }
        
        if(!found){
            reserved_card_no_array[reserved_card_no_count] = dev_no;
            reserved_card_no_count++;
        }
    }
    
    
    s8 dbus_service_name[512] = {};
    s8 dbus_object_path[512] = {};
    
    sprintf(dbus_service_name,"org.freedesktop.ReserveDevice1.Audio%d",dev_no);
    sprintf(dbus_object_path,"/org/freedesktop/ReserveDevice1/Audio%d",dev_no);
    
    auto dbus_bus_get_fptr = (DBusConnection* (*)(DBusBusType,DBusError*))LGetLibFunction(dbuslib,"dbus_bus_get");
    
    auto dbus_message_new_method_call_fptr = (DBusMessage* (*)(const s8*,const s8*,const s8*,const s8*))
        LGetLibFunction(dbuslib,"dbus_message_new_method_call");
    
    auto dbus_connection_send_with_reply_and_block_fptr = (DBusMessage* (*)(DBusConnection*,DBusMessage*,s32,DBusError*))LGetLibFunction(dbuslib,"dbus_connection_send_with_reply_and_block");
    
    
    auto dbus_bus_request_name_fptr = (s32 (*)(DBusConnection*,const s8*,u32,DBusError*))
        LGetLibFunction(dbuslib,"dbus_bus_request_name");
    
    
    
    
    
#if _reserveaudio_debug
    DBusError error_s = {};
    DBusError* dbus_error = &error_s;
    dbus_error_init_fptr(dbus_error);//MARK: need to free if set (on error code)
#else
    DBusError* dbus_error = 0;
#endif
    
    dbus_connection = dbus_bus_get_fptr(DBUS_BUS_SESSION,dbus_error);
    
    auto ret = dbus_bus_request_name_fptr(dbus_connection,dbus_service_name,_dbus_flag | DBUS_NAME_FLAG_DO_NOT_QUEUE,dbus_error);
    
    if(ret == -1){
        dbus_error_free(dbus_error);
        _kill("failed to request dbus\n",ret == -1);
    }
    
    _kill("not a valid return value\n",ret != DBUS_REQUEST_NAME_REPLY_EXISTS && ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER && ret != DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER);
    
    if(ret == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER || ret == DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER){
        
#ifdef DEBUG
        printf("Already own the audio device!!!\n");
#endif
        return RegisterDBus(dbus_service_name,dbus_object_path);
    }
    
    auto send_msg = dbus_message_new_method_call_fptr(dbus_service_name,dbus_object_path,dbus_interface_name,"RequestRelease");
    
    _kill("cannot create message\n",!send_msg);
    
    auto priority = _reserveaudio_priority;
    dbus_priority = priority;
    ret = dbus_message_append_args_fptr(send_msg,DBUS_TYPE_INT32,&priority,DBUS_TYPE_INVALID);
    
    _kill("failed to append msg\n",!ret);
    
    auto reply_msg = dbus_connection_send_with_reply_and_block_fptr(dbus_connection,send_msg,_reserveaudio_timeout,dbus_error);
    
    if(!reply_msg){
        dbus_error_free(dbus_error);
        _kill("no reply msg\n",!reply_msg);
    }
    
    
    b32 name_request = false;
    ret = dbus_message_get_args(reply_msg,dbus_error,DBUS_TYPE_BOOLEAN,&name_request,DBUS_TYPE_INVALID);
    
    if(!ret){
        dbus_error_free(dbus_error);
        _kill("failed to get args\n",!ret);
    }
    
    dbus_message_unref(send_msg);
    dbus_message_unref(reply_msg);
    
    return RegisterDBus(dbus_service_name,dbus_object_path);
    
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



void AGetAudioDevices(AAudioDeviceNames* _restrict  array,u32* _restrict  c){
    
    InternalLoadAudioLib();
    
    u32 count = 0;
    
    auto snd_ctl_open_fptr = (s32 (*)(snd_ctl_t**,const s8*,s32))LGetLibFunction(audiolib,"snd_ctl_open");
    
    auto snd_ctl_close_fptr = (s32 (*)(snd_ctl_t*))LGetLibFunction(audiolib,"snd_ctl_close");
    
    auto snd_ctl_card_info_fptr = (s32 (*)(snd_ctl_t*,snd_ctl_card_info_t*))LGetLibFunction(audiolib,"snd_ctl_card_info");
    
    
    
    //this doesn't work on default
    auto snd_ctl_card_info_get_card_fptr = (s32 (*)(const snd_ctl_card_info_t*))LGetLibFunction(audiolib,"snd_ctl_card_info_get_card");
    
    auto snd_ctl_card_info_get_id_fptr = (const s8* (*)(snd_ctl_card_info_t*))
        LGetLibFunction(audiolib,"snd_ctl_card_info_get_id");
    
    
    auto snd_ctl_card_info_get_name_fptr = (const s8* (*)(snd_ctl_card_info_t*))
        LGetLibFunction(audiolib,"snd_ctl_card_info_get_name");
    
    auto snd_ctl_card_info_get_longname_fptr = (const s8* (*)(snd_ctl_card_info_t*))
        LGetLibFunction(audiolib,"snd_ctl_card_info_get_longname");
    
    auto snd_ctl_card_info_get_components_fptr = (const s8* (*)(const snd_ctl_card_info_t*))
        LGetLibFunction(audiolib,"snd_ctl_card_info_get_components");
    
    auto snd_ctl_card_info_get_driver_fptr = (const s8* (*)(snd_ctl_card_info_t*))
        LGetLibFunction(audiolib,"snd_ctl_card_info_get_driver");
    
    auto snd_ctl_card_info_get_mixername_fptr = (const s8* (*)(snd_ctl_card_info_t*))
        LGetLibFunction(audiolib,"snd_ctl_card_info_get_mixername");
    
    
    auto snd_ctl_name_fptr = 
        (const s8* (*)(snd_ctl_t*))LGetLibFunction(audiolib,"snd_ctl_name");
    
    auto snd_ctl_card_info_malloc_fptr = (s32 (*)(snd_ctl_card_info_t**))LGetLibFunction(audiolib,"snd_ctl_card_info_malloc");
    
    auto snd_ctl_card_info_free_fptr = (void (*)(snd_ctl_card_info_t*))LGetLibFunction(audiolib,"snd_ctl_card_info_free");
    
    auto snd_card_next_fptr = (s32 (*)(s32*))LGetLibFunction(audiolib,"snd_card_next");
    
    auto snd_ctl_pcm_next_device_fptr = (s32 (*)(snd_ctl_t*,s32*))LGetLibFunction(audiolib,"snd_ctl_pcm_next_device");
    
    auto snd_ctl_pcm_info_fptr = (s32 (*)(snd_ctl_t*,snd_pcm_info_t*))LGetLibFunction(audiolib,"snd_ctl_pcm_info");
    
    auto snd_pcm_info_malloc_fptr = (s32 (*)(snd_pcm_info_t**))LGetLibFunction(audiolib,"snd_pcm_info_malloc");
    auto snd_pcm_info_free_fptr = (void (*)(snd_pcm_info_t*))LGetLibFunction(audiolib,"snd_pcm_info_free");
    
    auto snd_pcm_info_set_device_fptr = (void (*)(snd_pcm_info_t*,u32))LGetLibFunction(audiolib,"snd_pcm_info_set_device");
    
    auto snd_pcm_info_set_subdevice_fptr = (void (*)(snd_pcm_info_t*,u32))LGetLibFunction(audiolib,"snd_pcm_info_set_subdevice");
    
    auto snd_pcm_info_set_stream_fptr = (void (*)(snd_pcm_info_t*,snd_pcm_stream_t))
        LGetLibFunction(audiolib,"snd_pcm_info_set_stream");
    
    auto snd_pcm_info_get_name_fptr = (const s8* (*)(snd_pcm_info_t*))LGetLibFunction(audiolib,"snd_pcm_info_get_name");
    
    auto snd_pcm_info_get_subclass_fptr = (snd_pcm_subclass_t (*)(snd_pcm_info_t*))
        LGetLibFunction(audiolib,"snd_pcm_info_get_subclass");
    
    
    
    auto snd_pcm_open_fptr = (s32 (*)(snd_pcm_t**,const s8*,snd_pcm_stream_t,s32 mode))
        LGetLibFunction(audiolib,"snd_pcm_open");
    
    
    
    
    //
    
    snd_ctl_card_info_t* info = 0;
    snd_ctl_card_info_malloc_fptr(&info);
    
    snd_pcm_info_t* pcm_info = 0;
    snd_pcm_info_malloc_fptr(&pcm_info);
    
    s32 cur_card = -1;
    while(snd_card_next_fptr(&cur_card) == 0){
        
        if(cur_card == -1){
            break;
        }
        
        s8 pcm_name[512] = {};
        
        sprintf(pcm_name,"hw:%d",cur_card);
        
        snd_ctl_t* ctl = 0;
        
        auto ret = snd_ctl_open_fptr(&ctl,pcm_name,0);
        ret = snd_ctl_card_info_fptr(ctl,info);
        ret = snd_ctl_card_info_get_card_fptr(info);
        
        s32 cur_device = -1;
        while(snd_ctl_pcm_next_device_fptr(ctl,&cur_device) == 0){
            
            if(cur_device == -1){
                break;
            }
            
            sprintf(pcm_name,"hw:%d,%d",cur_card,cur_device);
            
            snd_pcm_info_set_device_fptr(pcm_info,cur_device);
            snd_pcm_info_set_subdevice_fptr(pcm_info,0);
            snd_pcm_info_set_stream_fptr(pcm_info,SND_PCM_STREAM_PLAYBACK);
            
            if(snd_ctl_pcm_info_fptr(ctl,pcm_info) < 0){
                continue;
            }
            /*
            NOTE:
            snd_ctl_card_info_get_id -- PCH / ODACrevB
            snd_ctl_card_info_get_name -- HDA Intel PCH / ODACrevB
            snd_ctl_card_info_get_driver_fptr -- HDA-Intel
            snd_pcm_info_get_id -- ALC892 Analog / USB Audio
            snd_pcm_info_get_name -- ALC892 Analog / USB Audio
*/
            
#if 0
            
            //card level
            printf("INFO::%d | %s | %s | %s | %s | %s | %s | %s\n",snd_ctl_card_info_get_card_fptr(info),snd_ctl_name_fptr(ctl),snd_ctl_card_info_get_id_fptr(info),snd_ctl_card_info_get_name_fptr(info),snd_ctl_card_info_get_longname_fptr(info),snd_ctl_card_info_get_components_fptr(info),snd_ctl_card_info_get_driver_fptr(info),snd_ctl_card_info_get_mixername_fptr(info));
            
            
            //device level
            
            printf("card %s(%d) device %s(%d) class ",snd_ctl_name_fptr(ctl),cur_card,snd_pcm_info_get_name_fptr(pcm_info),cur_device);
            
            switch(snd_pcm_info_get_subclass_fptr(pcm_info)){
                
                case SND_PCM_SUBCLASS_GENERIC_MIX:{printf("SND_PCM_SUBCLASS_GENERIC_MIX\n");}break;
                
                case SND_PCM_SUBCLASS_MULTI_MIX:{printf("SND_PCM_SUBCLASS_MULTI_MIX");}break;
                
                default:{printf("UNKNOWN\n");}break;
            }
            
#endif
            
            if(array){
                auto entry = &array[count];
                
                memcpy((s8*)&entry->logical_name[0],pcm_name,strlen(pcm_name));
                sprintf((s8*)&entry->device_name[0],"%s:%s",snd_ctl_card_info_get_name_fptr(info),snd_pcm_info_get_name_fptr(pcm_info));
                
                snd_pcm_t* pcm_handle = 0;
                entry->is_default = snd_pcm_open_fptr(&pcm_handle,pcm_name,SND_PCM_STREAM_PLAYBACK,
                                                      SND_PCM_NONBLOCK) < 0;
                
                if(entry->is_default){
                    _test(snd_pcm_open_fptr(&pcm_handle,"default",SND_PCM_STREAM_PLAYBACK,
                                            SND_PCM_NONBLOCK));
                }
                
                _test(snd_pcm_close(pcm_handle));
                
                printf("logical %s device %s\n",entry->logical_name,entry->device_name);
            }
            count++;
            
            
        }
        
        
        snd_ctl_close_fptr(ctl);
        
    }
    
    snd_pcm_info_free_fptr(pcm_info);
    snd_ctl_card_info_free_fptr(info);
    
    if(c){
        *c = count;
    }
    
}

AAudioDeviceProperties AGetAudioDeviceProperties(const s8* logical_name){
    
#ifdef DEBUG
    {
        if(logical_name){
            auto card = GetAudioCardNo(logical_name);
            b32 is_found = false;
            
            for(u32 i = 0; i < reserved_card_no_count; i++){
                if(card == reserved_card_no_array[i]){
                    is_found = true;
                    break;
                    
                }
                
            }
        }
        
        else{
            logical_name = "default";
        }
        
    }
#endif
    
    auto snd_pcm_open_fptr = (s32 (*)(snd_pcm_t**,const s8*,snd_pcm_stream_t,s32 mode))
        LGetLibFunction(audiolib,"snd_pcm_open");
    
    auto snd_pcm_hw_params_malloc_fptr  = (s32 (*)(snd_pcm_hw_params_t**))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_malloc");
    
    auto snd_pcm_hw_params_free_fptr = (void (*)(snd_pcm_hw_params_t *))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_free");
    
    
    auto snd_pcm_hw_params_any_fptr  = (s32 (*)(snd_pcm_t *,snd_pcm_hw_params_t *))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_any");
    
    auto snd_pcm_hw_params_test_format_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,snd_pcm_format_t))LGetLibFunction(audiolib,"snd_pcm_hw_params_test_format");
    
    auto snd_pcm_hw_params_get_rate_max_fptr = (s32 (*)(snd_pcm_hw_params_t*,u32*,s32*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_get_rate_max");
    
    auto snd_pcm_hw_params_get_rate_min_fptr = (s32 (*)(snd_pcm_hw_params_t*,u32*,s32*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_get_rate_min");
    
    auto snd_pcm_hw_params_get_channels_min_fptr  = (s32 (*)(snd_pcm_hw_params_t*,u32*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_get_channels_min");
    
    auto snd_pcm_hw_params_get_channels_max_fptr  = (s32 (*)(snd_pcm_hw_params_t*,u32*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_get_channels_max");
    
    
    //NOTE: these are in frames. we have to set the format and channels first and convert to bytes
    auto snd_pcm_hw_params_set_format_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,snd_pcm_format_t))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_set_format");
    
    auto snd_pcm_hw_params_set_channels_fptr =
        (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,u32))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_set_channels");
    
    auto snd_pcm_hw_params_get_period_size_max_fptr = (s32 (*)(snd_pcm_hw_params_t*,snd_pcm_uframes_t*,s32*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_get_period_size_max");
    
    auto snd_pcm_hw_params_get_period_size_min_fptr = (s32 (*)(snd_pcm_hw_params_t*,snd_pcm_uframes_t*,s32*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_get_period_size_min");
    
    
    auto snd_pcm_hw_params_get_buffer_size_max_fptr = (s32 (*)(snd_pcm_hw_params_t*,snd_pcm_uframes_t*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_get_buffer_size_max");
    
    auto snd_pcm_hw_params_get_buffer_size_min_fptr = (s32 (*)(snd_pcm_hw_params_t*,snd_pcm_uframes_t*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_get_buffer_size_min");
    
    
    AAudioDeviceProperties prop = {};
    
    snd_pcm_hw_params_t* hw_info = 0;
    snd_pcm_hw_params_malloc_fptr(&hw_info);
    
    
    snd_pcm_t* pcm_handle = 0;
    snd_pcm_open_fptr(&pcm_handle,logical_name,SND_PCM_STREAM_PLAYBACK,
                      SND_PCM_NONBLOCK);
    
    snd_pcm_hw_params_any_fptr(pcm_handle,hw_info);
    
    snd_pcm_format_t format_array[] = {
        SND_PCM_FORMAT_U16_LE,
        SND_PCM_FORMAT_U32_LE,
        SND_PCM_FORMAT_FLOAT_LE,
        SND_PCM_FORMAT_FLOAT64_LE,
    };
    
    for(u32 i = 0; i < _arraycount(format_array);i++){
        //test format 
        auto format = format_array[i];
        
        if(snd_pcm_hw_params_test_format_fptr(pcm_handle,hw_info,format)){
            
            if(format == SND_PCM_FORMAT_U16_LE){
                prop.format_array[prop.format_count] = AAUDIOFORMAT_S16;
                prop.format_count++;
            }
            else if(format == SND_PCM_FORMAT_U32_LE){
                prop.format_array[prop.format_count] = AAUDIOFORMAT_S32;
                prop.format_count++;
            }
            
            else{
                prop.format_array[prop.format_count] = (AAudioFormat)format;
                prop.format_count++;
            }
        }
    }
    
    auto cur_format = prop.format_array[0];
    
    s32 dir = 0;
    
    snd_pcm_hw_params_get_rate_max_fptr(hw_info,(u32*)&prop.min_rate,&dir);
    snd_pcm_hw_params_get_rate_min_fptr(hw_info,(u32*)&prop.min_rate,&dir);
    
    prop.min_rate = prop.min_rate < AAUDIOSAMPLERATE_44_1_KHZ ? AAUDIOSAMPLERATE_44_1_KHZ : prop.min_rate;
    
    prop.max_rate = !prop.max_rate ? AAUDIOSAMPLERATE_96_KHZ : prop.max_rate;
    
    /*
    TODO: replace with this 
int snd_pcm_hw_params_test_channels 	( 	snd_pcm_t *  	pcm,
  snd_pcm_hw_params_t *  	params,
  unsigned int  	val 
 ) 	
*/
    
    snd_pcm_hw_params_get_channels_min_fptr(hw_info,(u32*)&prop.min_channels);
    snd_pcm_hw_params_get_channels_max_fptr(hw_info,(u32*)&prop.max_channels);
    
    //NOTE: we are setting the format so we can convert to bytes
    //s16 should be available all the time
    _test(snd_pcm_hw_params_set_format_fptr(pcm_handle,hw_info,(snd_pcm_format_t)cur_format));
    
    _test(snd_pcm_hw_params_set_channels_fptr(pcm_handle,hw_info,prop.min_channels));
    
    _test(snd_pcm_hw_params_get_period_size_max_fptr(hw_info,(snd_pcm_uframes_t*)&prop.max_properties.internal_period_size,&dir));
    
    _test(snd_pcm_hw_params_get_period_size_min_fptr(hw_info,(snd_pcm_uframes_t*)&prop.min_properties.internal_period_size,&dir));
    
    _test(snd_pcm_hw_params_get_buffer_size_max_fptr(hw_info,(snd_pcm_uframes_t*)&prop.max_properties.internal_buffer_size));
    
    _test(snd_pcm_hw_params_get_buffer_size_min_fptr(hw_info,(snd_pcm_uframes_t*)&prop.min_properties.internal_buffer_size));
    
    
    
    //NOTE: convert from frames to bytes (frames * channels * sample_size)
    auto size = GetFormatSize(cur_format);
    
    prop.max_properties.internal_period_size *= size * prop.min_channels;
    prop.max_properties.internal_buffer_size *= size * prop.min_channels;
    prop.min_properties.internal_period_size *= size * prop.min_channels;
    prop.min_properties.internal_buffer_size *= size * prop.min_channels;
    
    prop.min_properties.internal_period_size = !prop.min_properties.internal_period_size ? prop.min_properties.internal_buffer_size : prop.min_properties.internal_period_size;
    
    prop.max_properties.internal_period_size = !prop.max_properties.internal_period_size ? prop.max_properties.internal_buffer_size : prop.max_properties.internal_period_size;
    
    
    snd_pcm_hw_params_free_fptr(hw_info);
    
#if DEBUG
    
    printf("rate min %d max %d\n",prop.min_rate,prop.max_rate);
    printf("channels min %d max %d\n",prop.min_channels,prop.max_channels);
    printf("period min %d max %d\n",prop.min_properties.internal_period_size,prop.max_properties.internal_period_size);
    
    printf("buffer min %d max %d\n",prop.min_properties.internal_buffer_size,prop.max_properties.internal_buffer_size);
    
#endif
    
    _test(snd_pcm_close(pcm_handle));
    
    return prop;
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

AAudioContext ACreateDevice(const s8* logical_name,AAudioFormat format,AAudioChannels channels,AAudioSampleRate rate,AAudioPerformanceProperties prop){
    
    if(logical_name == DEFAULT_AUDIO_DEVICE){
        logical_name = "default";
    }
    
    /*
      Now it only supports 48kHz. We should calculate the period length and period count according
      to the frequency passed to us
    */
    
    InternalLoadAudioLib();
    
    /*
    Stupid api design. no way to get the underlying hw devices for default because alsa is not even aware what a sound server is
    
    ctl gets hw info of the card
    we still need to open each device to get its properties (have fun boyo)
*/
    
    
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
    
    snd_pcm_t* handle = 0;
    
    snd_pcm_hw_params_t* hwparams = {};
    snd_pcm_sw_params_t* swparams = {};
    
    
    _test(snd_pcm_open_fptr(&handle,logical_name,SND_PCM_STREAM_PLAYBACK,
                            SND_PCM_NONBLOCK));
    
    snd_pcm_hw_params_malloc_fptr(&hwparams);
    
    //hw params
    {
        
        
        _test(snd_pcm_hw_params_any_fptr(handle, hwparams));
        
        _test(snd_pcm_hw_params_set_access_fptr(handle, hwparams,
                                                SND_PCM_ACCESS_RW_INTERLEAVED));
        
        snd_pcm_hw_params_set_format_fptr(handle,hwparams,(snd_pcm_format_t)format);
        
        //MARK: Figure this out programatically for other values
        context.sample_size = GetFormatSize(format);
        
        _test(snd_pcm_hw_params_set_channels_fptr(handle, hwparams,channels));
        
        context.channels = channels;
        
        
        _test(snd_pcm_hw_params_set_rate_near_fptr(handle, hwparams,(u32*)&rate,0));
    }
    
    snd_pcm_uframes_t period_frames = prop.internal_period_size/(GetFormatSize(format) * channels);
    
    //internal buffer configuration
    {
        
        _test(snd_pcm_hw_params_set_period_size_near_fptr(handle,hwparams,
                                                          (snd_pcm_uframes_t *)&period_frames,0));
        
        u32 periods = prop.internal_buffer_size/prop.internal_period_size;
        
        _test(snd_pcm_hw_params_set_periods_near_fptr(handle, hwparams, &periods, 0));
        
        
        snd_pcm_uframes_t bufferframes = period_frames * periods;
        
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


u32 AAudioDeviceWriteAvailable(AAudioContext* _restrict audiocontext){
    return snd_pcm_avail_update(audiocontext->handle);
}

void APlayAudioDevice(AAudioContext* _restrict audiocontext,void* _restrict data,u32 write_frames){
    
    s32 err = snd_pcm_writei(audiocontext->handle,data,write_frames);
    
    if(err == -EPIPE){
        
        printf("failed consistent write\n");
        
#ifdef DEBUG
        snd_pcm_recover(audiocontext->handle,err,0);//set to 1 to not print errors
#else
        snd_pcm_recover(audiocontext->handle,err,1);//set to 1 to not print errors
#endif
        
        
    }
    
    
    if(dbus_handler){
        dbus_handler();
    }
    
}

void ADestroyAudioDevice(AAudioContext* _restrict audiocontext){
    
    snd_pcm_close((snd_pcm_t*)audiocontext->handle);
    AReleaseAudioDevice(audiocontext);
}