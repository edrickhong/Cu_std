#pragma once

/*
TODO:
*/

#ifdef _WIN32

#define VK_USE_PLATFORM_WIN32_KHR

#else

#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR

#endif

#ifdef DEBUG
#include "aallocator.h"
#endif


#include "vulkan/vulkan.h"
#include "aanimation.h"

#include "ccolor.h"

#include "tthreadx.h"

struct WWindowContext;

#include "vvulkan_dyn.h"

#ifdef DEBUG
#define _vk_inject_cmdbuffers 1
#else
#define _vk_inject_cmdbuffers 0
#endif

#if 0

#define _vthreaddump(string, ...) _dprint(string, __VA_ARGS__)

#else

#define _vthreaddump(string, ...)

#endif


//NOTE:Those with the Memory label can be mapped
struct VImageMemoryContext{
    VkImage image;
    
    u32 mapid;
    u32 size;
};

struct VImageContext{
    VkImage image;
    VkImageView view;
};

struct VTextureContext : VImageContext{
    VkSampler sampler;
};



struct VSwapchainContext{
    
    struct DepthStencil : VImageContext{
        VkFormat format;
    };
    
    
    struct PresentImageResource{
        VkImage image;
        VkImageView view;
        VkFramebuffer framebuffer;
    };
    
    struct InternalData{
        DepthStencil depthstencil;
        VkSurfaceKHR surface;
        VkFormat format;
    };
    
    u16 image_index;
    u16 image_count;
    VkSwapchainKHR swap;
    
    
    InternalData* internal;
    
    u16 width;
    u16 height;
    PresentImageResource* presentresource_array;
};


struct VBufferContext{
    VkBuffer buffer;
    u32 size;
    
    //unique attrib according to buffer type. binding no on vertex buffer/instance,
    //count on index buffer
    
    union{
        
        struct{
            u16 bind_no;//NOTE: vertex and instance buffers only
            u16 inst_count;//NOTE: instance buffers only
        };
        
        //NOTE: index buffers only we will set the top most bit if 32bit, else it is 16bit
        u32 ind_count;
        
        
        //NOTE: mapped buffers only (eg uniform/storage buffers)
        u32 mapid; 
    };
    
    
};

#define VCREATEQUEUEBIT_ALL (u32)-1
#define VCREATEQUEUEBIT_ROOT 1
#define VCREATEQUEUEBIT_COMPUTE 2
#define VCREATEQUEUEBIT_TRANSFER 4

enum VQueueType{
    VQUEUETYPE_ROOT = 0,
    VQUEUETYPE_COMPUTE = 1,
    VQUEUETYPE_TRANSFER = 2
};


struct VDeviceContext{
    
    struct PhysDeviceInfo{
        
        VkPhysicalDevice physicaldevice_array[8];
        VkPhysicalDeviceMemoryProperties* memoryproperties;
        u32 physicaldevice_count;
    };
    PhysDeviceInfo* phys_info;
    VkDevice device;
};

struct VModel{
    VBufferContext vertexbuffercontext;
    VBufferContext indexbuffercontext;
};

//can contain many meshes with animations
struct VSkeletalModel{
    VBufferContext vertexbuffercontext;
    VBufferContext indexbuffercontext;
    
    ALinearBone* rootbone;
    AAnimationSet* animationset_array;
    
    u16 bone_count;
    u16 animationset_count;
};

struct VDescriptorPoolSpec{
    VkDescriptorPoolSize container[16];
    u32 count = 0;
    u32 desc_count = 0;
};

struct VDescriptorBindingSpec{
    VkDescriptorSetLayoutBinding container[20];
    u32 count = 0;
};

struct VDescriptorWriteSpec{
    VkWriteDescriptorSet container[20];
    u32 count = 0;
};

struct VDescriptorCopySpec{
    VkCopyDescriptorSet container[20];
    u32 count = 0;
};

struct VAttachmentSpec{
    VkAttachmentDescription des_container[10];
    VkAttachmentReference ref_container[10];
    u32 count = 0;
};

struct VSubpassDescriptionSpec{
    VkSubpassDescription container[10];
    u32 count = 0;
};

struct VSubpassDependencySpec{
    VkSubpassDependency container[20];
    u32 count = 0;
};

struct VSubmitBatch{
    VkSubmitInfo container[10];
    u32 count;
};


#define _write_block_flags (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
#define _readwrite_block_flags (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)

#define _direct_block_flags (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)

//NOTE: there is a memory flag to only allow device access (protected). should we use this??

enum VMemoryBlockHintFlag{
    VBLOCK_DEVICE, //device access only. device read/write-able
    VBLOCK_WRITE = _write_block_flags, //host writeable device readable
    VBLOCK_READWRITE = _readwrite_block_flags, //host read/write-able device read/write-able
    VBLOCK_DIRECT = _direct_block_flags, //host read/write-able device read/write-able
};


