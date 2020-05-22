#pragma once

#include "fcntl.h"
#include "ssys.h"
#include "ttimer.h"
#include "unistd.h"

/*
TODO:
Implement:
move all the global variables to one file


default cursor names --
static const char *cursor_names[] = {
"bottom_left_corner",
"bottom_right_corner",
"bottom_side",
"grabbing",
"left_ptr",
"left_side",
"right_side",
"top_left_corner",
"top_right_corner",
"top_side",
"xterm",
"hand1",
};

*/

#include "mmath.h"
#include "ssys.h"
#include "wayland-cursor.h"
#include "xkbcommon/xkbcommon.h"

#define _enable_resize 1

#define _decor_height 45
#define _element_dim 45
#define _element_thickness 3
#define _element_offset 15
#define _border_thickness 8

_global LibHandle xkb_lib = 0;
_global xkb_context* xkb_ctx = 0;
_global xkb_keymap* xkb_kbmap = 0;
_global xkb_state* xkb_kbstate = 0;

enum InternalCursorType {
	INTERNAL_CURSORTYPE_PTR = 0,
	INTERNAL_CURSORTYPE_LEFT = 1,
	INTERNAL_CURSORTYPE_RIGHT = 2,
	INTERNAL_CURSORTYPE_BOTTOM = 3,
	INTERNAL_CURSORTYPE_BOTTOMRIGHT = 4,
	INTERNAL_CURSORTYPE_NONE = -1,
};

struct InternalCursors {
	wl_surface* surface;
	wl_buffer* buffers[5] = {};
	wl_cursor_theme* theme;

	struct {
		u32 x;
		u32 y;
	} hotspots[5];
};

_global LibHandle cursor_lib = 0;
_global InternalCursors internal_cursors = {};
_global InternalCursorType cur_cursortype = INTERNAL_CURSORTYPE_NONE;

_global WWindowEvent wayland_event_array[32] = {};
_global u32 wayland_event_count = 0;

enum InternElementType {
	INTERN_ELEMENT_NONE = 0,
	INTERN_ELEMENT_HIDE,
	INTERN_ELEMENT_MINMAX,
	INTERN_ELEMENT_DRAG,
	INTERN_ELEMENT_CLOSE,
};

enum InternalWindowStateBit {
	INTERN_WINSTATE_NORESIZE = 1,
	INTERN_WINSTATE_MAXIMIZED = 1 << 1,

};

// TODO: rename this. this now represents external data
// the user doesn't need to handle themselves
struct InternWaylandDecorator {
	wl_surface* decor_surface;
	wl_subsurface* subsurface;

	wl_surface* parent_surface;

	u32 state_flag;
	u32 time;

	WBackBufferContext backbuffer;

	union {
		xdg_toplevel* parent_toplevel;
		// It could be pop up or whatever
	};

	struct Element {
		u32 dim;
		u32 x;
		u32 y;

		InternElementType type;
	};

	union {
		Element elements[4];
	};
};

_global wl_compositor* wayland_compositor = 0;
_global wl_subcompositor* wayland_subcompositor = 0;
_global xdg_wm_base* wayland_base = 0;
_global wl_seat* wayland_seat = 0;
_global wl_pointer* wayland_pointer = 0;
_global wl_keyboard* wayland_keyboard = 0;

// for sw rendering
_global wl_shm* wayland_shm;

// NOTE: should we limit to 32 windows??
_global InternWaylandDecorator decorator_array[32] = {};
_global u32 decorator_count = 0;

_global wl_surface* active_kb_window = 0;
_global wl_surface* active_ms_window = 0;
_global Vec2 active_mouse_pos = {};

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

WWindowEvent* InternalGetNextEvent() {
	_kill("too many events\n",
			wayland_event_count > _arraycount(wayland_event_array));

	auto event = &wayland_event_array[wayland_event_count];
	wayland_event_count++;

	return event;
}

void InternalPushEvent(WWindowEvent event) {
	wayland_event_array[wayland_event_count] = event;
	wayland_event_count++;
}

