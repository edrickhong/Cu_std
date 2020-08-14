#pragma once

_global void* wfptr_x11_xstorename = 0;
_global void* wfptr_x11_xflush = 0;
_global void* wfptr_x11_xpending = 0;
_global void* wfptr_x11_xnextevent = 0;
_global void* wfptr_x11_xpeekevent = 0;
_global void* wfptr_x11_xsetwmnormalhints = 0;
_global void* wfptr_x11_xrefreshkeyboardmapping = 0;
_global void* wfptr_x11_xsync = 0;
_global void* wfptr_x11_xsetwmprotocols = 0;
_global void* wfptr_x11_xlookupstring = 0;
_global void* wfptr_x11_xconfigurewindow = 0;

// for presenting
_global void* wfptr_x11_xputimage = 0;
_global void* wfptr_x11_xcopyarea = 0;

b32 InternalLoadLibraryX11() {
	const s8* x11_paths[] = {
	    "libX11.so.6.3.0",
	    "libX11.so.6",
	    "libX11.so",
	};

	for (u32 i = 0; i < _arraycount(x11_paths); i++) {
		wwindowlib_handle = LLoadLibrary(x11_paths[i]);

		if (wwindowlib_handle) {
			break;
		}
	}

	if (!wwindowlib_handle) {
		return false;
	}

	wfptr_x11_xflush = LGetLibFunction(wwindowlib_handle, "XFlush");

	wfptr_x11_xpending = LGetLibFunction(wwindowlib_handle, "XPending");

	wfptr_x11_xnextevent = LGetLibFunction(wwindowlib_handle, "XNextEvent");

	wfptr_x11_xpeekevent = LGetLibFunction(wwindowlib_handle, "XPeekEvent");

	wfptr_x11_xsetwmnormalhints =
	    LGetLibFunction(wwindowlib_handle, "XSetWMNormalHints");

	wfptr_x11_xrefreshkeyboardmapping =
	    LGetLibFunction(wwindowlib_handle, "XRefreshKeyboardMapping");

	wfptr_x11_xsync = LGetLibFunction(wwindowlib_handle, "XSync");

	wfptr_x11_xstorename = LGetLibFunction(wwindowlib_handle, "XStoreName");

	wfptr_x11_xsetwmprotocols =
	    LGetLibFunction(wwindowlib_handle, "XSetWMProtocols");

	// TODO: we should be using the utf8 version: Xutf8LookupString
	// XCreateIC
	wfptr_x11_xlookupstring =
	    LGetLibFunction(wwindowlib_handle, "XLookupString");

	wfptr_x11_xconfigurewindow =
	    LGetLibFunction(wwindowlib_handle, "XConfigureWindow");

	return true;
}

#define XStoreName ((s32(*)(Display*, Window, s8*))wfptr_x11_xstorename)

#define XFlush ((s32(*)(Display*))wfptr_x11_xflush)

#define XPending ((s32(*)(Display*))wfptr_x11_xpending)

#define XNextEvent ((s32(*)(Display*, XEvent*))wfptr_x11_xnextevent)

#define XPeekEvent ((s32(*)(Display*, XEvent*))wfptr_x11_xpeekevent)

#define XSetWMNormalHints \
	((void (*)(Display*, Window, XSizeHints*))wfptr_x11_xsetwmnormalhints)

#define XRefreshKeyboardMapping \
	((s32(*)(XMappingEvent*))wfptr_x11_xrefreshkeyboardmapping)

#define XSync ((s32(*)(Display*, Bool))wfptr_x11_xsync)

#define XSetWMProtocols \
	((Status(*)(Display*, Window, Atom*, s32))wfptr_x11_xsetwmprotocols)

#define XLookupString                           \
	((s32(*)(XKeyEvent*, s8*, s32, KeySym*, \
		 XComposeStatus*))wfptr_x11_xlookupstring)

#define XConfigureWindow                \
	((s32(*)(Display*, Window, u32, \
		 XWindowChanges*))wfptr_x11_xconfigurewindow)

#define XPutImage                                                          \
	((s32(*)(Display*, Drawable, GC, XImage*, s32, s32, s32, s32, u32, \
		 u32))wfptr_x11_xputimage)