void VDescPushBackPoolSpec(VDescriptorPoolSpec* poolspec,VkDescriptorType type,u32 count);

void VDescPushBackBindingSpec(VDescriptorBindingSpec* bindingspec,
                              VkDescriptorType type,u32 count,VkShaderStageFlags stage_flags,
                              VkSampler* immutable_samplers = 0);

void VDescPushBackWriteSpecImage(VDescriptorWriteSpec* spec,VkDescriptorSet dst_set,
                                 u32 dst_binding,u32 dst_startindex,u32 count,VkDescriptorType type,
                                 VkDescriptorImageInfo* imageinfo_array);

void VDescPushBackWriteSpecBuffer(VDescriptorWriteSpec* spec,VkDescriptorSet dst_set,
                                  u32 dst_binding,u32 dst_startindex,u32 count,VkDescriptorType type,
                                  VkDescriptorBufferInfo* bufferinfo_array);

void VDescPushBackWriteSpecView(VDescriptorWriteSpec* spec,VkDescriptorSet dst_set,
                                u32 dst_binding,u32 dst_startindex,u32 count,VkDescriptorType type,
                                VkBufferView* bufferview_array);

void VPushBackAttachmentSpec(VAttachmentSpec* spec,
                             VkAttachmentDescriptionFlags flags,VkFormat format,
                             VkSampleCountFlagBits samples,VkAttachmentLoadOp loadop,
                             VkAttachmentStoreOp storeop,VkAttachmentLoadOp stencil_loadop,
                             VkAttachmentStoreOp stencil_storeop,VkImageLayout initial,VkImageLayout final);

void VPushBackSubpassDescriptionSpec(VSubpassDescriptionSpec* spec,
                                     VkSubpassDescriptionFlags flags,VkPipelineBindPoint bindpoint,
                                     u32 inputattachment_count,
                                     const VkAttachmentReference* inputattachment_array,
                                     u32 colorattachment_count,
                                     const VkAttachmentReference* colorattachment_array,
                                     const VkAttachmentReference* resolveattachment_array,
                                     const VkAttachmentReference* depthstencilattachment_array,
                                     u32 preserveattachment_count,
                                     const u32* preserveattachment_array);

void VPushBackSubpassDependencySpec(VSubpassDependencySpec* spec,
                                    u32 srcsubpass_index,u32 dstsubpass_index,
                                    VkPipelineStageFlags src_stagemask,
                                    VkPipelineStageFlags dst_stagemask,
                                    VkAccessFlags src_accessmask,VkAccessFlags dst_accessmask,
                                    VkDependencyFlags dependencyflags);

enum V_Instance_Flags{
    V_INSTANCE_FLAGS_NONE = 0,
    V_INSTANCE_FLAGS_SINGLE_VKDEVICE = 1,
    V_INSTANCE_FLAGS_API_VERSION_OPTIONAL = 2,
};

enum V_VDeviceContext_Flags{
    V_VDEVICECONTEXT_FLAGS_NONE = 0,
    V_VDEVICECONTEXT_FLAGS_ENABLE_RENDER_TO_WINDOW = 1,
};

u32 VCreateInstance(const s8* applicationname_string,b32 validation_enable,u32 api_version,WWindowContext* window = 0,u32 v_inst_flags = V_INSTANCE_FLAGS_NONE);

VDeviceContext VCreateDeviceContext(VkPhysicalDevice* physdevice_array = 0,u32 physdevice_count = 1,u32 vdevice_flags = V_VDEVICECONTEXT_FLAGS_ENABLE_RENDER_TO_WINDOW,
                                    u32 createqueue_bits = VCREATEQUEUEBIT_ALL);

VkQueue VGetQueue(const VDeviceContext* _in_ vdevice,VQueueType type);

u32 VGetQueueFamilyIndex(VQueueType type);

enum VPresentSyncType{
    VSYNC_NONE = VK_PRESENT_MODE_IMMEDIATE_KHR,
    VSYNC_NORMAL = VK_PRESENT_MODE_FIFO_KHR,
    VSYNC_LAZY = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
    VSYNC_FAST = VK_PRESENT_MODE_MAILBOX_KHR,
    VSYNC_CHOOSE_BEST = -1,
};


//TODO: we should'nt take in a width and height, but instead query for it from the window
VSwapchainContext VCreateSwapchainContext(const VDeviceContext* _in_ vdevice,
                                          u32 swapcount,
                                          WWindowContext* windowcontext,
                                          VPresentSyncType sync_type = VSYNC_NONE,
                                          VSwapchainContext* oldswapchain = 0);

struct VPhysicalDevice_Index{
    VkPhysicalDevice physicaldevice;
    u32 index;
};

struct VPhysicalDeviceGroups{};