u32 InternalTopEvent(WWindowEvent* event) {
	if (wayland_event_count) {
		*event = wayland_event_array[wayland_event_count - 1];

		return 1;
	}

	return 0;
}

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

	cursor_lib = LLoadLibrary("libwayland-cursor.so");

	if (!wwindowlib_handle || !xkb_lib || !cursor_lib) {
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

	LUnloadLibrary(cursor_lib);
	cursor_lib = 0;
}

// wayland stuff

s8 WKeyCodeToASCIIWayland(u32 keycode) {
	s8 buffer[128] = {};

	xkb_state_key_get_utf8_fptr(xkb_kbstate, keycode, buffer,
			sizeof(buffer));

	return buffer[0];
}

#include "debugtimer.h"

u32 WWaitForWindowEventWayland(WWindowEvent* event) {
	auto display = internal_windowconnection.wayland_display;

	auto fd = wl_display_get_fd(display);

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
		wl_display_dispatch(display);
	}

	else {
		wl_display_dispatch_pending(display);
	}

	return InternalTopEvent(event);
}

void WRetireEventWayland(WWindowEvent* event) { wayland_event_count--; }

void WSetTitleWayland(WWindowContext* context, const s8* title) {
	xdg_toplevel* toplevel = 0;

	for (u32 i = 0; i < decorator_count; i++) {
		auto decor = &decorator_array[i];

		if (decor->parent_surface == context->window) {
			toplevel = decor->parent_toplevel;
			break;
		}
	}

	_kill("", !toplevel);

	xdg_toplevel_set_title(toplevel, title);
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
		wl_surface* surface, wl_array* keys) {
#ifdef DEBUG
	// printf("keyboard enter surface %p\n", (void*)surface);
#endif

	active_kb_window = surface;
}

void WaylandKeyboardLeave(void* data, wl_keyboard* keyboard, u32 serial,
		wl_surface* surface) {}

void WaylandKeyboardKey(void* data, wl_keyboard* keyboard, u32 serial, u32 time,
		u32 key, u32 state) {
	auto event = InternalGetNextEvent();

	if (state) {
		event->type = W_EVENT_KBEVENT_KEYDOWN;
	}

	else {
		event->type = W_EVENT_KBEVENT_KEYUP;
	}

	event->keyboard_event.keycode = key + 8;

	event->window = (u64)active_kb_window;
}

void WaylandKeyboardModifiers(void* data, wl_keyboard* keyboard, u32 serial,
		u32 mods_depressed, u32 mods_latched,
		u32 mods_locked, u32 group) {}

void WaylandPointerEnter(void* data, wl_pointer* pointer, u32 serial,
		wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy) {
	// NOTE: the serial needs to be stored to perform drag operations
	// NOTE: We can check for subsurface here

#ifdef DEBUG
	// printf("mouse enter surface %p \n", (void*)surface);
#endif
	active_ms_window = surface;

	InternalCursorType type = INTERNAL_CURSORTYPE_PTR;

	if (active_ms_window != active_kb_window &&
			active_mouse_pos.y >= _decor_height) {
		InternWaylandDecorator* decor = 0;

		for (u32 i = 0; i < decorator_count; i++) {
			auto d = &decorator_array[i];

			if (active_ms_window == d->decor_surface) {
				decor = d;
				break;
			}
		}

		_kill("", !decor);

		auto t_width = decor->backbuffer.width - _border_thickness;
		auto t_height = decor->backbuffer.height - _border_thickness;

		if (active_mouse_pos.y > t_height &&
				active_mouse_pos.x > t_width) {
			type = INTERNAL_CURSORTYPE_BOTTOMRIGHT;
		}

		else if (active_mouse_pos.y > t_height) {
			type = INTERNAL_CURSORTYPE_BOTTOM;
		}

		else if (active_mouse_pos.x < _border_thickness) {
			type = INTERNAL_CURSORTYPE_LEFT;
		}

		else if (active_mouse_pos.x > t_width) {
			type = INTERNAL_CURSORTYPE_RIGHT;
		}
	}

	if (type != cur_cursortype) {
		wl_surface_attach(internal_cursors.surface,
				internal_cursors.buffers[type], 0, 0);
		wl_surface_commit(internal_cursors.surface);

		auto hotspot = internal_cursors.hotspots[type];

		wl_pointer_set_cursor(pointer, serial, internal_cursors.surface,
				hotspot.x, hotspot.y);
	}
}