#define XCopyArea                                                           \
	((s32(*)(Display*, Drawable, Drawable, GC, s32, s32, u32, u32, s32, \
		 s32))wfptr_x11_xcopyarea)

s8 WKeyCodeToASCIIX11(u32 keycode) { return wtext_buffer[keycode]; }

void WAckResizeEventX11(WWindowEvent* event) {}

void WRetireEventX11(WWindowEvent* event) {
	auto display = internal_windowconnection.x11_display;

	XEvent xevent = {};
	XNextEvent(display, &xevent);
}

u32 WWaitForWindowEventX11(WWindowEvent* event) {
	// NOTE: we might need to create the exit atom. I have an example in the
	// handmade dir

	auto display = internal_windowconnection.x11_display;
	auto queue_count = XPending(display);

	if (queue_count) {
		XEvent xevent = {};
		XPeekEvent(display, &xevent);

		event->window = (u64)xevent.xany.window;

		switch (xevent.type) {
			case Expose: {
				event->type = W_EVENT_EXPOSE;
			} break;

			case ClientMessage: {
				event->type = W_EVENT_CLOSE;
			} break;

			case ConfigureNotify: {
				event->type = W_EVENT_RESIZE;
				event->width = xevent.xconfigure.width;
				event->height = xevent.xconfigure.height;
			} break;

			case KeymapNotify: {
				XRefreshKeyboardMapping(&xevent.xmapping);
			} break;

			case KeyPress: {
				event->type = W_EVENT_KBEVENT_KEYDOWN;
				event->keyboard_event.keycode =
				    xevent.xkey.keycode;

				XLookupString(
				    &xevent.xkey,
				    &wtext_buffer[xevent.xkey.keycode], 1, 0,
				    0);

			} break;

			case KeyRelease: {
				event->type = W_EVENT_KBEVENT_KEYUP;
				event->keyboard_event.keycode =
				    xevent.xkey.keycode;
			} break;

			case MotionNotify: {
				event->type = W_EVENT_MSEVENT_MOVE;
				event->mouse_event.x = xevent.xmotion.x;
				event->mouse_event.y = xevent.xmotion.y;
			} break;

			case ButtonPress: {
				event->type = W_EVENT_MSEVENT_DOWN;

				switch (xevent.xbutton.button) {
					case 1: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_LEFT;
					} break;  // left

					case 2: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_MIDDLE;
					} break;  // middle

					case 3: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_RIGHT;
					} break;  // right

					case 4: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_SCROLLUP;
					} break;  // up

					case 5: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_SCROLLDOWN;
					} break;  // down

					case 8: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_BUTTON1;
					} break;  // button1

					case 9: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_BUTTON2;
					} break;  // button2
				}

			} break;

			case ButtonRelease: {
				event->type = W_EVENT_MSEVENT_UP;

				switch (xevent.xbutton.button) {
					case 1: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_LEFT;
					} break;  // left

					case 2: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_MIDDLE;
					} break;  // middle

					case 3: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_RIGHT;
					} break;  // right

					case 4: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_SCROLLUP;
					} break;  // up

					case 5: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_SCROLLDOWN;
					} break;  // down

					case 8: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_BUTTON1;
					} break;  // button1

					case 9: {
						event->mouse_event.keycode =
						    MOUSEBUTTON_BUTTON2;
					} break;  // button2
				}

			} break;

			default: {
			} break;
		}
	}

	return queue_count;
}

void WSetTitleX11(WWindowContext* context, const s8* title) {
	XStoreName(internal_windowconnection.x11_display,
		   (Window)context->window, (s8*)title);
}

void WGetWindowSizeX11(WWindowContext* window, u32* w, u32* h) {
	auto XGetWindowAttributes_fptr =
	    (Status(*)(Display*, Window, XWindowAttributes*))LGetLibFunction(
		wwindowlib_handle, "XGetWindowAttributes");

	XWindowAttributes attribs = {};

	XGetWindowAttributes_fptr(internal_windowconnection.x11_display,
				  (Window)window->window, &attribs);

	*w = attribs.width;
	*h = attribs.height;
}

