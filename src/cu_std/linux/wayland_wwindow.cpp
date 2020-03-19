#pragma once

#include "fcntl.h"
#include "ssys.h"
#include "unistd.h"

/*
TODO:
fix the pointer to use the default pointer (requires dumb parsing)
fix quit message (requires xdg extensions)

xdg-shell-client-protocol.h

wl_shell is deprecated. we should be using xdg shell
we will need this for minmax and hide

*/

struct WaylandDecoratorHandler {
	wl_surface* surface;
	void (*handler)(void*, void*, void*);
	void* data;

	u32 mouse_x;
	u32 mouse_y;

	struct Element {
		u32 dim;
		u32 x;
		u32 y;

		enum ElementType {
			ELEMENT_HIDE,
			ELEMENT_MINMAX,
			ELEMENT_CLOSE,
		};

		ElementType type;
	};

	union {
		Element elements[3];
	};
};

#include "ssys.h"
#include "xkbcommon/xkbcommon.h"

_global LibHandle xkb_lib = 0;
_global xkb_context* xkb_ctx = 0;
_global xkb_keymap* xkb_kbmap = 0;
_global xkb_state* xkb_kbstate = 0;

_global WaylandDecoratorHandler* active_handler = 0;
_global WaylandDecoratorHandler dec_handler_array[16] = {};
_global u32 dec_handler_count = 0;

_global WWindowEvent wayland_event_array[32] = {};
_global u32 wayland_event_count = 0;

s32 (*xkb_state_key_get_utf8_fptr)(xkb_state*, xkb_keycode_t, s8*, size_t) = 0;

wl_proxy* (*wl_proxy_marshal_constructor_fptr)(wl_proxy*, u32,
					       const wl_interface*, ...) = 0;

s32 (*wl_proxy_add_listener_fptr)(wl_proxy*, void (**)(void), void*) = 0;

void (*wl_proxy_marshal_fptr)(wl_proxy*, u32, ...) = 0;

void (*wl_proxy_set_user_data_fptr)(wl_proxy*, void*) = 0;

void* (*wl_proxy_get_user_data_fptr)(wl_proxy*) = 0;

u32 (*wl_proxy_get_version_fptr)(wl_proxy*) = 0;

void (*wl_proxy_destroy_fptr)(wl_proxy*) = 0;

wl_proxy* (*wl_proxy_marshal_constructor_versioned_fptr)(wl_proxy*, u32,
							 const wl_interface*,
							 u32, ...) = 0;

s32 (*wl_display_prepare_read_fptr)(wl_display*) = 0;

s32 (*wl_display_dispatch_pending_fptr)(wl_display*) = 0;

s32 (*wl_display_flush_fptr)(wl_display*) = 0;

s32 (*wl_display_read_events_fptr)(wl_display*) = 0;

s32 (*wl_display_get_fd_fptr)(wl_display*) = 0;

s32 (*wl_display_dispatch_ftpr)(wl_display*) = 0;

const wl_interface* wl_display_interface_ptr = 0;
const wl_interface* wl_registry_interface_ptr = 0;
const wl_interface* wl_compositor_interface_ptr = 0;
const wl_interface* wl_subcompositor_interface_ptr = 0;
const wl_interface* wl_seat_interface_ptr = 0;
const wl_interface* wl_pointer_interface_ptr = 0;
const wl_interface* wl_keyboard_interface_ptr = 0;
const wl_interface* wl_surface_interface_ptr = 0;


const wl_interface* wl_callback_interface_ptr = 0;
const wl_interface* wl_region_interface_ptr = 0;

const wl_interface* wl_buffer_interface_ptr = 0;
const wl_interface* wl_shm_pool_interface_ptr = 0;
const wl_interface* wl_data_source_interface_ptr = 0;
const wl_interface* wl_data_device_interface_ptr = 0;
const wl_interface* wl_touch_interface_ptr = 0;
const wl_interface* wl_subsurface_interface_ptr = 0;
const wl_interface* wl_shm_interface_ptr = 0;
const wl_interface* wl_output_interface_ptr = 0;