void WaylandPointerLeave(void* data, wl_pointer* pointer, u32 serial,
		wl_surface* surface) {
	// NOTE: the serial needs to be stored to perform drag operations
}

void WaylandPointerMotion(void* data, wl_pointer* pointer, u32 time,
		wl_fixed_t sx, wl_fixed_t sy) {
	active_mouse_pos.x = wl_fixed_to_int(sx);
	active_mouse_pos.y = wl_fixed_to_int(sy);

	if (active_kb_window == active_ms_window) {
		auto event = InternalGetNextEvent();

		event->type = W_EVENT_MSEVENT_MOVE;
		event->mouse_event.x = wl_fixed_to_int(sx);
		event->mouse_event.y = wl_fixed_to_int(sy);
		event->window = (u64)active_ms_window;
	}
}

void InternalHandleDecoratorInput(InternWaylandDecorator* decor, u32 serial,
		u32 state, u32 time) {
	for (u32 i = 0; i < _arraycount(decor->elements); i++) {
		auto el = &decor->elements[i];

		auto rect_intersection = [](u32 r_x, u32 r_y, u32 dim, u32 m_x,
				u32 m_y) -> b32 {
			u32 x_end = r_x + dim;
			u32 y_end = r_y + dim;
			return m_x > r_x && m_x < x_end && m_y > r_y &&
				m_y < y_end;
		};

		if (rect_intersection(el->x, el->y, _element_dim,
					active_mouse_pos.x, active_mouse_pos.y)) {
			switch (el->type) {
				case INTERN_ELEMENT_NONE: {
								  printf("NONE\n");
								  goto exit_switch;
							  } break;

				case INTERN_ELEMENT_HIDE: {
								  xdg_toplevel_set_minimized(
										  decor->parent_toplevel);

							  } break;

				case INTERN_ELEMENT_MINMAX: {
								    u32 diff = time - decor->time;

								    if (diff > 100) {
									    decor->time = time;
									    if (decor->state_flag &
											    INTERN_WINSTATE_MAXIMIZED) {
										    xdg_toplevel_unset_maximized(
												    decor
												    ->parent_toplevel);
									    } else {
										    xdg_toplevel_set_maximized(
												    decor
												    ->parent_toplevel);
									    }

									    decor->state_flag ^=
										    INTERN_WINSTATE_MAXIMIZED;
								    }

							    } break;

				case INTERN_ELEMENT_CLOSE: {
								   auto event = InternalGetNextEvent();
								   event->type = W_EVENT_CLOSE;
								   event->window =
									   (u64)decor->parent_surface;
							   } break;
			}

			return;
		}
	}

exit_switch:

	auto t_width = decor->backbuffer.width - _border_thickness;
	auto t_height = decor->backbuffer.height - _border_thickness;

	if (active_mouse_pos.y < _decor_height) {
		xdg_toplevel_move(decor->parent_toplevel, wayland_seat, serial);
	}

	else if (active_mouse_pos.y > t_height &&
			active_mouse_pos.x > t_width) {
		xdg_toplevel_resize(decor->parent_toplevel, wayland_seat,
				serial,
				XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT);

	}

	else if (active_mouse_pos.y > t_height) {
		xdg_toplevel_resize(decor->parent_toplevel, wayland_seat,
				serial, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM);
	}

	else if (active_mouse_pos.x < _border_thickness) {
		xdg_toplevel_resize(decor->parent_toplevel, wayland_seat,
				serial, XDG_TOPLEVEL_RESIZE_EDGE_LEFT);
	} else if (active_mouse_pos.x > t_width) {
		xdg_toplevel_resize(decor->parent_toplevel, wayland_seat,
				serial, XDG_TOPLEVEL_RESIZE_EDGE_RIGHT);
	}
}