void WDestroyBackBufferX11(WBackBufferContext* buffer) {
	auto display = internal_windowconnection.x11_display;

	auto XFreePixmap_fptr = (s32(*)(Display*, Pixmap))LGetLibFunction(
	    wwindowlib_handle, "XFreePixmap");
	auto XFreeGC_fptr =
	    (s32(*)(Display*, GC))LGetLibFunction(wwindowlib_handle, "XFreeGC");

	s32 res = XFreePixmap_fptr(display, buffer->data->pixmap);

	_kill("", res == BadPixmap);

	res = XFreeGC_fptr(display, buffer->data->gc);

	_kill("", res == BadGC);

	unalloc(buffer->data);
}

WBackBufferContext WCreateBackBufferX11(WWindowContext* context) {
	u32 width = 0;
	u32 height = 0;

	WGetWindowSizeX11(context, &width, &height);

	WBackBufferContext buffer = {};

	buffer.data =
	    (InternalBackBufferData*)alloc(sizeof(InternalBackBufferData));

	buffer.width = width;
	buffer.height = height;

	// Init
	// XInitImage //raw pixels
	// XCreatePixmap // pixmap to associate pixels to
	// XCreateGC //backbuffer of the window

	auto XCreatePixmap_fptr =
	    (Pixmap(*)(Display*, Drawable, u32, u32, u32))LGetLibFunction(
		wwindowlib_handle, "XCreatePixmap");
	auto XCreateGC_fptr =
	    (GC(*)(Display*, Drawable, unsigned long,
		   XGCValues*))LGetLibFunction(wwindowlib_handle, "XCreateGC");

	s8* data = (s8*)alloc(width * height * 4);

	buffer.data->ximage = {
	    (s32)width,
	    (s32)height,
	    0,	      /* number of pixels offset in X direction */
	    ZPixmap,  /* XYBitmap, XYPixmap, ZPixmap */
	    data,     /* pointer to image data */
	    LSBFirst, /* data byte order, LSBFirst, MSBFirst */
	    32,	      /* quant. of scanline 8, 16, 32 */
	    LSBFirst, /* LSBFirst, MSBFirst */
	    32,	      /* 8, 16, 32 either XY or ZPixmap */
	    (s32)context->data->x11_depth, /* depth of image */
	    (s32)(width * 4),		   /* accelerator to next scanline */
	    32,				   /* bits per pixel (ZPixmap) */
	    0xFF0000,			   // red mask
	    0x00FF00,			   // green mask
	    0x0000FF,			   // blue mask
	};

	// for testing
#if 0
	auto XInitImage_fptr = (Status(*)(XImage*))LGetLibFunction(
	    wwindowlib_handle, "XInitImage");

	auto XGetImage_fptr = (XImage* (*)(Display*,Drawable,s32,s32,u32,u32,unsigned long,s32))LGetLibFunction(wwindowlib_handle,"XGetImage");

	auto image = XGetImage_fptr((Display*)context->handle,(Window)context->window,0,0,width,height,AllPlanes,ZPixmap);

	auto res = XInitImage_fptr(&buffer.data->ximage);

	_kill("",1);

	_kill("Failed in init image\n",!res);

#endif

	auto display = internal_windowconnection.x11_display;

	buffer.data->pixmap =
	    XCreatePixmap_fptr(display, (Window)context->window, width, height,
			       context->data->x11_depth);

	XGCValues values = {};

	buffer.data->gc =
	    XCreateGC_fptr(display, buffer.data->pixmap, 0, &values);

	if (!wfptr_x11_xputimage) {
		wfptr_x11_xputimage =
		    LGetLibFunction(wwindowlib_handle, "XPutImage");
		wfptr_x11_xcopyarea =
		    LGetLibFunction(wwindowlib_handle, "XCopyArea");
	}

	buffer.pixels = (u32*)data;

	return buffer;
}

void WPresentBackBufferX11(WWindowContext* window, WBackBufferContext* buffer) {
	// MARK: continue from here

	// for drawing
	// copy the image to the pixmap

	auto display = internal_windowconnection.x11_display;

	XPutImage(display, buffer->data->pixmap, buffer->data->gc,
		  &buffer->data->ximage, 0, 0, 0, 0, buffer->width,
		  buffer->height);

	XCopyArea(display, buffer->data->pixmap, (Window)window->window,
		  buffer->data->gc, 0, 0, buffer->width, buffer->height, 0, 0);
}