void VEnumeratePhysicalDevices(VkPhysicalDevice * array,u32* count,WWindowContext* window = 0);

void VEnumeratePhysicalDeviceGroups(VPhysicalDeviceGroups* array,u32* count,WWindowContext* window = 0);

VkFence VCreateFence(VDeviceContext* _in_ vdevice,VkFenceCreateFlags flags);

VkDescriptorPool VCreateDescriptorPool(VDeviceContext* _in_ vdevice,
                                       VDescriptorPoolSpec poolspec,u32 flags,u32 max_sets);

VkDescriptorSetLayout VCreateDescriptorSetLayout(
const  VDeviceContext* _restrict vdevice,
VDescriptorBindingSpec bindingspec);

void VAllocDescriptorSetArray(const  VDeviceContext* _restrict vdevice,
                              VkDescriptorPool pool,u32 count,VkDescriptorSetLayout* layout_array,
                              VkDescriptorSet* set_array);

void _ainline VUpdateDescriptorSets(const  VDeviceContext* _restrict vdevice,
                                    VDescriptorWriteSpec writespec,VDescriptorCopySpec copyspec = {}){
    
    vkUpdateDescriptorSets(vdevice->device,writespec.count,writespec.container,
                           copyspec.count,copyspec.container);
}

VkRenderPass VCreateRenderPass(const  VDeviceContext* _restrict vdevice,
                               VkRenderPassCreateFlags flags,VAttachmentSpec attachmentspec,
                               VSubpassDescriptionSpec subpassdescspec,
                               VSubpassDependencySpec subpassdepspec);


VkPipelineLayout VCreatePipelineLayout(const  VDeviceContext* _restrict vdevice,
                                       VkDescriptorSetLayout* descriptorsetlayout_array,
                                       u32 descriptorsetlayout_count,
                                       VkPushConstantRange* pushconstrange_array,
                                       u32 pushconstrange_count);

void VDestroyPipeline(const  VDeviceContext* _restrict vdevice,VkPipeline pipeline);

void VSubmitCommandBuffer(VkQueue queue,VkCommandBuffer commandbuffer,
                          VkSemaphore* wait_semaphore = 0,u32 wait_count = 0,
                          VkSemaphore* signal_semaphore = 0,u32 signal_count = 0,
                          VkPipelineStageFlags* wait_dstmask = 0,VkFence fence = 0);

void VSubmitCommandBuffer(VkQueue queue,VkCommandBuffer commandbuffer,
                          VkSemaphore wait_semaphore,
                          VkSemaphore signal_semaphore,VkPipelineStageFlags wait_dstmask,
                          VkFence fence = 0);

void VSubmitCommandBufferArray(VkQueue queue,VkCommandBuffer* commandbuffer,
                               u32 buffer_count,VkSemaphore* wait_semaphore = 0,u32 wait_count = 0,
                               VkSemaphore* signal_semaphore = 0,u32 signal_count = 0,
                               VkPipelineStageFlags* wait_dstmask = 0,VkFence fence = 0);

void VSubmitCommandBufferArray(VkQueue queue,VkCommandBuffer* commandbuffer,
                               u32 buffer_count,VkSemaphore wait_semaphore,
                               VkSemaphore signal_semaphore,VkPipelineStageFlags wait_dstmask,
                               VkFence fence);

void VSubmitCommandBufferBatch(VkQueue queue,VSubmitBatch batch,VkFence fence);


VBufferContext VCreateStaticVertexBuffer(const  VDeviceContext* _restrict vdevice,
                                         u32 data_size,
                                         VkDeviceMemory memory,
                                         VkDeviceSize offset,
                                         u32 bindingno);

VBufferContext VCreateStaticIndexBuffer(const  VDeviceContext* _restrict vdevice,
                                        VkDeviceMemory memory,
                                        VkDeviceSize offset,
                                        ptrsize data_size,u32 ind_size = sizeof(u32));

VBufferContext VCreateStaticVertexBuffer(const  VDeviceContext* _restrict vdevice,
                                         ptrsize data_size,u32 bindingno,VMemoryBlockHintFlag flag = VBLOCK_DEVICE);

VBufferContext VCreateStaticIndexBuffer(const  VDeviceContext* _restrict vdevice,
                                        ptrsize size,u32 ind_size = sizeof(u32),
                                        VMemoryBlockHintFlag flag = VBLOCK_DEVICE);


VImageContext VCreateColorImage(const  VDeviceContext* _restrict vdevice,
                                u32 width,u32 height,u32 usage,
                                VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                                VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);


VImageMemoryContext VCreateColorImageMemory(const  VDeviceContext* _restrict vdevice,
                                            u32 width,u32 height,u32 usage,VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,VMemoryBlockHintFlag flag = VBLOCK_WRITE);