void WaylandPointerButton(void* data, wl_pointer* pointer, u32 serial, u32 time,
		u32 button, u32 state) {
#if 0
	printf("TIME %d\n",time);
#endif

	// This is to handle window decorators

	if (active_ms_window != active_kb_window) {
		for (u32 i = 0; i < decorator_count; i++) {
			auto d = &decorator_array[i];

			if (active_ms_window == d->decor_surface) {
				InternalHandleDecoratorInput(d, serial, state,
						time);
				return;
			}
		}
	}

	// NOTE: the serial needs to be stored to perform drag operations

	auto event = InternalGetNextEvent();

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

	event->window = (u64)active_ms_window;
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

void PrintTopLevelState(xdg_toplevel_state state){

	switch(state){
		case XDG_TOPLEVEL_STATE_MAXIMIZED:{
							  printf("MAXMIMIZED\n");
						  }break;
		case XDG_TOPLEVEL_STATE_FULLSCREEN:{
							   printf("FULLSCREEN\n");
						   }break;
		case XDG_TOPLEVEL_STATE_RESIZING:{
							 printf("RESIZING\n");
						 }break;
		case XDG_TOPLEVEL_STATE_ACTIVATED:{
							  printf("ACTIVATED\n");
						  }break;
		case XDG_TOPLEVEL_STATE_TILED_LEFT:{
							   printf("LEFT\n");
						   }break;
		case XDG_TOPLEVEL_STATE_TILED_RIGHT:{
							    printf("RIGHT\n");
						    }break;
		case XDG_TOPLEVEL_STATE_TILED_TOP:{
							  printf("TOP\n");
						  }break;
		case XDG_TOPLEVEL_STATE_TILED_BOTTOM:{
							     printf("BOTTOM\n");
						     }break;
	}

}

void Wayland_TopConfigure(void* data, xdg_toplevel* toplevel, s32 width,
		s32 height, wl_array* states) {
	// see xdg-shell.h xdg_toplevel_state

#ifdef DEBUG
	printf("TOP CONFIGURE WIDTH %d HEIGHT %d KB_SURFACE %p\n", width,
			height, (void*)active_kb_window);
#endif

	xdg_toplevel_state* cur_state = 0;

	for (cur_state = (xdg_toplevel_state*)states->data;
			(s8*)cur_state < (s8*)(states->data) + states->size; cur_state++) {

		PrintTopLevelState(*cur_state);

		switch (*cur_state) {
			case XDG_TOPLEVEL_STATE_FULLSCREEN: {
							    } break;

			case XDG_TOPLEVEL_STATE_ACTIVATED: {
								   auto event = InternalGetNextEvent();
								   event->type = W_EVENT_EXPOSE;
								   event->window = (u64)active_kb_window;
							   } break;

			default: {
					 // NOTE: We do not post if a specific dim isn't
					 // given We are getting 0,0 when we scale down
					 if ((width + height) && active_kb_window) {
						 // NOTE: disabling for now. we are gonna
						 // block all paths to resizing.idk if
						 // that will work
#if 1

						 for (u32 i = 0; i < decorator_count;
								 i++) {
							 auto d = &decorator_array[i];

							 if (d->parent_surface ==
									 active_kb_window &&
									 d->state_flag &
									 INTERN_WINSTATE_NORESIZE) {
								 return;
							 }
						 }

#endif

						 width = width - (2 * _border_thickness);
						 height = height - (_decor_height +
								 _border_thickness);

						 WWindowEvent* event = 0;

						 if (wayland_event_count) {
							 event =
								 &wayland_event_array
								 [wayland_event_count];
							 if (event->type !=
									 W_EVENT_RESIZE ||
									 event->window !=
									 (u64)active_kb_window) {
								 goto get_next_event;
							 }
						 } else {
get_next_event:
							 event = InternalGetNextEvent();
							 event->type = W_EVENT_RESIZE;
						 }

						 event->width = width;
						 event->height = height;
						 event->window = (u64)active_kb_window;
					 }
				 } break;
		}
	}
}

void Wayland_Close(void* data, xdg_toplevel* toplevel) {
	auto event = InternalGetNextEvent();
	event->type = W_EVENT_CLOSE;
	event->window = (u64)active_kb_window;
}

void Wayland_SurfaceConfigure(void* data, xdg_surface* surface, u32 serial) {
	auto context = (WWindowContext*)data;

#ifdef DEBUG

	printf("SURFACE CONFIGURE\n");

#endif

	// TODO: we should move this as an internal event
	xdg_surface_ack_configure(surface, serial);
}

_global wl_pointer_listener pointer_listener = {
	WaylandPointerEnter, WaylandPointerLeave, WaylandPointerMotion,
	WaylandPointerButton, WaylandPointerAxis};

_global wl_keyboard_listener keyboard_listener = {
	WaylandKeyboardMap, WaylandKeyboardEnter, WaylandKeyboardLeave,
	WaylandKeyboardKey, WaylandKeyboardModifiers};

void SeatCapabilities(void* data, wl_seat* seat, u32 caps) {
	if (caps & WL_SEAT_CAPABILITY_POINTER) {
		wayland_pointer = wl_seat_get_pointer(seat);

		wl_pointer_add_listener(wayland_pointer, &pointer_listener,
				data);
	}

	if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
		wayland_keyboard = wl_seat_get_keyboard(seat);

		wl_keyboard_add_listener(wayland_keyboard, &keyboard_listener,
				data);
	}
}

