#include "wwindow.h"
#include "libload.h"

#include "pparse.h"

#include "wayland_dyn.hpp"

//TODO: file dialog -- looks like we have to check through gtk2. we might be able to just step through and find out how it figures out the default file manager

struct WaylandData{

	u16 width;
	u16 height;

	//We don't touch these alot
	wl_compositor* compositor;
	wl_shell* shell;
	wl_seat* seat;
	wl_pointer* pointer;
	wl_keyboard* keyboard;

	//for sw rendering
	wl_shm* shm;
};

struct InternalWindowData{

	u32 type;

	union{

		struct{
			u32 x11_depth;
			VisualID x11_visualid;
			Colormap x11_colormap;
		};


		struct{
			void* wayland_shell_surface;
			void* internaldata;
		       	WaylandData wayland_data;
		};
	};

};

struct InternalBackBufferData{

	union{
		struct{
			XImage ximage;
			Pixmap pixmap;
			GC gc;
		};

		struct{
			wl_buffer* buffer;
		};
	};


};

_global LibHandle wwindowlib_handle = 0;
_global u32 loaded_lib_type = 0;
_global s8 wtext_buffer[256] ={};

//function implementations
_global s8 (*impl_wkeycodetoascii)(u32) = 0;
_global u32 (*impl_wwaitforevent)(WWindowContext*,WWindowEvent*) = 0;
_global void (*impl_wsettitle)(WWindowContext*,const s8*) = 0;
_global WBackBufferContext (*impl_wcreatebackbuffer)(WWindowContext*) = 0;
_global void (*impl_getwindowsize)(WWindowContext*,u32*,u32*) = 0;
_global void (*impl_wpresentbackbuffer)(WWindowContext*,WBackBufferContext*) = 0;

#include "x11_wwindow.cpp"
#include "wayland_wwindow.cpp"

//TODO: We can probably get rid of some of the branching



s8 WKeyCodeToASCII(u32 keycode){
	return impl_wkeycodetoascii(keycode);
}

u32 WWaitForWindowEvent(WWindowContext* windowcontext,
		WWindowEvent* event){

	return impl_wwaitforevent(windowcontext,event);
}

void WSetTitle(WWindowContext* context,const s8* title){
	impl_wsettitle(context,title);
}

#define W_CREATE_NO_CHECK (1 << 29)


#include "pparse.h"
#include "vvulkan.h"

#ifdef __cplusplus
extern "C" {
#endif


WWindowContext WCreateWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,
		u32 width,u32 height){

	WWindowContext context = {};
	context.data = (InternalWindowData*)alloc(sizeof(InternalWindowData));

	memset(context.data,0,sizeof(InternalWindowData));

	b32 res = 0;

	if(!(flags & W_CREATE_BACKEND_XLIB)){
		res = InternalCreateWaylandWindow(&context,title,flags,x,y,width,height);
	}

	if(!res && !(flags & W_CREATE_BACKEND_WAYLAND)){
		res = InternalCreateX11Window(&context,title,flags,x,y,width,height);  
	}


	_kill("Create window failed: either failed to load window lib,failed to connect to window manager or failed to get a hw enabled window\n",!res && !(W_CREATE_NO_CHECK & flags));

	return context;
}


WWindowContext WCreateVulkanWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,u32 width,
		u32 height){

	WWindowContext context = {};


	VkExtensionProperties extension_array[32] = {};
	u32 count = 0;

	_kill("VInitVulkan must be called before calling this function\n",vkEnumerateInstanceExtensionProperties == 0);

	vkEnumerateInstanceExtensionProperties(0,&count,0);

	_kill("too many\n",count > _arraycount(extension_array));

	vkEnumerateInstanceExtensionProperties(0,&count,&extension_array[0]);

	b32 wayland_enabled = false;

	for(u32 i = 0; i < count; i++){

		if(PHashString(extension_array[i].extensionName) == PHashString("VK_KHR_wayland_surface")){
			wayland_enabled = true;
			break;
		}
	}

	context = WCreateWindow(title,(WCreateFlags)(flags | W_CREATE_BACKEND_WAYLAND | W_CREATE_NO_CHECK),x,y,width,height);

	if(!context.handle){
		context = WCreateWindow(title,(WCreateFlags)(flags | W_CREATE_BACKEND_XLIB | W_CREATE_NO_CHECK),x,y,width,height);
	}

	_kill("Create window failed: either failed to load window lib,failed to connect to window manager or failed to get a hw enabled window\n",!context.handle);

	return context;
}

WBackBufferContext WCreateBackBuffer(WWindowContext* windowcontext){
	return impl_wcreatebackbuffer(windowcontext);
}

void WPresentBackBuffer(WWindowContext* windowcontext,WBackBufferContext* buffer){
	impl_wpresentbackbuffer(windowcontext,buffer);
}


#ifdef __cplusplus
}
#endif

void _ainline InternalGetWindowSize(WWindowContext* window,u32* w,u32* h){
	impl_getwindowsize(window,w,h);
}