b32 InternalLoadLibraryWayland() {
	if (wwindowlib_handle) {
		if (loaded_lib_type != _WAYLAND_WINDOW) {
			return false;
		}

		return true;
	}

	const s8* wayland_paths[] = {
	    "libwayland-client.so.0.3.0",
	    "libwayland-client.so.0",
	    "libwayland-client.so",
	};

	for (u32 i = 0; i < _arraycount(wayland_paths); i++) {
		wwindowlib_handle = LLoadLibrary(wayland_paths[i]);

		if (wwindowlib_handle) {
			break;
		}
	}

	const s8* xkb_paths[] = {"libxkbcommon.so.0.0.0", "libxkbcommon.so.0",
				 "libxkbcommon.so"};

	for (u32 i = 0; i < _arraycount(xkb_paths); i++) {
		xkb_lib = LLoadLibrary(xkb_paths[i]);

		if (xkb_lib) {
			break;
		}
	}

	if (!wwindowlib_handle || !xkb_lib) {
		return false;
	}

	loaded_lib_type = _WAYLAND_WINDOW;

	return true;
}

void InternalUnloadLibraryWayland() {
	LUnloadLibrary(wwindowlib_handle);
	wwindowlib_handle = 0;
	loaded_lib_type = 0;

	LUnloadLibrary(xkb_lib);
	xkb_lib = 0;
}

// wayland stuff

s8 WKeyCodeToASCIIWayland(u32 keycode) {
	s8 buffer[128] = {};

	xkb_state_key_get_utf8_fptr(xkb_kbstate, keycode, buffer,
				    sizeof(buffer));

	return buffer[0];
}

#include "debugtimer.h"

u32 WWaitForWindowEventWayland(WWindowContext* windowcontext,
			       WWindowEvent* event) {
	auto fd = wl_display_get_fd((wl_display*)windowcontext->handle);

	s32 res = 0;

	do {
		pollfd poll_info = {
		    fd,
		    POLLIN | POLLPRI,  // wait for input events
		};

		// returns 0 if no events read and timed out
		// returns positive if got events
		// returns negative if error
		res = poll(&poll_info, 1, 0);

	} while (res < 0 && errno == EINTR);  // EINTR - interrupt occured

	if (res) {
		wl_display_dispatch((wl_display*)windowcontext->handle);
	}

	else {
		wl_display_dispatch_pending((wl_display*)windowcontext->handle);
	}

	if (wayland_event_count) {
		wayland_event_count--;
		*event = wayland_event_array[wayland_event_count];

		return 1;
	}

	return 0;
}

void WSetTitleWayland(WWindowContext* context, const s8* title) {
	auto toplevel = context->data->wayland_xdg_toplevel;
	xdg_toplevel_set_title(toplevel,title);
}

void WaylandKeyboardMap(void* data, wl_keyboard* keyboard, u32 format, s32 fd,
			u32 size) {
	if (!xkb_kbmap) {
		auto string = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);

		auto xkb_keymap_new_from_string_fptr =
		    (xkb_keymap * (*)(xkb_context*, const s8*,
				      xkb_keymap_format,
				      xkb_keymap_compile_flags))
			LGetLibFunction(xkb_lib, "xkb_keymap_new_from_string");

		auto xkb_state_new_fptr = (xkb_state * (*)(xkb_keymap*))
		    LGetLibFunction(xkb_lib, "xkb_state_new");

		xkb_kbmap = xkb_keymap_new_from_string_fptr(
		    xkb_ctx, (const s8*)string, XKB_KEYMAP_FORMAT_TEXT_V1,
		    XKB_KEYMAP_COMPILE_NO_FLAGS);

		xkb_kbstate = xkb_state_new_fptr(xkb_kbmap);

		munmap(string, size);
	}
}

void WaylandKeyboardEnter(void* data, wl_keyboard* keyboard, u32 serial,
			  wl_surface* surface, wl_array* keys) {}

void WaylandKeyboardLeave(void* data, wl_keyboard* keyboard, u32 serial,
			  wl_surface* surface) {}