_global const wl_seat_listener seat_listener = {SeatCapabilities};
_global wl_shm_listener shm_listener = {WaylandSHMFormat};

_global const xdg_wm_base_listener wm_base_listener = {Wayland_Ping};

_global const xdg_surface_listener surface_listener = {
	Wayland_SurfaceConfigure};

_global const xdg_toplevel_listener toplevel_listener = {Wayland_TopConfigure,
	Wayland_Close};

void Wayland_Display_Handle_Global(void* data, struct wl_registry* registry,
		u32 id, const s8* interface, u32 version) {
	if (PHashString(interface) == PHashString("wl_compositor")) {
		wayland_compositor = (wl_compositor*)wl_registry_bind(
				registry, id, &wl_compositor_interface, 1);
	}

	if (PHashString(interface) == PHashString("wl_subcompositor")) {
		wayland_subcompositor = (wl_subcompositor*)wl_registry_bind(
				registry, id, &wl_subcompositor_interface, 1);
	}

	if (PHashString(interface) == PHashString("xdg_wm_base")) {
		wayland_base = (xdg_wm_base*)wl_registry_bind(
				registry, id, &xdg_wm_base_interface, 2);
		xdg_wm_base_add_listener(wayland_base, &wm_base_listener, data);
	}

	if (PHashString(interface) == PHashString("wl_seat")) {
		wayland_seat = (wl_seat*)wl_registry_bind(
				registry, id, &wl_seat_interface, 1);
		wl_seat_add_listener(wayland_seat, &seat_listener, data);
	}

	// NOTE: this is needed for sw rendering
	if (PHashString(interface) == PHashString("wl_shm")) {
		wayland_shm = (wl_shm*)wl_registry_bind(registry, id,
				&wl_shm_interface, 1);
		wl_shm_add_listener(wayland_shm, &shm_listener, 0);
	}

#ifdef DEBUG
	printf("interface: %s\n", interface);
#endif
}

_global const wl_registry_listener registry_listener = {
	Wayland_Display_Handle_Global, 0};

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
			wwindowlib_handle, "wl_output_interface");
}

void InternalLoadXkbSymbols() {
	xkb_state_key_get_utf8_fptr =
		(s32(*)(xkb_state*, xkb_keycode_t, s8*, size_t))LGetLibFunction(
				xkb_lib, "xkb_state_key_get_utf8");
}