void VQueuePresentArray(VkQueue queue,u32* imageindex_array,
                        VkSwapchainKHR* swapchain_array,
                        ptrsize swapchain_count,
                        VkSemaphore* _restrict waitsemaphore_array,
                        ptrsize waitsemaphore_count,
                        VkResult* result_array);

void VQueuePresent(VkQueue queue,u32 image_index,VkSwapchainKHR swapchain,
                   VkSemaphore waitsemaphore);

void inline VDrawIndex(VBufferContext vertex_buffer,VBufferContext index_buffer,
                       VkCommandBuffer commandbuffer,VkDeviceSize offset){
    
    vkCmdBindVertexBuffers(commandbuffer,vertex_buffer.bind_no,1,
                           &vertex_buffer.buffer,
                           &offset);
    
    vkCmdBindIndexBuffer(commandbuffer,index_buffer.buffer,
                         0,VK_INDEX_TYPE_UINT32);
    
    vkCmdDrawIndexed(commandbuffer,index_buffer.ind_count,1,0,0,0);
    
}

void inline VDrawIndexInstanced(VBufferContext vertex_buffer,VBufferContext index_buffer,
                                VBufferContext instance_buffer,u32 instance_count,
                                VkCommandBuffer commandbuffer,VkDeviceSize offset){
    
    vkCmdBindVertexBuffers(commandbuffer,vertex_buffer.bind_no,1,
                           &vertex_buffer.buffer,
                           &offset);
    
    vkCmdBindVertexBuffers(commandbuffer,instance_buffer.bind_no,1,
                           &instance_buffer.buffer,
                           &offset);
    
    vkCmdBindIndexBuffer(commandbuffer,index_buffer.buffer,
                         0,VK_INDEX_TYPE_UINT32);
    
    vkCmdDrawIndexed(commandbuffer,index_buffer.ind_count,instance_count,0,0,0);
    
}


VkFramebuffer VCreateFrameBuffer(const  VDeviceContext* _restrict vdevice,
                                 VkFramebufferCreateFlags flags,VkRenderPass renderpass,
                                 VkImageView* attachment_array,u32 attachment_count,u32 width,u32 height,
                                 u32 layers);

void VStartCommandBuffer(VkCommandBuffer cmdbuffer,
                         VkCommandBufferUsageFlags flags = 0);

void VStartCommandBuffer(VkCommandBuffer cmdbuffer,
                         VkCommandBufferUsageFlags flags,
                         VkRenderPass renderpass,u32 subpass,
                         VkFramebuffer framebuffer,
                         VkBool32 occlusion_enable,
                         VkQueryControlFlags queryflags,
                         VkQueryPipelineStatisticFlags querypipelineflags);

void VEndCommandBuffer(VkCommandBuffer cmdbuffer);

void VAllocateCommandBufferArray(const  VDeviceContext* _restrict vdevice,VkCommandPool pool,
                                 VkCommandBufferLevel level,
                                 VkCommandBuffer* _restrict commandbuffer_array,
                                 u32 commandbuffer_count);

VkCommandBuffer VAllocateCommandBuffer(const  VDeviceContext* _restrict vdevice,
                                       VkCommandPool pool,VkCommandBufferLevel level);

VkCommandPool VCreateCommandPool(const  VDeviceContext* _restrict vdevice,
                                 VkCommandPoolCreateFlags flags,u32 familyindex);

void VStartRenderpass(VkCommandBuffer commandbuffer,VkSubpassContents contents,
                      VkRenderPass renderpass,VkFramebuffer framebuffer,VkRect2D renderarea,
                      VkClearValue* clearvalue_array,u32 clearvalue_count);

void VStartRenderpass(VkCommandBuffer commandbuffer,VkSubpassContents contents,
                      VkRenderPass renderpass,VkFramebuffer framebuffer,VkRect2D renderarea,
                      VkClearValue clearvalue);

void VEndRenderPass(VkCommandBuffer commandbuffer);

VkSemaphore VCreateSemaphore(const  VDeviceContext* _restrict vdevice);


void VSetDriverAllocator(VkAllocationCallbacks allocator);
void VSetDeviceAllocator(VkDeviceMemory (*allocator)(VkDevice,VkDeviceSize,u32,
                                                     VkAllocationCallbacks*));


struct _cachealign CacheAlignedCommandbuffer{
    VkCommandBuffer cmdbuffer;
};

struct VThreadCommandbufferList{
    CacheAlignedCommandbuffer* container;
    volatile u32 count;
};

void _ainline VPushThreadCommandbufferList(VThreadCommandbufferList* list,
                                           VkCommandBuffer cmdbuffer){
    
    u32 index = TGetEntryIndex(&list->count);
    
    _kill("submitted null cmdbuffer\n",!cmdbuffer);
    
    list->container[index].cmdbuffer = cmdbuffer;
    
    _vthreaddump("submit %p index %d\n",(void*)cmdbuffer,index);
    
    
}