_intern XVisualInfo* GetVisualInfoArray(Display* dpy, s32* _restrict count) {
	auto XGetVisualInfo_fptr =
	    (XVisualInfo * (*)(Display*, long, XVisualInfo*, s32*))
		LGetLibFunction(wwindowlib_handle, "XGetVisualInfo");

	XVisualInfo visual_array[] = {

	    {
		0,
		0,
		0,
		32,
		TrueColor,

		// Aren't used but pretty sure these are the only formats for
		// true color anyway
		0xFF0000,  // red mask
		0x00FF00,  // green mask
		0x0000FF,  // blue mask

		0,
		8,
	    },

	    {
		0,
		0,
		0,
		24,
		TrueColor,

		// Aren't used but pretty sure these are the only formats for
		// true color anyway
		0xFF0000,  // red mask
		0x00FF00,  // green mask
		0x0000FF,  // blue mask

		0,
		8,
	    },

	};

	XVisualInfo* ret = 0;

	for (u32 i = 0; i < _arraycount(visual_array); i++) {
		ret = XGetVisualInfo_fptr(
		    dpy,
		    VisualClassMask | VisualDepthMask | VisualRedMaskMask |
			VisualGreenMaskMask | VisualBlueMaskMask |
			VisualBitsPerRGBMask,
		    &visual_array[i], count);

		if (ret) {
#ifdef DEBUG
			printf("Chose visualinfo %d\n", i);
#endif
			break;
		}
	}

	return ret;
}

void InternalUnloadLibraryX11() {
	LUnloadLibrary(wwindowlib_handle);
	wwindowlib_handle = 0;
	loaded_platform = WPLATFORM_NONE;
}

void InternalX11DeinitOneTime() {
	auto disconnect_fptr =
	    (u32(*)(void*))LGetLibFunction(wwindowlib_handle, "XCloseDisplay");

	disconnect_fptr((void*)internal_windowconnection.x11_display);
}

b32 InternalX11InitOneTime() {

	if (!InternalLoadLibraryX11()) {
		return false;
	}

	auto XOpenDisplay_fptr = (Display * (*)(s8*))
	    LGetLibFunction(wwindowlib_handle, "XOpenDisplay");

	internal_windowconnection.x11_display = XOpenDisplay_fptr(0);
	auto display = internal_windowconnection.x11_display;

	if (!display) {
		InternalUnloadLibraryX11();
		return false;
	}

	impl_wkeycodetoascii = WKeyCodeToASCIIX11;
	impl_wwaitforevent = WWaitForWindowEventX11;
	impl_wsettitle = WSetTitleX11;
	impl_wcreatebackbuffer = WCreateBackBufferX11;
	impl_getwindowsize = WGetWindowSizeX11;
	impl_wpresentbackbuffer = WPresentBackBufferX11;
	impl_wdestroybackbuffer = WDestroyBackBufferX11;
	impl_wretireevent = WRetireEventX11;
	impl_wackresizeevent = WAckResizeEventX11;

	return true;
}

