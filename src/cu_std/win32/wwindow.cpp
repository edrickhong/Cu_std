#include "wwindow.h"
#include "string.h"

#include "Windowsx.h"


_global WWindowEvent event_array[32];
_global u32 event_count = 0;

void _ainline PostEvent(WWindowEvent event){
    
    _kill("msg stack overflow\n",event_count > _arraycount(event_array));
    
    event_array[event_count] = event;
    event_count++;
}

#define _WIN32_DOWN_BIT (1 << 30)

LRESULT CALLBACK WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){

	//TODO: we should check if hwnd can be encapsulated in 32 bits
    
    LRESULT result = 0;
    
    switch (uMsg){
        
        case WM_DESTROY:
        {
            PostEvent({W_EVENT_CLOSE,(u64)hwnd});
            PostQuitMessage(0);
        } break;
        
        case WM_CLOSE:
        {
            PostEvent({W_EVENT_CLOSE,(u64)hwnd});
            PostQuitMessage(0);
        } break;
        
        case WM_ACTIVATEAPP:{
            PostEvent({(WEventType)uMsg,(u64)hwnd});
        }break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        {
            auto vcode = (u32)wParam;
            
            if(!(_WIN32_DOWN_BIT & lParam)){
                WWindowEvent event = {};
                event.type = W_EVENT_KBEVENT_KEYDOWN;
		event.window = (u64)hwnd;
                event.keyboard_event.keycode = vcode;
                PostEvent(event);
            }
            
        }break;
        case WM_KEYUP:{
            
            auto vcode = (u32)wParam;
            
            if((_WIN32_DOWN_BIT & lParam)){
                WWindowEvent event = {};
                event.type = W_EVENT_KBEVENT_KEYUP;
		event.window = (u64)hwnd;
                event.keyboard_event.keycode = vcode;
                PostEvent(event);
            }
            
        }break;
        
        case WM_MOUSEMOVE:{
            WWindowEvent event = {};
            event.type = W_EVENT_MSEVENT_MOVE;
	    event.window = (u64)hwnd;
            event.mouse_event.x = GET_X_LPARAM(lParam);
            event.mouse_event.y = GET_Y_LPARAM(lParam);
            PostEvent(event);
        }break;
        
        case WM_LBUTTONDOWN:{
            WWindowEvent event = {};
            event.type = W_EVENT_MSEVENT_DOWN;
	    event.window = (u64)hwnd;
            event.mouse_event.keycode = MOUSEBUTTON_LEFT;
            PostEvent(event);
        }break;
        
        case WM_LBUTTONUP:{
            WWindowEvent event = {};
            event.type = W_EVENT_MSEVENT_UP;
	    event.window = (u64)hwnd;
            event.mouse_event.keycode = MOUSEBUTTON_LEFT;
            PostEvent(event);
        }break;
        
        case WM_RBUTTONDOWN: {
            WWindowEvent event = {};
            event.type = W_EVENT_MSEVENT_DOWN;
	    event.window = (u64)hwnd;
            event.mouse_event.keycode = MOUSEBUTTON_RIGHT;
            PostEvent(event);
        }break;
        
        case WM_RBUTTONUP: {
            WWindowEvent event = {};
            event.type = W_EVENT_MSEVENT_UP;
	    event.window = (u64)hwnd;
            event.mouse_event.keycode = MOUSEBUTTON_RIGHT;
            PostEvent(event);
        }break;
        
        case WM_SIZE:{
            
            WWindowEvent event = {};
            event.type = W_EVENT_RESIZE;
	    event.window = (u64)hwnd;
            
            union U32_PACKED{
                u64 u;
                
                struct{
                    u16 w;
                    u16 h;
                };
            };
            
            U32_PACKED p = {(u64)lParam};
            
            event.width = p.w;
            event.height = p.h;
            
            PostEvent(event);
            
        }break;
        
        default:
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    
    return result;
}

void* WGetWindowConnection(){

	void* connection = 0;

    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 0,
                      (HMODULE*)&connection);

    return connection;
}