void VDestroyBuffer(const  VDeviceContext* _restrict vdevice,VkBuffer buffer);

void VDestroyBufferContext(const  VDeviceContext* _restrict vdevice,
                           VBufferContext buffer);

void VFreeMemory(const  VDeviceContext* _restrict vdevice,VkDeviceMemory memory);


VkDeviceMemory  
VRawDeviceAlloc(VkDevice device,VkDeviceSize alloc_size,u32 memorytype_index,void* next = 0);

u32 VGetMemoryTypeIndex(VkPhysicalDeviceMemoryProperties properties,
                        u32 typebits,u32 flags);

VkShaderModule VCreateShaderModule(VkDevice device,void* data,
                                   ptrsize size,
                                   VkShaderModuleCreateFlags flags = 0);

enum VFilter{
    VFILTER_NONE,
    VFILTER_BILINEAR,
    VFILTER_TRILINEAR,
    VFILTER_ANISO,
};

VkSampler VCreateSampler(u32 filtering);


struct VComputePipelineSpec{
    
    void* shader_data;
    u32 shader_size;
    VkSpecializationInfo shader_specialization;
    
    VkPipelineLayout layout;
    VkPipelineCreateFlags flags;//controls if pipeline has parent for now
    VkPipeline parent_pipeline;
    s32 parentpipeline_index;
};



void VSetComputePipelineSpecShader(VComputePipelineSpec* spec,void* shader_data,
                                   u32 shader_size,VkSpecializationInfo specialization = {});

void VGenerateComputePipelineSpec(VComputePipelineSpec* spec,VkPipelineLayout layout,
                                  VkPipelineCreateFlags flags = 0,VkPipeline parent_pipeline = 0,
                                  s32 parentpipeline_index = -1);

void VCreateComputePipelineArray(const  VDeviceContext* _restrict vdevice,
                                 VkPipelineCache cache,VComputePipelineSpec* spec_array,u32 spec_count,
                                 VkPipeline* pipeline_array);


VkDescriptorBufferInfo _ainline VGetBufferInfo(const VBufferContext* buffer,
                                               VkDeviceSize offset = 0,
                                               VkDeviceSize range = VK_WHOLE_SIZE){
    
    _kill("Offset too large\n",(offset + range) > buffer->size);
    
    return {buffer->buffer,offset,range};
}


VkBuffer VRawCreateBuffer(const  VDeviceContext* _restrict vdevice,
                          VkBufferCreateFlags flags,
                          VkDeviceSize size,VkBufferUsageFlags usage,
                          VkSharingMode sharingmode = VK_SHARING_MODE_EXCLUSIVE,
                          u32* queuefamilyindex_array = 0 ,
                          ptrsize queuefamilyindex_count = 0);



u32 _ainline VFormatHash(VkFormat* format_array,u32 count){
    
    u32 hash = 0;
    
    for(u32 i = 0; i < count; i++){
        hash += ((format_array[i] * (i + 1)) ^ hash) * 31;
    }
    
    return hash;
}





//

struct VShaderObj{
    
#ifdef DEBUG
    
    u64 vert_hash;
    
#endif
    
    u8 vert_desc_count = 0;
    u8 vert_attrib_count = 0;
    u8 descset_count;
    u8 range_count = 0;
    u32 shader_count = 0;
    u32 range_hash_array[16];
    u32 spv_size_array[8];
    
    VkVertexInputBindingDescription vert_desc_array[4];
    VkVertexInputAttributeDescription vert_attrib_array[16];
    u32 vert_attrib_size_array[16];
    
    
    struct DescSetElement{
        VkDescriptorType type;
        u32 binding_no;
        u32 array_count;
    };
    
    struct DescSetEntry{
        u32 shader_stage;
        u32 set_no;
        DescSetElement element_array[32];
        u32 element_count;
        
    };
    
    DescSetEntry descset_array[16];
    VkPushConstantRange range_array[16];
    
    void* shader_data_array[8];
    VkShaderStageFlagBits shaderstage_array[8];
    VkSpecializationInfo spec_array[8];
    
};

struct VGraphicsPipelineSpecObj{
    
    VkPipelineVertexInputStateCreateInfo vertexinput;
    VkPipelineInputAssemblyStateCreateInfo assembly;
    VkPipelineRasterizationStateCreateInfo raster;
    
    
    VkPipelineViewportStateCreateInfo viewport;
    VkPipelineMultisampleStateCreateInfo multisample;
    VkPipelineDepthStencilStateCreateInfo depthstencil;
    VkPipelineColorBlendStateCreateInfo colorblendstate;
    
    //everything here is optional
    VkPipelineTessellationStateCreateInfo tessalationstate;
    VkPipelineDynamicStateCreateInfo dynamicstate;
    
    VkPipelineCreateFlags flags;
    