b32 InternalCreateX11Window(WWindowContext* context, const s8* title,
			    WCreateFlags flags, u32 x, u32 y, u32 width,
			    u32 height) {

	// get all the functions needed for init

	auto XCreateWindow_fptr =
	    (Window(*)(Display*, Window, s32, s32, u32, u32, u32, s32, u32,
		       Visual*, unsigned long,
		       XSetWindowAttributes*))LGetLibFunction(wwindowlib_handle,
							      "XCreateWindow");

	auto XWhitePixel_fptr = (unsigned long (*)(
	    Display*, s32))LGetLibFunction(wwindowlib_handle, "XWhitePixel");

	auto XRootWindow_fptr = (Window(*)(Display*, s32))LGetLibFunction(
	    wwindowlib_handle, "XRootWindow");

	auto XSelectInput_fptr =
	    (s32(*)(Display*, Window, long))LGetLibFunction(wwindowlib_handle,
							    "XSelectInput");

	auto XMapRaised_fptr = (s32(*)(Display*, Window))LGetLibFunction(
	    wwindowlib_handle, "XMapRaised");

	auto XInternAtom_fptr =
	    (Atom(*)(Display*, _Xconst char*, Bool))LGetLibFunction(
		wwindowlib_handle, "XInternAtom");

	auto XVisualIDFromVisual_fptr = (VisualID(*)(Visual*))LGetLibFunction(
	    wwindowlib_handle, "XVisualIDFromVisual");

	auto XFree_fptr =
	    (void (*)(void*))LGetLibFunction(wwindowlib_handle, "XFree");

	auto XDefaultScreen_fptr = (s32(*)(Display*))LGetLibFunction(
	    wwindowlib_handle, "XDefaultScreen");

	auto XCreateColormap_fptr =
	    (Colormap(*)(Display*, Window, Visual*, s32))LGetLibFunction(
		wwindowlib_handle, "XCreateColormap");

	context->data->type = _X11_WINDOW;
	auto display = internal_windowconnection.x11_display;

	_kill("failed to open display\n", !display);

	auto screen_no = XDefaultScreen_fptr(display);

#ifdef DEBUG
	printf("The default screen no is %d\n", screen_no);
#endif

	Visual* visual_ptr = 0;
	u32 depth = 0;

	{
		s32 info_count = 0;
		auto info_array = GetVisualInfoArray(display, &info_count);

#if 0

		printf("count %d\n",info_count);

		for(s32 i = 0; i < info_count; i++){

			auto info = info_array[i];

			printf("(%d %d):%d 0x%08x 0x%08x 0x%08x %d %d %d\n",(u32)info.visualid,info.screen,info.depth,(u32)info.red_mask,
					(u32)info.green_mask,(u32)info.blue_mask,info.bits_per_rgb,info.colormap_size,info.c_class);
		}

#endif

		_kill("no suitable window format\n", !info_count);

		auto vis = info_array[0];

		visual_ptr = vis.visual;
		depth = vis.depth;
		context->data->x11_visualid = vis.visualid;

		XFree_fptr(info_array);
	}

	XSetWindowAttributes frame_attrib = {};
	frame_attrib.background_pixel = XWhitePixel_fptr(display, screen_no);

	// MARK: handling visual
	context->data->x11_colormap =
	    XCreateColormap_fptr(display, XRootWindow_fptr(display, screen_no),
				 visual_ptr, AllocNone);

	frame_attrib.colormap = context->data->x11_colormap;

	frame_attrib.background_pixmap = None;
	frame_attrib.border_pixel = 0;
	//

#define borderwidth 0
	u32 mask = CWBackPixel | CWColormap | CWBackPixmap | CWBorderPixel;

	context->window = (void*)XCreateWindow_fptr(
	    display, XRootWindow_fptr(display, screen_no), x, y, width, height,
	    borderwidth, depth, InputOutput, visual_ptr, mask, &frame_attrib);

	context->data->x11_depth = depth;

	WSetTitle(context, title);

	XSelectInput_fptr(display, (Window)context->window,
			  ExposureMask | ButtonPressMask | ButtonReleaseMask |
			      KeyReleaseMask | KeyPressMask |
			      StructureNotifyMask | PointerMotionMask);

	XSizeHints hints = {};

	hints.flags = PPosition | PSize;
	hints.x = (s32)x;
	hints.y = (s32)y;
	hints.width = width;
	hints.height = height;

	if (flags & W_CREATE_NORESIZE) {
		hints.flags |= PMinSize | PMaxSize;
		hints.min_width = width;
		hints.min_height = height;
		hints.max_width = width;
		hints.max_height = height;
	}

	XSetWMNormalHints(display, (Window)context->window, &hints);

	// create exit atom - MARK:Idk if this handles the case where the atom
	// already exists
	auto atom_exit = XInternAtom_fptr(display, "WM_DELETE_WINDOW", false);

	if (atom_exit) {
		XSetWMProtocols(display, (Window)context->window, &atom_exit,
				1);
	}

	// Set window class

	auto XSetClassHint_fptr =
	    (void (*)(Display*, Window, XClassHint*))LGetLibFunction(
		wwindowlib_handle, "XSetClassHint");

	XClassHint hint = {(s8*)title, (s8*)title};

	XSetClassHint_fptr(display, (Window)context->window, &hint);

	XMapRaised_fptr(display, (Window)context->window);

	// MARK: we can set multiple with XSetWMProperties
	XFlush(display);

	return true;
}