void WaylandKeyboardKey(void* data, wl_keyboard* keyboard, u32 serial, u32 time,
			u32 key, u32 state) {
	_kill("too many events\n",
	      wayland_event_count > _arraycount(wayland_event_array));

	auto event = &wayland_event_array[wayland_event_count];
	wayland_event_count++;

	if (state) {
		event->type = W_EVENT_KBEVENT_KEYDOWN;
	}

	else {
		event->type = W_EVENT_KBEVENT_KEYUP;
	}

	event->keyboard_event.keycode = key + 8;
}

void WaylandKeyboardModifiers(void* data, wl_keyboard* keyboard, u32 serial,
			      u32 mods_depressed, u32 mods_latched,
			      u32 mods_locked, u32 group) {}

void WaylandPointerEnter(void* data, wl_pointer* pointer, u32 serial,
			 wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy) {
	// NOTE: the serial needs to be stored to perform drag operations

	active_handler = 0;
	for (u32 i = 0; i < dec_handler_count; i++) {
		auto cur_handler = &dec_handler_array[i];
		if (cur_handler->surface == surface) {
			active_handler = cur_handler;
			break;
		}
	}
}

void WaylandPointerLeave(void* data, wl_pointer* pointer, u32 serial,
			 wl_surface* surface) {
	// NOTE: the serial needs to be stored to perform drag operations
}

void WaylandPointerMotion(void* data, wl_pointer* pointer, u32 time,
			  wl_fixed_t sx, wl_fixed_t sy) {
	_kill("too many events\n",
	      wayland_event_count > _arraycount(wayland_event_array));

	auto event = &wayland_event_array[wayland_event_count];
	wayland_event_count++;

	event->type = W_EVENT_MSEVENT_MOVE;

	event->mouse_event.x = wl_fixed_to_int(sx);
	event->mouse_event.y = wl_fixed_to_int(sy);

	if (active_handler) {
		active_handler->mouse_x = event->mouse_event.x;
		active_handler->mouse_y = event->mouse_event.y;
	}
}

void WaylandPointerButton(void* data, wl_pointer* pointer, u32 serial, u32 time,
			  u32 button, u32 state) {
	// NOTE: the serial needs to be stored to perform drag operations

	_kill("too many events\n",
	      wayland_event_count > _arraycount(wayland_event_array));

	auto event = &wayland_event_array[wayland_event_count];
	wayland_event_count++;

	if (state) {
		event->type = W_EVENT_MSEVENT_DOWN;
	}

	else {
		event->type = W_EVENT_MSEVENT_UP;
	}

	switch (button) {
		case 272: {
			event->mouse_event.keycode = MOUSEBUTTON_LEFT;
		} break;

		case 274: {
			event->mouse_event.keycode = MOUSEBUTTON_MIDDLE;
		} break;

		case 273: {
			event->mouse_event.keycode = MOUSEBUTTON_RIGHT;
		} break;
	}

	if (active_handler) {
		WWindowEvent out_event = {};
		active_handler->handler(active_handler->data, event,
					&out_event);

		if (out_event.type) {
			wayland_event_array[wayland_event_count] = out_event;
			wayland_event_count++;
		}
	}
}

void WaylandPointerAxis(void* data, wl_pointer* pointer, u32 time, u32 axis,
			wl_fixed_t value) {
	/*MARK: fill mouse scroll events here*/
}

void WaylandSHMFormat(void* data, wl_shm* shm, u32 format) {
	// NOTE: I think this prints the formats available
	// printf("shm %p has format %d\n",(void*)shm,format);
}



void Wayland_Ping(void* data, xdg_wm_base* wm_base, u32 serial) {
	xdg_wm_base_pong(wm_base, serial);
}

void Wayland_TopConfigure(void* data, xdg_toplevel* toplevel,s32 width, s32 height, wl_array* states) {
	//TODO: need to send an ack configure
	//See line 1111 of xdg-shell.h

}

void Wayland_Close(void* data,xdg_toplevel* toplevel){
	//TODO: see xdg-shell.h line 1128
}