    VkPipelineLayout layout;
    VkRenderPass renderpass;
    u32 subpass_index;
    
    VkPipeline parent_pipeline;
    s32 parentpipeline_index;
    
    VkPipelineColorBlendAttachmentState colorattachment_array[16] = {};
    
    VkVertexInputBindingDescription vert_desc_array[4];
    VkVertexInputAttributeDescription vert_attrib_array[16];
    
    VkDynamicState dynamic_array[16];
    
    VkViewport viewport_array[8];
    VkRect2D scissor_array[8];
    
    VkPipelineShaderStageCreateInfo shaderinfo_array[8];
    VkSpecializationInfo spec_array[8];
    VkShaderModule shadermodule_array[8];
    u32 shadermodule_count = 0;
    
    VkSampleMask* samplemask = 0;
};

u32 VGetDescriptorSetLayoutHash(VShaderObj* obj,u32 descset_no);

VGraphicsPipelineSpecObj VMakeGraphicsPipelineSpecObj(const  VDeviceContext* vdevice,VShaderObj* obj,VkPipelineLayout layout,VkRenderPass renderpass,u32 subpass_index = 0,VSwapchainContext* swap = 0,u32 colorattachment_count = 1,VkPipelineCreateFlags flags = 0,
                                                      VkPipeline parent_pipeline = 0,s32 parentpipeline_index = -1);

void VCreateGraphicsPipelineArray(const  VDeviceContext* _restrict vdevice,VGraphicsPipelineSpecObj* spec_array,u32 spec_count,VkPipeline* pipeline_array,VkPipelineCache cache = 0);


VkViewport _ainline VMakeViewport(f32 x,f32 y,f32 w,f32 h,f32 min_z = 0.0f,f32 max_z = 1.0f){
#if _positive_y_up
	return {x,y + h,w,h * -1,min_z,max_z};
#else
	return {x,y,w,h,min_z,max_z};
#endif
}


VkViewport _ainline VMakeViewport(VkViewport viewport){
#if _positive_y_up
	viewport.y += viewport.height;
	viewport.height *= -1;
	return viewport;
#else
	return viewport;
#endif
}

#ifdef __cplusplus
struct VViewport{
	VkViewport viewport = {};

	VViewport(f32 x,f32 y, f32 w, f32 h, f32 min_z = 0,f32 max_z = 0){
		this->viewport = VMakeViewport(x,y,w,h,min_z,max_z);
	}


	VViewport(VkViewport vp){
		this->viewport = VMakeViewport(vp);
	}
};
#else
typedef VkViewport VViewport;
#endif



void VSetFixedViewportGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                           VViewport* viewport,u32 viewport_count,VkRect2D* scissor,
                                           u32 scissor_count);

void VSetFixedViewportGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                           u16 width,u16 height);

void VSetMultisampleGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                         VkSampleCountFlagBits samplecount_bits = VK_SAMPLE_COUNT_1_BIT,
                                         VkBool32 is_persample_perfragment = VK_FALSE,//true = sample,else frag
                                         f32 minsampleshading = 1.0f,
                                         VkSampleMask* samplemask = 0,
                                         VkBool32 enable_alpha_to_coverage = VK_FALSE,
                                         VkBool32 enable_alpha_to_one = VK_FALSE);


void VSetDepthStencilGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                          VkBool32 depthtest_enable = VK_FALSE,
                                          VkBool32 depthwrite_enable = VK_FALSE,VkCompareOp depthtest_op = VK_COMPARE_OP_NEVER,
                                          VkBool32 depthboundstest_enable = VK_FALSE,
                                          f32 min_depthbounds = 0.0f,
                                          f32 max_depthbounds = 1.0f,
                                          VkBool32 stencil_enable = false,
                                          VkStencilOpState front = {},
                                          VkStencilOpState back = {});

void VSetColorBlend(VGraphicsPipelineSpecObj* spec,
                    VkPipelineColorBlendAttachmentState* attachment_array,u32 attachment_count,
                    VkBool32 b32op_enable = VK_FALSE,VkLogicOp b32_op = VK_LOGIC_OP_CLEAR,
                    f32 blendconstants[4] = {});

void VEnableColorBlendTransparency(VGraphicsPipelineSpecObj* spec,
                                   u32 colorattachment_bitmask = 1,
                                   VkBlendFactor srccolor_blendfactor = VK_BLEND_FACTOR_SRC_ALPHA,
                                   VkBlendFactor dstcolor_blendfactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                   VkBlendOp colorblend_op = VK_BLEND_OP_ADD,
                                   VkBlendFactor srcalpha_blendfactor = VK_BLEND_FACTOR_ONE,
                                   VkBlendFactor dst_alphablendfactor = VK_BLEND_FACTOR_ZERO,
                                   VkBlendOp alphablend_op = VK_BLEND_OP_ADD,
                                   VkColorComponentFlags colorWriteMask = 0xf);

void VEnableDynamicStateGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                             VkDynamicState* dynamic_array,u32 dynamic_count);


void VPushBackShaderData(VShaderObj* obj,VkShaderStageFlagBits type,void* data,
                         u32 size,VkSpecializationInfo spec = {});



void VSetInputAssemblyState(VGraphicsPipelineSpecObj* spec,VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,VkBool32 restart = false);

void VSetRasterState(VGraphicsPipelineSpecObj* spec,VkCullModeFlags cullmode = VK_CULL_MODE_BACK_BIT,VkFrontFace frontface = VK_FRONT_FACE_CLOCKWISE,VkBool32 enable_depthclamp = false,VkPolygonMode polymode = VK_POLYGON_MODE_FILL,VkBool32 enable_depthbias = false,f32 depthbias_const = 0.0f,f32 depthbias_clamp = 0.0f,f32 depthbias_slope = 0.0f,f32 linewidth = 1.0f,VkBool32 enable_discard = false);

void VPushBackVertexAttrib(VShaderObj* obj,u32 binding_no,VkFormat format,u32 attrib_size);

void VPushBackVertexDesc(VShaderObj* obj,u32 binding_no,u32 vert_size,VkVertexInputRate inputrate);

void VPushBackSetElement(VShaderObj::DescSetEntry* set,VkDescriptorType type,u32 bind,u32 array_count);

VShaderObj::DescSetEntry* VGetSet(VShaderObj* obj,u32 set_no);

void VPushBackDescSet(VShaderObj* obj,u32 set_no,u32 shader_stage);

void VPushBackPushConstRange(VShaderObj* _restrict obj,VkFormat* format_array,u32 format_count,u32 size,VkShaderStageFlagBits shader_stage);

VkPipelineCache VCreatePipelineCache(const VDeviceContext* _in_ vdevice,void* init_data = 0,ptrsize init_size = 0);

void VGetPipelineCacheData(const VDeviceContext* _in_ vdevice,VkPipelineCache cache,void* init_data,ptrsize* init_size);

void VMapMemory(VkDevice device,VkDeviceMemory memory,
                VkDeviceSize offset,VkDeviceSize size,void** ppData,VkMemoryMapFlags flags = 0);

void _ainline VMapMemory(const VDeviceContext* _in_ vdevice,VkDeviceMemory memory,
                         VkDeviceSize offset,VkDeviceSize size,void** ppData,VkMemoryMapFlags flags = 0){
    
    VMapMemory(vdevice->device,memory,offset,size,ppData,flags);
}

struct VMemoryRangesArray{
    
    VkMappedMemoryRange range_array[16] = {};
    u32 count = 0;
};

struct VMemoryRangesPtr{
    
    VkMappedMemoryRange* range_array = 0;
    u32 count = 0;
};

void VPushBackMemoryRanges(VMemoryRangesArray* ranges,VkDeviceMemory memory,
                           VkDeviceSize offset,VkDeviceSize size);

void VPushBackMemoryRanges(VMemoryRangesPtr* ranges,VkDeviceMemory memory,
                           VkDeviceSize offset,VkDeviceSize size);

void VFlushMemoryRanges(VkDevice device,VMemoryRangesArray* ranges);

void _ainline VFlushMemoryRanges(const VDeviceContext* _in_ vdevice,VMemoryRangesArray* ranges){
    
    VFlushMemoryRanges(vdevice->device,ranges);
    
}

void VFlushMemoryRanges(VkDevice device,VMemoryRangesPtr* ranges);

void _ainline VFlushMemoryRanges(const VDeviceContext* _in_ vdevice,VMemoryRangesPtr* ranges){
    
    VFlushMemoryRanges(vdevice->device,ranges);
    
}

void VInvalidateMemoryRanges(VkDevice device,VMemoryRangesPtr* ranges);

void _ainline VInvalidateMemoryRanges(const VDeviceContext* _in_ vdevice,VMemoryRangesPtr* ranges){
    
    VInvalidateMemoryRanges(vdevice->device,ranges);
    
}

void VInvalidateMemoryRanges(VkDevice device,VMemoryRangesArray* ranges);

void _ainline VInvalidateMemoryRanges(const VDeviceContext* _in_ vdevice,VMemoryRangesArray* ranges){
    
    VInvalidateMemoryRanges(vdevice->device,ranges);
    
}

void VInitVulkan();



//we only use vkDeviceMemory for mapping
//


enum VResult{
    V_SUCCESS = 0,
    V_NO_DIRECT_MEMORY = 1,
};

//DeviceMemory allocator

VResult VInitDeviceBlockAllocator(const VDeviceContext* _restrict vdevice,u32 device_size = _megabytes(512),u32 write_size = _megabytes(256),
                                  u32 transferbuffer_size = _megabytes(64),
                                  u32 readwrite_size = _megabytes(64),
                                  u32 direct_size = _megabytes(32));