WWindowContext WCreateWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,u32 width,
                             u32 height){
    
    WWindowContext context = {};

    HMODULE connection = WGetWindowConnection();
    
    
    WNDCLASSEX wndclass = {};
    
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = flags;
    wndclass.lpfnWndProc = WindowCallback;
    wndclass.hInstance = (HMODULE)connection;
    wndclass.lpszClassName = title;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    auto res = RegisterClassEx(&wndclass);
    
    auto style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    
    if(W_CREATE_NORESIZE & flags){
        style ^= (WS_MAXIMIZEBOX | WS_THICKFRAME);
    }
    
    
    context.window =
        (void*)CreateWindow(wndclass.lpszClassName,title,style, x, y,
                            width, height, 0, 0, (HMODULE)connection, 0);
    
    _kill("Failed to register WNDCLASS", !res);
    _kill("Unable to create window", !(context.window));
    
    return context;
}

u32 WWaitForWindowEvent(WWindowEvent* event){
    
    MSG msg;
    
    auto ret = event_count;
   
   //NOTE: we are now looking at all messages instead of per window. we will remove the first param in this funtion soon 
    while(PeekMessage(&msg,0,0,0,PM_REMOVE | PM_NOYIELD) > 0){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    if(ret){
        *event = event_array[event_count - 1];
    }
    
    return ret;
}

void WAckResizeEvent(WWindowEvent* event){
	event->ack_resize = 1;
}
void WIgnoreResizeEvent(WWindowEvent* event){
	event->ack_resize = 2;
}

void WRetireEvent(WWindowEvent* event){
#ifdef DEBUG
	_kill("Resize events must be explicitly handled WIgnoreResizeEvent WAckResizeEvent\n",!event->ack_resize && event->type == W_EVENT_RESIZE);
#endif
        event_count --;
}

void WSetIcon(WWindowContext windowcontext,void* icondata,u32 width,u32 height){}

void WSetTitle(WWindowContext windowcontext,const s8* title_string){}


s8 WKeyCodeToASCII(u32 keycode){
    
    auto scancode = MapVirtualKey(keycode,MAPVK_VSC_TO_VK);
    
    WORD ascii_char[2];
    BYTE keyboardstate[256];
    
    auto error = GetKeyboardState(&keyboardstate[0]);
    
    _kill("failed to get keyboard state\n",!error);
    
    ToAscii(keycode,scancode,&keyboardstate[0],&ascii_char[0],0);
    
    return ascii_char[0];
}

WWindowContext WCreateVulkanWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,u32 width,
                                   u32 height){
    
    return WCreateWindow(title,flags,x,y,width,height);
}

void _ainline InternalGetWindowSize(WWindowContext* window,u32* w,u32* h){
    
    RECT rect = {};
    GetWindowRect((HWND)window->window,&rect);
    
    *w = rect.right - rect.left;
    *h = rect.bottom - rect.top;
}

struct InternalBackBufferData{
    HDC devicecontext;
    BITMAPINFO info;
};


WBackBufferContext WCreateBackBuffer(WWindowContext* windowcontext) {
    
	WBackBufferContext buffer = {};
    
    u32 width = 0;
    u32 height= 0;
    
    InternalGetWindowSize(windowcontext,&width,&height);
    
    u32 size = width * height * 4;
    
    buffer.pixels = (u32*)alloc(size);
    buffer.width = width;
    buffer.height = height;
    
    buffer.data = 
        (InternalBackBufferData*)alloc(sizeof(InternalBackBufferData));
    
    buffer.data->info = {
        {
            sizeof(BITMAPINFOHEADER),
            (s32)width,
            (s32)-height,
            1,
            32,
            BI_RGB
        }
    };
    
    buffer.data->devicecontext = GetDC((HWND)windowcontext->window);
    
    //MARK: maybe set the pixel format?? (Seems like the default is already ARGB)
    
    _kill("failed to get a hdc\n",!buffer.data->devicecontext);
    
	return buffer;
}

void WPresentBackBuffer(WWindowContext* windowcontext, WBackBufferContext* buffer) {
    
    auto res = StretchDIBits(
        buffer->data->devicecontext,
        0,
        0,
        buffer->width,
        buffer->height,
        0,
        0,
        buffer->width,
        buffer->height,
        (void*)buffer->pixels,
        &buffer->data->info,
        DIB_RGB_COLORS,
        SRCCOPY);
    
    _kill("failed to present backbuffer\n",!res);
    
}