void Wayland_SurfaceConfigure(void* data,xdg_surface* surface, u32 serial){
	auto context = (WWindowContext*)data;

	xdg_surface_ack_configure(surface,serial);
	wl_surface_commit((wl_surface*)context->window);
}


_global wl_pointer_listener pointer_listener = {
    WaylandPointerEnter, WaylandPointerLeave, WaylandPointerMotion,
    WaylandPointerButton, WaylandPointerAxis};

_global wl_keyboard_listener keyboard_listener = {
    WaylandKeyboardMap, WaylandKeyboardEnter, WaylandKeyboardLeave,
    WaylandKeyboardKey, WaylandKeyboardModifiers};



void SeatCapabilities(void* data, wl_seat* seat, u32 caps) {
	auto w = (WaylandData*)(&((WWindowContext*)data)->data->wayland_data);

	if (caps & WL_SEAT_CAPABILITY_POINTER) {
		w->pointer = wl_seat_get_pointer(seat);

		wl_pointer_add_listener(w->pointer, &pointer_listener, data);
	}

	if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
		w->keyboard = wl_seat_get_keyboard(seat);

		wl_keyboard_add_listener(w->keyboard, &keyboard_listener, data);
	}
}

// MARK: Listeners TODO: Do we really need these to be global??


_global const wl_seat_listener seat_listener = {SeatCapabilities};
_global wl_shm_listener shm_listener = {WaylandSHMFormat};



void Wayland_Display_Handle_Global(void* data, struct wl_registry* registry,
				   u32 id, const s8* interface, u32 version) {
	auto w = (WaylandData*)(&((WWindowContext*)data)->data->wayland_data);

	if (PHashString(interface) == PHashString("wl_compositor")) {
		w->compositor = (wl_compositor*)wl_registry_bind(
		    registry, id, &wl_compositor_interface, 1);
	}

	if (PHashString(interface) == PHashString("wl_subcompositor")) {
		w->subcompositor = (wl_subcompositor*)wl_registry_bind(
		    registry, id, &wl_subcompositor_interface, 1);
	}

	if (PHashString(interface) == PHashString("xdg_wm_base")) {
		w->base =
		    (xdg_wm_base*)wl_registry_bind(registry, id, 
				    &xdg_wm_base_interface,1);
	}

	if (PHashString(interface) == PHashString("wl_seat")) {
		w->seat = (wl_seat*)wl_registry_bind(registry, id,
						     &wl_seat_interface, 1);
		wl_seat_add_listener(w->seat, &seat_listener, data);
	}

	// NOTE: this is needed for sw rendering
	if (PHashString(interface) == PHashString("wl_shm")) {
		w->shm = (wl_shm*)wl_registry_bind(registry, id,
						   &wl_shm_interface, 1);
		wl_shm_add_listener(w->shm, &shm_listener, 0);
	}

#ifdef DEBUG
	printf("interface: %s\n", interface);
#endif
}


_global const wl_registry_listener registry_listener = {
    Wayland_Display_Handle_Global, 0};

_global const xdg_wm_base_listener wm_base_listener = {Wayland_Ping};

_global const xdg_surface_listener surface_listener = {
	Wayland_SurfaceConfigure
};

_global const xdg_toplevel_listener toplevel_listener = {
	Wayland_TopConfigure,
	Wayland_Close
};