void GetWindowSizeWayland(WWindowContext* window, u32* w, u32* h) {
	InternWaylandDecorator* decor = 0;

	for (u32 i = 0; i < decorator_count; i++) {
		auto d = &decorator_array[i];

		if (window->window == d->parent_surface) {
			decor = d;
			break;
		}
	}

	_kill("", !decor);

	u32 border_thickness = _border_thickness;

	if (decor->state_flag & INTERN_WINSTATE_NORESIZE) {
		border_thickness = 0;
	}
	*w = decor->backbuffer.width - (2 * border_thickness);
	*h = decor->backbuffer.height - (_decor_height + border_thickness);
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

void WDestroyBackBufferWayland(WBackBufferContext* buffer) {
	wl_buffer_destroy(buffer->data->buffer);

	munmap(buffer->pixels, 4 * buffer->width * buffer->height);
	close(buffer->data->fd);
	unalloc(buffer->data);

	*buffer = {};
}

WBackBufferContext InternalCreateBackBufferWayland(
		WWindowContext* windowcontext, u32 override_w, u32 override_h) {
	u32 width = 0;
	u32 height = 0;

	// FIXME: technically, we can only have one

	if (windowcontext) {
		GetWindowSizeWayland(windowcontext, &width, &height);
	}

	if (override_w) {
		width = override_w;
	}

	if (override_h) {
		height = override_h;
	}

	_kill("", !(width + height));

	auto shm = wayland_shm;
	auto size = width * height * 4;

	WBackBufferContext backbuffer = {};

	backbuffer.data =
		(InternalBackBufferData*)alloc(sizeof(InternalBackBufferData));

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

	backbuffer.data->fd = fd;

	return backbuffer;
}

WBackBufferContext WCreateBackBufferWayland(WWindowContext* windowcontext) {
	return InternalCreateBackBufferWayland(windowcontext, 0, 0);
}

void WPresentBackBufferWayland(WWindowContext* windowcontext,
		WBackBufferContext* buffer) {
	wl_surface_attach((wl_surface*)windowcontext->window,
			buffer->data->buffer, 0, 0);
	wl_surface_commit((wl_surface*)windowcontext->window);
}

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

// TODO: we have to draw the title string
void _ainline InternalDrawDecor(InternWaylandDecorator* decor, u32 width,
		u32 height, WCreateFlags flags) {
	auto pixels = decor->backbuffer.pixels;

	// clear color
	for (u32 i = 0; i < (width * height); i++) {
		u32* pixel = pixels + i;
		*pixel = _encode_argb(255, 36, 36, 36);
	}

	u32 count = 3;

	if (flags & W_CREATE_NORESIZE) {
		count--;
	}
	u32 startx = width - (count * _element_dim);

	u32 element_count = 0;

	InternalDrawHide(pixels + startx, width, height);

	decor->elements[element_count] = {_element_dim, startx, 0,
		INTERN_ELEMENT_HIDE};
	element_count++;

	if (!(flags & W_CREATE_NORESIZE)) {
		startx += _element_dim;
		InternalDrawMinMax(pixels + startx, width, height);

		decor->elements[element_count] = {_element_dim, startx, 0,
			INTERN_ELEMENT_MINMAX};
		element_count++;
	}

	startx += _element_dim;
	InternalDrawClose(pixels + startx, width, height);

	decor->elements[element_count] = {_element_dim, startx, 0,
		INTERN_ELEMENT_CLOSE};

	element_count++;
}

void WAckResizeEventWayland(WWindowEvent* event) {
	InternWaylandDecorator* decor = 0;

	for (u32 i = 0; i < decorator_count; i++) {
		auto d = &decorator_array[i];

		if (d->parent_surface == active_kb_window) {
			decor = d;
			break;
		}
	}

	// FIXME: the resize issue seems to stem from here
#if _enable_resize

	u32 border_thickness = _border_thickness;

	if (decor->state_flag & INTERN_WINSTATE_NORESIZE) {
		border_thickness = 0;
	}

	u32 width = event->width + (2 * border_thickness);
	u32 height = _decor_height + event->height + border_thickness;

	auto old_buffer = decor->backbuffer;

	decor->backbuffer = InternalCreateBackBufferWayland(0, width, height);

	InternalDrawDecor(decor, width, height, (WCreateFlags)0);

	wl_surface_attach(decor->decor_surface, decor->backbuffer.data->buffer,
			0, 0);
	wl_surface_commit(decor->decor_surface);

	WDestroyBackBufferWayland(&old_buffer);
#endif
}

b32 InternalCreateDefaultCursors() {

	LibHandle lib = cursor_lib; 

	if (!lib) {
		return false;
	}

	auto wl_cursor_theme_load_fptr =
		(wl_cursor_theme * (*)(const s8*, s32, wl_shm*))
		LGetLibFunction(lib, "wl_cursor_theme_load");

	auto wl_cursor_theme_get_cursor_fptr =
		(wl_cursor * (*)(wl_cursor_theme*, const s8*))
		LGetLibFunction(lib, "wl_cursor_theme_get_cursor");

	auto wl_cursor_image_get_buffer_fptr =
		(wl_buffer * (*)(wl_cursor_image*))
		LGetLibFunction(lib, "wl_cursor_image_get_buffer");


	auto theme = wl_cursor_theme_load_fptr(0, 16, wayland_shm);

	auto cursor = wl_cursor_theme_get_cursor_fptr(theme, "left_ptr");
	internal_cursors.buffers[0] =
		wl_cursor_image_get_buffer_fptr(cursor->images[0]);
	internal_cursors.hotspots[0].x = cursor->images[0]->hotspot_x;
	internal_cursors.hotspots[0].y = cursor->images[0]->hotspot_y;

	cursor = wl_cursor_theme_get_cursor_fptr(theme, "left_side");
	internal_cursors.buffers[1] =
		wl_cursor_image_get_buffer_fptr(cursor->images[0]);
	internal_cursors.hotspots[1].x = cursor->images[0]->hotspot_x;
	internal_cursors.hotspots[1].y = cursor->images[0]->hotspot_y;

	cursor = wl_cursor_theme_get_cursor_fptr(theme, "right_side");
	internal_cursors.buffers[2] =
		wl_cursor_image_get_buffer_fptr(cursor->images[0]);
	internal_cursors.hotspots[2].x = cursor->images[0]->hotspot_x;
	internal_cursors.hotspots[2].y = cursor->images[0]->hotspot_y;

	cursor = wl_cursor_theme_get_cursor_fptr(theme, "bottom_side");
	internal_cursors.buffers[3] =
		wl_cursor_image_get_buffer_fptr(cursor->images[0]);
	internal_cursors.hotspots[3].x = cursor->images[0]->hotspot_x;
	internal_cursors.hotspots[3].y = cursor->images[0]->hotspot_y;

	cursor = wl_cursor_theme_get_cursor_fptr(theme, "bottom_right_corner");
	internal_cursors.buffers[4] =
		wl_cursor_image_get_buffer_fptr(cursor->images[0]);
	internal_cursors.hotspots[4].x = cursor->images[0]->hotspot_x;
	internal_cursors.hotspots[4].y = cursor->images[0]->hotspot_y;


	internal_cursors.surface =
		wl_compositor_create_surface(wayland_compositor);

	internal_cursors.theme = theme;

	return true;
}

void InternalCreateWindowDecorator(WWindowContext* context, u32 w, u32 h,
		xdg_toplevel* toplevel, WCreateFlags flags) {
	auto decor = &decorator_array[decorator_count];
	decorator_count++;

	decor->decor_surface = wl_compositor_create_surface(wayland_compositor);

	decor->parent_surface = (wl_surface*)context->window;
	decor->parent_toplevel = toplevel;

	decor->subsurface = wl_subcompositor_get_subsurface(
			wayland_subcompositor, decor->decor_surface,
			(wl_surface*)context->window);

	// TODO: this should be scaled by screen height

	u32 border_thickness = _border_thickness;

	if (flags & W_CREATE_NORESIZE) {
		border_thickness = 0;
		decor->state_flag |= INTERN_WINSTATE_NORESIZE;
	}

	u32 width = w + (2 * border_thickness);
	u32 height = _decor_height + h + border_thickness;

	decor->backbuffer =
		InternalCreateBackBufferWayland(context, width, height);

	InternalDrawDecor(decor, width, height, flags);
	wl_subsurface_set_position(decor->subsurface, -border_thickness,
			-_decor_height);

	wl_subsurface_place_below(decor->subsurface, decor->parent_surface);

	wl_surface_attach(decor->decor_surface, decor->backbuffer.data->buffer,
			0, 0);
	wl_surface_commit(decor->decor_surface);
}

b32 InternalWaylandInitOneTime() {
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
wayland_shutdown:

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

		if(internal_cursors.theme){
			auto wl_cursor_theme_destroy_fptr =
				(void (*)(wl_cursor_theme*))LGetLibFunction(
						cursor_lib, "wl_cursor_theme_destroy");

			wl_cursor_theme_destroy_fptr(internal_cursors.theme);
		}

		InternalUnloadLibraryWayland();

		return false;
	}

	InternalLoadWaylandSymbols();
	InternalLoadXkbSymbols();

	// wayland stuff
	wl_registry* registry = wl_display_get_registry(display);

	wl_registry_add_listener(registry, &registry_listener, 0);

	wl_display_dispatch_fptr(display);
	wl_display_roundtrip_fptr(display);

	internal_windowconnection.wayland_display = display;

	if (!InternalCreateDefaultCursors()) {
		goto wayland_shutdown;
	}

	return true;
}

