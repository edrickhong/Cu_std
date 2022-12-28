#include "wwindow.h"

#include "libload.h"
#include "pparse.h"
#include "wayland_dyn.hpp"

// TODO: file dialog -- looks like we have to check through gtk2. we might be
// able to just step through and find out how it figures out the default file
// manager

union InternalConnection {
	Display* x11_display;
	wl_display* wayland_display;
};

struct InternalWindowData {
	u32 type;

	union {
		struct {
			u32 x11_depth;
			VisualID x11_visualid;
			Colormap x11_colormap;
		};

#ifndef NO_WAYLAND_EXTENSIONS
		struct {
			xdg_surface* wayland_xdg_surface;
		};
#endif
	};
};

struct InternalBackBufferData {
	union {
		struct {
			XImage ximage;
			Pixmap pixmap;
			GC gc;
		};

		struct {
			wl_buffer* buffer;
			u32 fd;
		};
	};
};

_global InternalConnection internal_windowconnection = {};

_global LibHandle wwindowlib_handle = 0;
_global WPlatform loaded_platform = WPLATFORM_NONE;
_global s8 wtext_buffer[256] = {};

// function implementations
_global s8 (*impl_wkeycodetoascii)(u32) = 0;
_global u32 (*impl_wwaitforevent)(WWindowEvent*) = 0;
_global void (*impl_wsettitle)(WWindowContext*, const s8*) = 0;
_global WBackBufferContext (*impl_wcreatebackbuffer)(WWindowContext*) = 0;
_global void (*impl_getwindowsize)(WWindowContext*, u32*, u32*) = 0;
_global void (*impl_wpresentbackbuffer)(WWindowContext*,
					WBackBufferContext*) = 0;
_global void (*impl_wdestroybackbuffer)(WBackBufferContext*) = 0;
_global void (*impl_wretireevent)(WWindowEvent*) = 0;
_global void (*impl_wackresizeevent)(WWindowEvent*) = 0;


#ifndef NO_WAYLAND_EXTENSIONS
#include "wayland_wwindow.cpp"
#endif

#include "x11_wwindow.cpp"

void WCreateWindowConnection(WPlatform platform) {
	_kill("An exising connection exsists\n", wwindowlib_handle);

	loaded_platform = platform;

	if (platform == WPLATFORM_WAYLAND) {
#ifndef NO_WAYLAND_EXTENSIONS
		InternalWaylandInitOneTime();
#endif
	}

	if (platform == WPLATFORM_X11) {
		InternalX11InitOneTime();
	}
}

void WDestroyWindowConnection() {

	if (loaded_platform == WPLATFORM_WAYLAND) {
	//TODO: need to handle all loose ends from the other libs
	//Not sure we got all of them
#ifndef NO_WAYLAND_EXTENSIONS
	InternalWaylandDeinitOneTime();
	InternalUnloadLibraryWayland();
#endif
	}

	if (loaded_platform == WPLATFORM_X11) {
	InternalX11DeinitOneTime();	
	InternalUnloadLibraryX11();
	}


	internal_windowconnection = {};
}

void* WGetWindowConnection() {
	return (void*)internal_windowconnection.x11_display;
}

// TODO: We can probably get rid of some of the branching

s8 WKeyCodeToASCII(u32 keycode) { return impl_wkeycodetoascii(keycode); }

u32 WWaitForWindowEvent(WWindowEvent* event) {
	return impl_wwaitforevent(event);
}

void WSetTitle(WWindowContext* context, const s8* title) {
	impl_wsettitle(context, title);
}

#include "pparse.h"
#include "vvulkan.h"