void InternalLoadWaylandSymbols() {
	wl_proxy_marshal_constructor_fptr =
	    (wl_proxy * (*)(wl_proxy*, u32, const wl_interface*, ...))
		LGetLibFunction(wwindowlib_handle,
				"wl_proxy_marshal_constructor");

	wl_proxy_add_listener_fptr =
	    (s32(*)(wl_proxy*, void (**)(void), void*))LGetLibFunction(
		wwindowlib_handle, "wl_proxy_add_listener");

	wl_proxy_marshal_fptr = (void (*)(wl_proxy*, u32, ...))LGetLibFunction(
	    wwindowlib_handle, "wl_proxy_marshal");

	wl_proxy_set_user_data_fptr =
	    (void (*)(wl_proxy*, void*))LGetLibFunction(
		wwindowlib_handle, "wl_proxy_set_user_data");

	wl_proxy_get_user_data_fptr = (void* (*)(wl_proxy*))LGetLibFunction(
	    wwindowlib_handle, "wl_proxy_get_user_data");

	wl_proxy_get_version_fptr = (u32(*)(wl_proxy*))LGetLibFunction(
	    wwindowlib_handle, "wl_proxy_get_version");

	wl_proxy_destroy_fptr = (void (*)(wl_proxy*))LGetLibFunction(
	    wwindowlib_handle, "wl_proxy_destroy");

	wl_proxy_marshal_constructor_versioned_fptr =

	    (wl_proxy * (*)(wl_proxy*, u32, const wl_interface*, u32, ...))
		LGetLibFunction(wwindowlib_handle,
				"wl_proxy_marshal_constructor_versioned");

	wl_display_prepare_read_fptr = (s32(*)(wl_display*))LGetLibFunction(
	    wwindowlib_handle, "wl_display_prepare_read");

	wl_display_dispatch_pending_fptr = (s32(*)(wl_display*))LGetLibFunction(
	    wwindowlib_handle, "wl_display_dispatch_pending");

	wl_display_flush_fptr = (s32(*)(wl_display*))LGetLibFunction(
	    wwindowlib_handle, "wl_display_flush");

	wl_display_read_events_fptr = (s32(*)(wl_display*))LGetLibFunction(
	    wwindowlib_handle, "wl_display_read_events");

	wl_display_get_fd_fptr = (s32(*)(wl_display*))LGetLibFunction(
	    wwindowlib_handle, "wl_display_get_fd");

	wl_display_dispatch_ftpr = (s32(*)(wl_display*))LGetLibFunction(
	    wwindowlib_handle, "wl_display_dispatch");

	// wl_interface*
	wl_display_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_display_interface");

	wl_registry_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_registry_interface");

	wl_compositor_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_compositor_interface");

	wl_subcompositor_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_subcompositor_interface");

	wl_seat_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_seat_interface");

	wl_pointer_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_pointer_interface");

	wl_keyboard_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_keyboard_interface");

	wl_surface_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_surface_interface");

	wl_callback_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_callback_interface");

	wl_region_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_region_interface");

	wl_buffer_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_buffer_interface");

	wl_shm_pool_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_shm_pool_interface");

	wl_data_source_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_data_source_interface");

	wl_data_device_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_data_device_interface");

	wl_touch_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_touch_interface");

	wl_subsurface_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_subsurface_interface");

	wl_shm_interface_ptr = (wl_interface*)LGetLibFunction(
	    wwindowlib_handle, "wl_shm_interface");

	wl_output_interface_ptr = (wl_interface*)LGetLibFunction(
			wwindowlib_handle,"wl_output_interface");
}

void InternalLoadXkbSymbols() {
	xkb_state_key_get_utf8_fptr =
	    (s32(*)(xkb_state*, xkb_keycode_t, s8*, size_t))LGetLibFunction(
		xkb_lib, "xkb_state_key_get_utf8");
}

void GetWindowSizeWayland(WWindowContext* window, u32* w, u32* h) {
	*w = window->data->wayland_data.width;
	*h = window->data->wayland_data.height;
}

// MARK: this could actually be useful
FileHandle InternalCreateTempAnonymouseFile() {
	// create temp file descriptor (mkstemp) (unlink to hide the file)

	s8 string[32] = {"wayland-anon-file-XXXXXX"};

	auto fd = mkstemp(string);

	_kill("failed to make the file\n", fd == -1);

	auto flags = fcntl(fd, F_GETFD);

	_kill("failed to get flags\n", flags == -1);

	auto res = fcntl(fd, F_SETFD, flags | FD_CLOEXEC);

	_kill("failed to set flags\n", res == -1);

	unlink(string);

	return fd;
}