b32 InternalCreateWaylandWindow(WWindowContext* context, const s8* title,
		WCreateFlags flags, u32 x, u32 y, u32 width,
		u32 height) {
	if (!internal_windowconnection.wayland_display) {
		if (!InternalWaylandInitOneTime()) {
			return false;
		}
	}

	auto wl_display_roundtrip_fptr = (s32(*)(wl_display*))LGetLibFunction(
			wwindowlib_handle, "wl_display_roundtrip");

	auto display = internal_windowconnection.wayland_display;

	// create surfaces

	context->window =
		(void*)wl_compositor_create_surface(wayland_compositor);

	context->data->wayland_xdg_surface = xdg_wm_base_get_xdg_surface(
			wayland_base, (wl_surface*)context->window);

	auto wayland_xdg_surface = context->data->wayland_xdg_surface;
	auto toplevel = xdg_surface_get_toplevel(wayland_xdg_surface);

	xdg_surface_add_listener(wayland_xdg_surface, &surface_listener, 0);

	xdg_toplevel_add_listener(toplevel, &toplevel_listener, 0);

	xdg_toplevel_set_title(toplevel, title);

	xdg_toplevel_set_min_size(toplevel, width, height);

	// NOTE: we need to add an extra commit here

	wl_surface_commit((wl_surface*)context->window);
	wl_display_roundtrip_fptr(display);

	impl_wkeycodetoascii = WKeyCodeToASCIIWayland;
	impl_wwaitforevent = WWaitForWindowEventWayland;
	impl_wsettitle = WSetTitleWayland;
	impl_getwindowsize = GetWindowSizeWayland;
	impl_wpresentbackbuffer = WPresentBackBufferWayland;
	impl_wcreatebackbuffer = WCreateBackBufferWayland;
	impl_wdestroybackbuffer = WDestroyBackBufferWayland;
	impl_wackresizeevent = WAckResizeEventWayland;
	impl_wretireevent = WRetireEventWayland;

	context->data->type = _WAYLAND_WINDOW;

	InternalCreateWindowDecorator(context, width, height, toplevel, flags);

	return true;
}
