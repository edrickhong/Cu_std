#pragma once

#include "ttype.h"

#ifdef _WIN32

#include "windows.h"

#else

#include "X11/Xlib.h"
#include "X11/Xutil.h"

#endif

/*
TODO:
set icon
drag and drop with xdnd protocol
message box?
*/

typedef struct InternalWindowData InternalWindowData;
typedef struct InternalBackBufferData InternalBackBufferData;

typedef struct WWindowContext {
	void* window;

	InternalWindowData*
	    data;  // hidden data that isn't accessed often anyway
} WWindowContext;

// backbuffer used for sw rendering
typedef struct WBackBufferContext {
	u32* pixels;
	u16 width;
	u16 height;
	InternalBackBufferData* data;
} WBackBufferContext;

#ifdef _WIN32

#define _WAYLAND_WINDOW ((u32)(0))
#define _X11_WINDOW ((u32)(0))

#else

#define _WAYLAND_WINDOW ((u32)(1 << 31))
#define _X11_WINDOW ((u32)(1 << 30))

#endif


typedef enum WCreateFlags {
	W_CREATE_NONE = 0,
	W_CREATE_NORESIZE = 1,

} WCreateFlags;

typedef enum WEventType {
	W_EVENT_NONE = 0,
	W_EVENT_CLOSE = (s32)0xFFFFFFFF,
	W_EVENT_RESIZE = (s32)0xFFFFFFFE,

	//NOTE: These are internal events that 
	//the user will never see
	
	W_EVENT_INTERN_MS_I = (s32)0xFFFFFFFD,
	W_EVENT_INTERN_MS_O = (s32)0xFFFFFFFC,
	W_EVENT_INTERN_KB_I = (s32)0xFFFFFFFB,
	W_EVENT_INTERN_KB_O = (s32)0xFFFFFFFA,

#ifdef _WIN32

	W_EVENT_EXPOSE = WM_ACTIVATEAPP,
	W_EVENT_KBEVENT_KEYDOWN = WM_KEYDOWN,
	W_EVENT_KBEVENT_KEYUP = WM_KEYUP,
	W_EVENT_MSEVENT_MOVE = WM_MOUSEMOVE,

#else

	W_EVENT_EXPOSE = Expose,
	W_EVENT_KBEVENT_KEYDOWN = KeyPress,
	W_EVENT_KBEVENT_KEYUP = KeyRelease,
	W_EVENT_MSEVENT_MOVE = MotionNotify,

#endif

	W_EVENT_MSEVENT_DOWN,
	W_EVENT_MSEVENT_UP,
} WEventType;

typedef struct WKeyboardEvent {
	u32 keycode;
} WKeyboardEvent;

typedef struct WMouseEvent {
	union {
		struct {
			// relative to the top left corner of the window
			u16 x;
			u16 y;
		};
		u32 keycode;
	};
} WMouseEvent;

typedef struct WWindowEvent {
	WEventType type; //32 bits

	union {
		WKeyboardEvent keyboard_event;
		WMouseEvent mouse_event;

		struct {
			u16 width;
			u16 height;
			b32 ack_resize;
		};
	}; // 32 bits
	u64 window; 
} WWindowEvent;

typedef struct KeyboardState {
	s8 prevkeystate[256];
	s8 curkeystate[256];
} KeyboardState;

typedef struct MouseState {
	u16 x;
	u16 y;
	s8 curstate[8];
	s8 prevstate[8];
} MouseState;

typedef enum MouseButton {
	MOUSEBUTTON_LEFT = 0,
	MOUSEBUTTON_RIGHT = 1,
	MOUSEBUTTON_MIDDLE = 2,
	MOUSEBUTTON_BUTTON1 = 3,
	MOUSEBUTTON_BUTTON2 = 4,
	MOUSEBUTTON_SCROLLUP = 5,
	MOUSEBUTTON_SCROLLDOWN = 6,
} MouseButton;

typedef enum WPlatform{
	WPLATFORM_NONE,
	WPLATFORM_WIN32,
	WPLATFORM_X11,
	WPLATFORM_WAYLAND,
} WPlatform;

#ifdef __cplusplus
extern "C" {
#endif


void WGetPlatforms(WPlatform* array,u32* count,b32 vulkan_enabled);
void WCreateWindowConnection(WPlatform platform);
void WDestroyWindowConnection();

void* WGetWindowConnection();

u32 WWaitForWindowEvent(WWindowEvent* event);
void WRetireEvent(WWindowEvent* event);
void WAckResizeEvent(WWindowEvent* event);
void WIgnoreResizeEvent(WWindowEvent* event);

// TODO: we should make selecting the backend a separate thing
WWindowContext WCreateWindow(const s8* title, 
		WCreateFlags flags, u32 x, u32 y,u32 width, u32 height);

WBackBufferContext WCreateBackBuffer(WWindowContext* windowcontext);

void WDestroyBackBuffer(WBackBufferContext* buffer);

void WPresentBackBuffer(WWindowContext* windowcontext,
			WBackBufferContext* buffer);

void WDestroyWindow(WWindowContext* windowcontext);

// destroy and removes a subwindow at index from the subwindow list
void WDestroySubWindow(WWindowContext* windowcontext, ptrsize index);

void WMessageBox(WWindowContext* windowcontext, const s8* text,
		 const s8* caption, u32 type);

// TODO: implement this (win32,xlib,wayland)
void WSetIcon(WWindowContext* windowcontext, void* icondata, u32 width,
	      u32 height);

void WSetTitle(WWindowContext* windowcontext, const s8* title_string);

s8 WKeyCodeToASCII(u32 keycode);

b32 _ainline IsKeyPressed(KeyboardState* state, u32 keysym) {
	return state->curkeystate[keysym] && !state->prevkeystate[keysym];
}

b32 _ainline IsKeyDown(KeyboardState* state, u32 keysym) {
	return state->curkeystate[keysym];
}

b32 _ainline IsKeyUp(KeyboardState* state, u32 keysym) {
	return !state->curkeystate[keysym];
}

b32 _ainline IsMouseDown(MouseState* state, MouseButton mousekey) {
	return state->curstate[mousekey];
}

b32 _ainline IsMouseUp(MouseState* state, MouseButton mousekey) {
	return !state->curstate[mousekey];
}


b32 _ainline OnMouseUp(MouseState* state, MouseButton mousekey) {
	return !state->curstate[mousekey] && state->prevstate[mousekey] ;
}

#ifdef __cplusplus
}
#endif