VkDeviceMemory VGetDeviceBlockMemory();
VkDeviceMemory VGetWriteBlockMemory();
VkDeviceMemory VGetReadWriteBlockMemory();
VkDeviceMemory VGetDirectBlockMemory();

//TODO: do bounds checking here and should we track these??

s8* VGetWriteBlockPtr(VBufferContext* _restrict buffer);
s8* VGetReadWriteBlockPtr(VBufferContext* _restrict buffer);
s8* VGetDirectBlockPtr(VBufferContext* _restrict buffer);

s8* VGetWriteBlockPtr(VImageMemoryContext* _restrict image);
s8* VGetReadWriteBlockPtr(VImageMemoryContext* _restrict image);
s8* VGetDirectBlockPtr(VImageMemoryContext* _restrict image);

struct VBufferCopy{
    VkBufferCopy array[32] = {};
    u32 count = 0;
};

struct VBufferImageCopy{
    VkBufferImageCopy array[32] = {};
    u32 count = 0;
};

s8* VGetTransferBufferPtr(u32 size);

//NOTE: this takes in a ptr returned by VGetTransferBufferPtr and offsets from it
VkDeviceSize VGetTransferBufferOffset(s8* _restrict transferbufferptr);
VkBuffer VGetTransferBuffer();

//NOTE: this takes in a ptr returned by VGetTransferBufferPtr and offsets from it. it has to be within the size range requested.
void VPushBackCopyBuffer(s8* _restrict transferbufferptr,VBufferCopy* _restrict copy,VkDeviceSize dst_offset,VkDeviceSize size);

void VCmdCopyBuffer(VkCommandBuffer cmdbuffer,VkBuffer dst_buffer,VBufferCopy* _restrict copy);

//NOTE: this takes in a ptr returned by VGetTransferBufferPtr and offsets from it. it has to be within the size range requested.
void VPushBackCopyBufferImage(s8* _restrict transferbufferptr,VBufferImageCopy* _restrict copy,VkExtent3D extent,VkOffset3D offset = {},
                              VkImageSubresourceLayers layers = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},
                              u32 buffer_rowlength = 0,u32 buffer_imageheight = 0);

void VCmdBufferImageCopy(VkCommandBuffer cmdbuffer,VkImage dst_image,VkImageLayout layout,VBufferImageCopy* _restrict copy);


VBufferContext VCreateUniformBufferContext(const  VDeviceContext* _restrict vdevice,
                                           u32 data_size,VMemoryBlockHintFlag flag = VBLOCK_WRITE);

VBufferContext VCreateShaderStorageBufferContext(
const  VDeviceContext* _restrict vdevice,
u32 data_size,VMemoryBlockHintFlag flag = VBLOCK_WRITE);

VTextureContext VCreateTexture(const  VDeviceContext* _restrict vdevice,u32 width,u32 height,u32 miplevels = 1,VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);

u32 _ainline VGetMaxSupportedVkVersion(){
	u32 version = VK_MAKE_VERSION(1,0,0);
	if(vkenumerateinstanceversion){
		VkResult res = vkEnumerateInstanceVersion(&version);
		_kill("",res != VK_SUCCESS);
	}

	return version;
}

void _ainline VInitQueueFamilyProperties(VkQueueFamilyProperties2* array,u32 count){
	for(u32 i = 0; i < count; i++){
		array[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
	}
}

void _ainline VCmdSetViewport(VkCommandBuffer cmdbuffer,VViewport* viewports,u32 count,u32 offset = 0){
	vkCmdSetViewport(cmdbuffer,offset,count,(VkViewport*)viewports);
}

void* VChainVKStruct(void** info_array,u32 count);


//TODO: remove these
VBufferContext TCreateStaticVertexBuffer(const  VDeviceContext* _restrict vdevice,
                                         ptrsize data_size,u32 bindingno,VMemoryBlockHintFlag flag);

VBufferContext TCreateStaticIndexBuffer(const  VDeviceContext* _restrict vdevice,
                                        ptrsize size,u32 ind_size,VMemoryBlockHintFlag flag);

//TODO: should we move these out to somewhere else??
VTextureContext VCreateTextureCache(const  VDeviceContext* _restrict vdevice,u32 width,
                                    u32 height,VkFormat format);

VTextureContext VCreateTexturePageTable(const  VDeviceContext* _restrict vdevice,
                                        u32 width,u32 height,u32 miplevels);

void VLinearDeviceMemoryBlockAlloc(u32 size,VkDeviceMemory* _restrict memory,VkDeviceSize* _restrict offset);

void VNonLinearDeviceMemoryBlockAlloc(u32 size,VkDeviceMemory* _restrict memory,VkDeviceSize* _restrict offset);

//1.1