WBackBufferContext WCreateBackBufferWayland(WWindowContext* windowcontext) {
	auto width = windowcontext->data->wayland_data.width;
	auto height = windowcontext->data->wayland_data.height;
	auto shm = windowcontext->data->wayland_data.shm;
	auto size = width * height * 4;

	WBackBufferContext backbuffer = {};
	backbuffer.data =
	    (InternalBackBufferData*)malloc(sizeof(InternalBackBufferData));

	backbuffer.width = width;
	backbuffer.height = height;

	auto fd = InternalCreateTempAnonymouseFile();

	// MARK: we cannot mmap an empty file
	ftruncate(fd, size);

	// mmap the file
	backbuffer.pixels =
	    (u32*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	_kill("failed to map file\n", backbuffer.pixels == MAP_FAILED);

	auto pool = wl_shm_create_pool(shm, fd, size);

	// MARK: format is always xrgb
	backbuffer.data->buffer = wl_shm_pool_create_buffer(
	    pool, 0, width, height, width * 4, WL_SHM_FORMAT_ARGB8888);

	wl_shm_pool_destroy(pool);

	return backbuffer;
}

void WPresentBackBufferWayland(WWindowContext* windowcontext,
			       WBackBufferContext* buffer) {
	wl_surface_attach((wl_surface*)windowcontext->window,
			  buffer->data->buffer, 0, 0);
	wl_surface_commit((wl_surface*)windowcontext->window);
}

#define _decor_height 45
#define _element_dim 45
#define _element_thickness 3
#define _element_offset 15

void _ainline InternalDrawHide(u32* pixels, u32 width, u32 height) {
	for (u32 y = 0; y < height; y++) {
		for (u32 x = 0; x < _element_dim; x++) {
			u32* pixel = pixels + (y * width) + x;

			u32 startx = _element_offset;
			u32 endx = _element_dim - _element_offset;
			u32 endy = _decor_height - _element_offset;
			u32 starty = endy - _element_thickness;

			b32 condx = x > startx && x < endx;
			b32 condy = y > starty && y < endy;

			if (condx && condy) {
				*pixel = 0xFFFFFFFF;
			}
		}
	}
}

void _ainline InternalDrawMinMax(u32* pixels, u32 width, u32 height) {
	for (u32 y = 0; y < height; y++) {
		for (u32 x = 0; x < _element_dim; x++) {
			u32* pixel = pixels + (y * width) + x;

			u32 startx1 = _element_offset;
			u32 endx1 = _element_dim - _element_offset;
			u32 starty1 = _element_offset;
			u32 endy1 = _decor_height - _element_offset;

			u32 startx2 = _element_offset + _element_thickness;
			u32 endx2 = _element_dim -
				    (_element_thickness + _element_offset);
			u32 starty2 = _element_thickness + _element_offset;
			u32 endy2 = _decor_height -
				    (_element_thickness + _element_offset);

			b32 condition1 = (x > startx1 && x < endx1) &&
					 (y > starty1 && y < endy1);

			b32 condition2 = (x > startx2 && x < endx2) &&
					 (y > starty2 && y < endy2);

			if (condition1 && !condition2) {
				*pixel = 0xFFFFFFFF;
			}
		}
	}
}

void _ainline InternalDrawClose(u32* pixels, u32 width, u32 height) {
	u32 startx = _element_offset;
	u32 endx = _element_dim - _element_offset;
	u32 y0 = _element_offset;
	u32 y1 = _decor_height - _element_offset;

	for (u32 x = 0; x < width; x++) {
		if (x > startx && x < endx) {
			for (u32 i = 0; i < _element_thickness; i++) {
				u32* pixel0 = pixels + ((y0 + i) * width) + x;
				*pixel0 = 0xFFFFFFFF;

				u32* pixel1 = pixels + ((y1 - i) * width) + x;
				*pixel1 = 0xFFFFFFFF;
			}

			y0++;
			y1--;
		}
	}
}

void _ainline InternalDrawDecor(u32* pixels, u32 width, u32 height,
				WCreateFlags flags,
				WaylandDecoratorHandler* handler) {
	// clear color
	for (u32 i = 0; i < (width * height); i++) {
		u32* pixel = pixels + i;
		*pixel = _encode_argb(255, 166, 46, 10);
	}

	u32 count = 3;

	if (flags & W_CREATE_NORESIZE) {
		count--;
	}
	u32 startx = width - (count * _element_dim);

	memset(handler->elements, 0, _arraycount(handler->elements));
	u32 element_count = 0;

	InternalDrawHide(pixels + startx, width, height);
	handler->elements[element_count] = {
	    _element_dim, startx, 0,
	    WaylandDecoratorHandler::Element::ELEMENT_HIDE};
	element_count++;

	if (!(flags & W_CREATE_NORESIZE)) {
		startx += _element_dim;
		InternalDrawMinMax(pixels + startx, width, height);
		handler->elements[element_count] = {
		    _element_dim, startx, 0,
		    WaylandDecoratorHandler::Element::ELEMENT_MINMAX};
		element_count++;
	}

	startx += _element_dim;
	InternalDrawClose(pixels + startx, width, height);
	handler->elements[element_count] = {
	    _element_dim, startx, 0,
	    WaylandDecoratorHandler::Element::ELEMENT_CLOSE};
	element_count++;
}

void InternalHandleDecorator(void* data, void* args, void* out) {
	auto decor = (WaylandDecorator*)data;

	// perform drag

	b32 is_interact = false;
	for (u32 i = 0; i < _arraycount(active_handler->elements); i++) {
		auto element = &active_handler->elements[i];

		if (!element->dim) {
			break;
		}

		// TODO: replace this with a generic rect intersection
		auto rect_intersection = [](u32 r_x, u32 r_y, u32 dim, u32 m_x,
					    u32 m_y) -> b32 {
			u32 x_end = r_x + dim;
			u32 y_end = r_y + dim;
			return m_x > r_x && m_x < x_end && m_y > r_y &&
			       m_y < y_end;
		};
		if (rect_intersection(element->x, element->y, _element_dim,
				      active_handler->mouse_x,
				      active_handler->mouse_y)) {
			is_interact = true;

			switch (element->type) {
				case WaylandDecoratorHandler::Element::
				    ElementType::ELEMENT_HIDE: {
					// TODO:perform hide
				} break;
				case WaylandDecoratorHandler::Element::
				    ElementType::ELEMENT_MINMAX: {
					// TODO: perform minmax event
				} break;
				case WaylandDecoratorHandler::Element::
				    ElementType::ELEMENT_CLOSE: {
					auto event = (WWindowEvent*)out;
					event->type = W_EVENT_CLOSE;
				} break;
			}
		}
	}

	if (!is_interact) {
		// TODO: perform drag
	}
}

void InternalCreateWindowDecorator(WWindowContext* context,
				   WCreateFlags flags) {
	auto wdata = &context->data->wayland_data;
	WaylandDecorator* decor = &wdata->decor;
	decor->surface = wl_compositor_create_surface(wdata->compositor);

	decor->subsurface = wl_subcompositor_get_subsurface(
	    wdata->subcompositor, decor->surface, (wl_surface*)context->window);

	// TODO: this should be scaled by screen height

	u32 width = context->data->wayland_data.width;
	u32 height = _decor_height;
	{
		auto shm = context->data->wayland_data.shm;
		auto size = width * height * 4;

		auto fd = InternalCreateTempAnonymouseFile();

		// MARK: we cannot mmap an empty file
		ftruncate(fd, size);

		// mmap the file
		decor->pixels = (u32*)mmap(0, size, PROT_READ | PROT_WRITE,
					   MAP_SHARED, fd, 0);

		_kill("failed to map file\n", decor->pixels == MAP_FAILED);

		auto pool = wl_shm_create_pool(shm, fd, size);

		// MARK: format is always xrgb
		decor->buffer = wl_shm_pool_create_buffer(
		    pool, 0, width, height, width * 4, WL_SHM_FORMAT_ARGB8888);

		wl_shm_pool_destroy(pool);
	}

	auto handler = &dec_handler_array[dec_handler_count];
	dec_handler_count++;

	handler->surface = decor->surface;
	handler->handler = InternalHandleDecorator;
	handler->data = (void*)decor;

	InternalDrawDecor(decor->pixels, width, height, flags, handler);
	wl_subsurface_set_position(decor->subsurface, 0, -height);

	wl_surface_attach(decor->surface, decor->buffer, 0, 0);
	wl_surface_commit(decor->surface);
}

b32 InternalCreateWaylandWindow(WWindowContext* context, const s8* title,
				WCreateFlags flags, u32 x, u32 y, u32 width,
				u32 height) {
	if (!InternalLoadLibraryWayland()) {
		return false;
	}

	// get all the functions needed for init

	auto xkb_context_new_fptr = (xkb_context * (*)(xkb_context_flags))
	    LGetLibFunction(xkb_lib, "xkb_context_new");

	auto wl_display_connect_fptr = (wl_display * (*)(const s8*))
	    LGetLibFunction(wwindowlib_handle, "wl_display_connect");

	auto wl_display_dispatch_fptr = (s32(*)(wl_display*))LGetLibFunction(
	    wwindowlib_handle, "wl_display_dispatch");
	auto wl_display_roundtrip_fptr = (s32(*)(wl_display*))LGetLibFunction(
	    wwindowlib_handle, "wl_display_roundtrip");

	auto display = wl_display_connect_fptr(0);
	xkb_ctx = xkb_context_new_fptr(XKB_CONTEXT_NO_FLAGS);

	if (!display || !xkb_ctx) {
		if (display) {
			auto wl_display_disconnect_fptr =
			    (void (*)(wl_display*))LGetLibFunction(
				wwindowlib_handle, "wl_display_disconnect");

			wl_display_disconnect_fptr(display);
		}

		if (xkb_ctx) {
			auto xkb_context_unref_fptr =
			    (void (*)(xkb_context*))LGetLibFunction(
				xkb_lib, "xkb_context_unref");

			xkb_context_unref_fptr(xkb_ctx);
		}

		// MARK: we probably don't need to do this
		InternalUnloadLibraryWayland();

		return false;
	}

	auto wdata = (WaylandData*)&context->data->wayland_data;

	InternalLoadWaylandSymbols();
	InternalLoadXkbSymbols();

	// wayland stuff
	wl_registry* registry = wl_display_get_registry(display);

	wl_registry_add_listener(registry, &registry_listener, (void*)context);

	wl_display_dispatch_fptr(display);
	wl_display_roundtrip_fptr(display);

	// create surfaces

	context->window =
	    (void*)wl_compositor_create_surface(wdata->compositor);

	context->data->wayland_xdg_surface = xdg_wm_base_get_xdg_surface(
	    wdata->base, (wl_surface*)context->window);

	auto wayland_xdg_surface = context->data->wayland_xdg_surface;
	context->data->wayland_xdg_toplevel =
	    xdg_surface_get_toplevel(wayland_xdg_surface);

	auto toplevel = context->data->wayland_xdg_toplevel;

	xdg_surface_add_listener(wayland_xdg_surface,&surface_listener,
			(void*)context);

	xdg_toplevel_add_listener(toplevel,&toplevel_listener,
			(void*)context);

	xdg_toplevel_set_title(toplevel,title);

	//NOTE: we need to add an extra commit here
	
	wl_surface_commit((wl_surface*)context->window);
	wl_display_roundtrip_fptr(display);


	impl_wkeycodetoascii = WKeyCodeToASCIIWayland;
	impl_wwaitforevent = WWaitForWindowEventWayland;
	impl_wsettitle = WSetTitleWayland;
	impl_getwindowsize = GetWindowSizeWayland;
	impl_wpresentbackbuffer = WPresentBackBufferWayland;
	impl_wcreatebackbuffer = WCreateBackBufferWayland;

	context->data->type = _WAYLAND_WINDOW;
	context->data->wayland_data.width = width;
	context->data->wayland_data.height = height;

	context->handle = display;

	InternalCreateWindowDecorator(context, flags);

	return true;
}
