#pragma once

#include "ttype.h"
#include "vvulkan.h"
#include "mmath.h"
#include "debugtimer.h"

struct KeyboardState;
struct MouseState;


struct GUIVertex{
    f32 pos[3];
    f32 uv[2];
    f32 color[4];
};

struct GUIBoundingRect{
    u16 x;
    u16 y;
    u16 width;
    u16 height;
};

typedef Vec2 GUIVec2;
typedef Vec3 GUIVec3;

struct GUIDim2{
    f32 w;
    f32 h;
};

struct GUIFont{
    VTextureContext texture;
    u16 width;
    u16 height;
    VkDescriptorSet descset;
};

enum GUIRenderMode{
    GUI_RENDER_SOLID = 0,
    GUI_RENDER_LINE = 1,
    GUI_RENDER_TEXT = 2,
};

enum GUICameraMode{
    GUI_CAMERA_NONE = 0,
    GUI_CAMERA_PERSPECTIVE = 1,
};

GUIFont GUICreateFontFromFile(const s8* filepath,VkCommandBuffer cmdbuffer,
                              VDeviceContext* vdevice,VkQueue queue);

void GUIInit(VDeviceContext* vdevice,VSwapchainContext* swap,
             VkRenderPass renderpass,VkQueue queue,VkCommandBuffer cmdbuffer,VkPipelineCache cache = 0,
             u32 vertexbinding_no = 0,
             GUIFont* fonthandle = 0);

void GUIUpdate(VSwapchainContext* swapchain,KeyboardState* keyboardstate,s8* keyboard_ascii_buffer,u32 keyboard_ascii_count,
               MouseState* mousestate,Mat4 view,Mat4 proj);

void GUIDraw(VkCommandBuffer cmdbuffer);

void GUISetRenderMode(GUIRenderMode rendermode);

void GUISetCameraMode(GUICameraMode cameramode);

void GUISetBackColor(Color4 color);
void GUISetFrontColor(Color4 color);
void GUISetTitleColor(Color4 color);
void GUISetTextColor(Color4 color);

enum GUIWindowBehavoir{
    GUIWINDOW_NONE = 0,
    GUIWINDOW_NORESIZE = 1,
    GUIWINDOW_NOMOVE = 2,
};

void GUIBegin(const s8* title = 0,GUIVec2* pos = 0,GUIDim2* dim = 0);
void GUIEnd();

void GUIBeginWindow(const s8* title,GUIVec2* pos = 0,GUIDim2* dim = 0);

void GUIString(const s8* string);

b32 GUITextBox(const s8* label,const s8* buffer,b32 fill_w = true,GUIDim2 dim = {0.5f,0.5f});

b32 GUITextField(const s8* label,const s8* buffer,b32 fill_w = true,f32 w = 0.5f);

b32 GUIButton(const s8* title);

b32 GUIComboBox(const s8* label,const s8** options_array,u32 options_count,u32* index,
                b32 fill_w = true);

void GUISlider();

void GUI3DTranslate(f32* x,f32* y,f32* z);
void GUI3DScale(f32* x);
void GUI3DRotate(f32* x,f32* y,f32* z);

b32 GUIHistogram(const s8* label_x,const s8* label_y,GUIVec2* data_array,u32 data_count,
                 u32* out_entry_index,f32* max = 0,GUIDim2 dim = {0.5f,0.5f},u32* highlight_index = 0);



b32 GUIProfileView(const s8* profilename,const DebugTable* table,
                   GUIDim2 dim = {0.5f,0.5f});

b32 GUIIsElementActive(const s8* element_name);

b32 GUIIsAnyElementActive();

void GUILineGraph();

#define GUIDEFAULT_X -0.7f
#define GUIDEFAULT_Y 0.5f

#define GUIDEFAULT_W 0.25f
#define GUIDEFAULT_H 0.25f

b32 GUITranslateGizmo(GUIVec3* world_pos);

b32 GUIScaleGizmo(GUIVec3 world_pos,f32* scale);

b32 GUIRotationGizmo(GUIVec3 world_pos,Quat* rot);

void GUIDrawPosMarker(GUIVec3 pos,Color4 color = White, b32 is_perspective = true);

GUIVec2 GUIMouseCoordToScreenCoord();


struct GUIContext;

GUIContext* GetGUIContext();
void SetGUIContext(GUIContext* context);


void GUIGenFontFile(const s8* filepath,const s8* writepath,f32 fontsize);

void GUIDebugGetCurrentHolder();

void GUIDrawAxisSphere(Vec3 obj_w,f32 radius,Color4 x = White,Color4 y = White,Color4 z = White);


void GUIGetVertexBufferAndOffset(VBufferContext** vert_buffercontext);

void GUIDrawLine3D(GUIVec3 a,GUIVec3 b,Color4 color = White);

//TODO: implement a color picker (https://en.wikipedia.org/wiki/HSL_and_HSV)