#ifdef __cplusplus
extern "C" {
#endif

WWindowContext WCreateWindow(const s8* title, 
		WCreateFlags flags, u32 x, u32 y, u32 width, u32 height) {
	_kill("A connection has not been created yet\n",
			!internal_windowconnection.x11_display);

	WWindowContext context = {};
	context.data = (InternalWindowData*)alloc(sizeof(InternalWindowData));

	memset(context.data, 0, sizeof(InternalWindowData));

	b32 res = 0;

	if(loaded_platform == WPLATFORM_WAYLAND){
#ifndef NO_WAYLAND_EXTENSIONS
		res = InternalCreateWaylandWindow(&context, title, flags, x, y,
						  width, height);
#endif
	}

	if(loaded_platform == WPLATFORM_X11){
		res = InternalCreateX11Window(&context, title, flags, x, y,
					      width, height);
	}

	_kill(
	    "Create window failed: either failed to load window lib,failed to "
	    "connect to window manager or failed to get a hw enabled window\n",
	    !res);

	return context;
}

void WGetPlatforms(WPlatform* array, u32* count, b32 vk_enabled) {
	auto test_lib = [](const s8* library, const s8* connect,
			   const s8* disconnect) -> b32 {
		auto lib = LLoadLibrary(library);

		if (!lib) {
			return false;
		}

		auto connect_fptr =
		    (void* (*)(void*))LGetLibFunction(lib, connect);
		auto disconnect_fptr =
		    (u32(*)(void*))LGetLibFunction(lib, disconnect);

		auto c = connect_fptr(0);

		if (c) {
			disconnect_fptr(c);
			LUnloadLibrary(lib);
			return true;
		}

		LUnloadLibrary(lib);
		return false;
	};

	auto test_vk = [](const s8* extension) -> b32 {
		VkExtensionProperties extension_array[32] = {};
		u32 count = 0;

		_kill(
		    "VInitVulkan must be called before calling this function\n",
		    vkEnumerateInstanceExtensionProperties == 0);

		vkEnumerateInstanceExtensionProperties(0, &count, 0);

		_kill("too many\n", count > _arraycount(extension_array));

		vkEnumerateInstanceExtensionProperties(0, &count,
						       &extension_array[0]);

		for (u32 i = 0; i < count; i++) {
			if (PHashString(extension_array[i].extensionName) ==
			    PHashString(extension)) {
				return true;
			}
		}

		return false;
	};

	WPlatform ar[2] = {};
	u32 c = 0;

	// test for wayland
	if (test_lib("libwayland-client.so", "wl_display_connect",
		     "wl_display_disconnect")) {
		ar[c] = WPLATFORM_WAYLAND;
		c++;

		if (vk_enabled && !test_vk("VK_KHR_wayland_surface")) {
			c--;
		}
	}

	// test for x11
	if (test_lib("libX11.so", "XOpenDisplay", "XCloseDisplay")) {
		ar[c] = WPLATFORM_X11;
		c++;

		if (vk_enabled && !test_vk("VK_KHR_xlib_surface")) {
			c--;
		}
	}

	if (count) {
		*count = c;
	}

	if (array) {
		memcpy(array, ar, c * sizeof(WPlatform));
	}
}

WBackBufferContext WCreateBackBuffer(WWindowContext* windowcontext) {
	return impl_wcreatebackbuffer(windowcontext);
}

void WDestroyBackBuffer(WBackBufferContext* buffer) {
	impl_wdestroybackbuffer(buffer);
}

void WPresentBackBuffer(WWindowContext* windowcontext,
			WBackBufferContext* buffer) {
	impl_wpresentbackbuffer(windowcontext, buffer);
}

void WAckResizeEvent(WWindowEvent* event) {
	event->ack_resize = 1;
	impl_wackresizeevent(event);
}

void WIgnoreResizeEvent(WWindowEvent* event) { event->ack_resize = 2; }

void WRetireEvent(WWindowEvent* event) {
#ifdef DEBUG
	_kill(
	    "Resize events must be explicitly handled WIgnoreResizeEvent "
	    "WAckResizeEvent\n",
	    !event->ack_resize && event->type == W_EVENT_RESIZE);
#endif

	impl_wretireevent(event);
}

#ifdef __cplusplus
}
#endif

void _ainline InternalGetWindowSize(WWindowContext* window, u32* w, u32* h) {
	impl_getwindowsize(window, w, h);
}
