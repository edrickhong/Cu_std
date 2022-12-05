
extern void* vkenumerateinstanceextensionproperties;
extern void* vkenumerateinstancelayerproperties;
extern void* vkcreateinstance;
extern void* vkgetinstanceprocaddress;
extern void* vkgetdeviceprocaddress;

extern void* vkenumeratephysicaldevices;
extern void* vkgetphysicaldeviceproperties;
extern void* vkenumeratedevicelayerproperties;
extern void* vkenumeratedeviceextensionproperties;
extern void* vkgetphysicaldevicequeuefamilyproperties;
extern void* vkgetphysicaldevicefeatures;
extern void* vkcreatedevice;
extern void* vkgetphysicaldeviceformatproperties;
extern void* vkgetphysicaldevicememoryproperties;
extern void* vkcmdpipelinebarrier;
extern void* vkcreateshadermodule;
extern void* vkcreatebuffer;
extern void* vkgetbuffermemoryrequirements;
extern void* vkmapmemory;
extern void* vkunmapmemory;
extern void* vkflushmappedmemoryranges;
extern void* vkinvalidatemappedmemoryranges;
extern void* vkbindbuffermemory;
extern void* vkdestroybuffer;
extern void* vkallocatememory;
extern void* vkfreememory;
extern void* vkcreaterenderpass;
extern void* vkcmdbeginrenderpass;
extern void* vkcmdendrenderpass;
extern void* vkcmdnextsubpass;
extern void* vkcmdexecutecommands;
extern void* vkcreateimage;
extern void* vkgetimagememoryrequirements;
extern void* vkcreateimageview;
extern void* vkdestroyimageview;
extern void* vkbindimagememory;
extern void* vkgetimagesubresourcelayout;
extern void* vkcmdcopyimage;
extern void* vkcmdblitimage;
extern void* vkdestroyimage;
extern void* vkcmdclearattachments;
extern void* vkcmdcopybuffer;
extern void* vkcmdcopybuffertoimage;
extern void* vkcreatesampler;
extern void* vkdestroysampler;
extern void* vkcreatesemaphore;
extern void* vkdestroysemaphore;
extern void* vkcreatefence;
extern void* vkdestroyfence;
extern void* vkwaitforfences;
extern void* vkresetfences;
extern void* vkcreatecommandpool;
extern void* vkdestroycommandpool;
extern void* vkallocatecommandbuffers;
extern void* vkbegincommandbuffer;
extern void* vkendcommandbuffer;
extern void* vkgetdevicequeue;
extern void* vkqueuesubmit;
extern void* vkqueuewaitidle;
extern void* vkdevicewaitidle;
extern void* vkcreateframebuffer;
extern void* vkcreatepipelinecache;
extern void* vkcreatepipelinelayout;
extern void* vkcreategraphicspipelines;
extern void* vkcreatecomputepipelines;
extern void* vkcreatedescriptorpool;
extern void* vkcreatedescriptorsetlayout;
extern void* vkallocatedescriptorsets;
extern void* vkupdatedescriptorsets;
extern void* vkcmdbinddescriptorsets;
extern void* vkcmdbindpipeline;
extern void* vkcmdbindvertexbuffers;
extern void* vkcmdbindindexbuffer;
extern void* vkcmdsetviewport;
extern void* vkcmdsetscissor;
extern void* vkcmdsetlinewidth;
extern void* vkcmdsetdepthbias;
extern void* vkcmdpushconstants;
extern void* vkcmddrawindexed;
extern void* vkcmddraw;
extern void* vkcmddrawindexedindirect;
extern void* vkcmddrawindirect;
extern void* vkcmddispatch;
extern void* vkdestroypipeline;
extern void* vkdestroypipelinelayout;
extern void* vkdestroydescriptorsetlayout;
extern void* vkdestroydevice;
extern void* vkdestroyinstance;
extern void* vkdestroydescriptorpool;
extern void* vkfreecommandbuffers;
extern void* vkdestroyrenderpass;
extern void* vkdestroyframebuffer;
extern void* vkdestroyshadermodule;
extern void* vkdestroypipelinecache;
extern void* vkcreatequerypool;
extern void* vkdestroyquerypool;
extern void* vkgetquerypoolresults;
extern void* vkcmdbeginquery;
extern void* vkcmdendquery;
extern void* vkcmdresetquerypool;
extern void* vkcmdcopyquerypoolresults;
extern void* vkcreate_xlib_wayland_win32surfacekhr;
extern void* vkdestroysurfacekhr;
extern void* vkcmdfillbuffer;
extern void* vkacquirenextimagekhr;
extern void* vkgetfencestatus;
extern void* vkcreateswapchainkhr;
extern void* vkgetswapchainimageskhr;
extern void* vkqueuepresentkhr;
extern void* vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr;
extern void* vkgetphysicaldevicesurfacesupportkhr;
extern void* vkcmdclearcolorimage;
extern void* vkgetphysicaldeviceimageformatproperties;
extern void* vkcmdcopyimagetobuffer;
extern void* vkgetpipelinecachedata;
extern void* vkgetimagesparsememoryrequirements;
extern void* vkgetphysicaldevicesparseimageformatproperties;

//vulkan 1.1
extern void* vkenumerateinstanceversion;
extern void* vkenumeratephysicaldevicegroups;
extern void* vkbindbuffermemory2;
extern void* vkbindimagememory2;

extern void* vkgetbuffermemoryrequirements2;
extern void* vkgetimagememoryrequirements2;
extern void* vkgetimagesparsememoryrequirements2;


extern void* vkgetphysicaldeviceproperties2;
extern void* vkgetphysicaldevicefeatures2;
extern void* vkgetphysicaldeviceformatproperties2;
extern void* vkgetphysicaldeviceimageformatproperties2;
extern void* vkgetphysicaldevicememoryproperties2;
extern void* vkgetphysicaldevicequeuefamilyproperties2;
extern void* vkgetphysicaldevicesparseimageformatproperties2;

//defines
#define vkEnumerateInstanceExtensionProperties ((PFN_vkEnumerateInstanceExtensionProperties)(vkenumerateinstanceextensionproperties))
#define vkEnumerateInstanceLayerProperties ((PFN_vkEnumerateInstanceLayerProperties)(vkenumerateinstancelayerproperties))
#define vkCreateInstance ((PFN_vkCreateInstance)(vkcreateinstance))
#define vkGetInstanceProcAddr ((PFN_vkGetInstanceProcAddr)(vkgetinstanceprocaddress))
#define vkGetDeviceProcAddr ((PFN_vkGetDeviceProcAddr)(vkgetdeviceprocaddress))

#define vkEnumeratePhysicalDevices ((PFN_vkEnumeratePhysicalDevices)(vkenumeratephysicaldevices))
#define vkGetPhysicalDeviceProperties ((PFN_vkGetPhysicalDeviceProperties)(vkgetphysicaldeviceproperties))
#define vkEnumerateDeviceLayerProperties ((PFN_vkEnumerateDeviceLayerProperties)(vkenumeratedevicelayerproperties))
#define vkEnumerateDeviceExtensionProperties ((PFN_vkEnumerateDeviceExtensionProperties)(vkenumeratedeviceextensionproperties))
#define vkGetPhysicalDeviceQueueFamilyProperties ((PFN_vkGetPhysicalDeviceQueueFamilyProperties)(vkgetphysicaldevicequeuefamilyproperties))
#define vkGetPhysicalDeviceFeatures ((PFN_vkGetPhysicalDeviceFeatures)(vkgetphysicaldevicefeatures))
#define vkCreateDevice ((PFN_vkCreateDevice)(vkcreatedevice))
#define vkGetPhysicalDeviceFormatProperties ((PFN_vkGetPhysicalDeviceFormatProperties)(vkgetphysicaldeviceformatproperties))
#define vkGetPhysicalDeviceMemoryProperties ((PFN_vkGetPhysicalDeviceMemoryProperties)(vkgetphysicaldevicememoryproperties))
#define vkCmdPipelineBarrier ((PFN_vkCmdPipelineBarrier)(vkcmdpipelinebarrier))
#define vkCreateShaderModule ((PFN_vkCreateShaderModule)(vkcreateshadermodule))
#define vkCreateBuffer ((PFN_vkCreateBuffer)(vkcreatebuffer))
#define vkGetBufferMemoryRequirements ((PFN_vkGetBufferMemoryRequirements)(vkgetbuffermemoryrequirements))
#define vkMapMemory ((PFN_vkMapMemory)(vkmapmemory))
#define vkUnmapMemory ((PFN_vkUnmapMemory)(vkunmapmemory))
#define vkFlushMappedMemoryRanges ((PFN_vkFlushMappedMemoryRanges)(vkflushmappedmemoryranges))
#define vkInvalidateMappedMemoryRanges ((PFN_vkInvalidateMappedMemoryRanges)(vkinvalidatemappedmemoryranges))
#define vkBindBufferMemory ((PFN_vkBindBufferMemory)(vkbindbuffermemory))
#define vkDestroyBuffer ((PFN_vkDestroyBuffer)(vkdestroybuffer))
#define vkAllocateMemory ((PFN_vkAllocateMemory)(vkallocatememory))
#define vkFreeMemory ((PFN_vkFreeMemory)(vkfreememory))
#define vkCreateRenderPass ((PFN_vkCreateRenderPass)(vkcreaterenderpass))
#define vkCmdBeginRenderPass ((PFN_vkCmdBeginRenderPass)(vkcmdbeginrenderpass))
#define vkCmdEndRenderPass ((PFN_vkCmdEndRenderPass)(vkcmdendrenderpass))
#define vkCmdNextSubpass ((PFN_vkCmdNextSubpass)(vkcmdnextsubpass))
#define vkCmdExecuteCommands ((PFN_vkCmdExecuteCommands)(vkcmdexecutecommands))
#define vkCreateImage ((PFN_vkCreateImage)(vkcreateimage))
#define vkGetImageMemoryRequirements ((PFN_vkGetImageMemoryRequirements)(vkgetimagememoryrequirements))
#define vkCreateImageView ((PFN_vkCreateImageView)(vkcreateimageview))
#define vkDestroyImageView ((PFN_vkDestroyImageView)(vkdestroyimageview))
#define vkBindImageMemory ((PFN_vkBindImageMemory)(vkbindimagememory))
#define vkGetImageSubresourceLayout ((PFN_vkGetImageSubresourceLayout)(vkgetimagesubresourcelayout))
#define vkCmdCopyImage ((PFN_vkCmdCopyImage)(vkcmdcopyimage))
#define vkCmdBlitImage ((PFN_vkCmdBlitImage)(vkcmdblitimage))
#define vkDestroyImage ((PFN_vkDestroyImage)(vkdestroyimage))
#define vkCmdClearAttachments ((PFN_vkCmdClearAttachments)(vkcmdclearattachments))
#define vkCmdCopyBuffer ((PFN_vkCmdCopyBuffer)(vkcmdcopybuffer))
#define vkCmdCopyBufferToImage ((PFN_vkCmdCopyBufferToImage)(vkcmdcopybuffertoimage))
#define vkCreateSampler ((PFN_vkCreateSampler)(vkcreatesampler))
#define vkDestroySampler ((PFN_vkDestroySampler)(vkdestroysampler))
#define vkCreateSemaphore ((PFN_vkCreateSemaphore)(vkcreatesemaphore))
#define vkDestroySemaphore ((PFN_vkDestroySemaphore)(vkdestroysemaphore))
#define vkCreateFence ((PFN_vkCreateFence)(vkcreatefence))
#define vkDestroyFence ((PFN_vkDestroyFence)(vkdestroyfence))
#define vkWaitForFences ((PFN_vkWaitForFences)(vkwaitforfences))
#define vkResetFences ((PFN_vkResetFences)(vkresetfences))
#define vkCreateCommandPool ((PFN_vkCreateCommandPool)(vkcreatecommandpool))
#define vkDestroyCommandPool ((PFN_vkDestroyCommandPool)(vkdestroycommandpool))
#define vkAllocateCommandBuffers ((PFN_vkAllocateCommandBuffers)(vkallocatecommandbuffers))
#define vkBeginCommandBuffer ((PFN_vkBeginCommandBuffer)(vkbegincommandbuffer))
#define vkEndCommandBuffer ((PFN_vkEndCommandBuffer)(vkendcommandbuffer))
#define vkGetDeviceQueue ((PFN_vkGetDeviceQueue)(vkgetdevicequeue))
#define vkQueueSubmit ((PFN_vkQueueSubmit)(vkqueuesubmit))
#define vkQueueWaitIdle ((PFN_vkQueueWaitIdle)(vkqueuewaitidle))
#define vkDeviceWaitIdle ((PFN_vkDeviceWaitIdle)(vkdevicewaitidle))
#define vkCreateFramebuffer ((PFN_vkCreateFramebuffer)(vkcreateframebuffer))
#define vkCreatePipelineCache ((PFN_vkCreatePipelineCache)(vkcreatepipelinecache))
#define vkCreatePipelineLayout ((PFN_vkCreatePipelineLayout)(vkcreatepipelinelayout))
#define vkCreateGraphicsPipelines ((PFN_vkCreateGraphicsPipelines)(vkcreategraphicspipelines))
#define vkCreateComputePipelines ((PFN_vkCreateComputePipelines)(vkcreatecomputepipelines))
#define vkCreateDescriptorPool ((PFN_vkCreateDescriptorPool)(vkcreatedescriptorpool))
#define vkCreateDescriptorSetLayout ((PFN_vkCreateDescriptorSetLayout)(vkcreatedescriptorsetlayout))
#define vkAllocateDescriptorSets ((PFN_vkAllocateDescriptorSets)(vkallocatedescriptorsets))
#define vkUpdateDescriptorSets ((PFN_vkUpdateDescriptorSets)(vkupdatedescriptorsets))
#define vkCmdBindDescriptorSets ((PFN_vkCmdBindDescriptorSets)(vkcmdbinddescriptorsets))
#define vkCmdBindPipeline ((PFN_vkCmdBindPipeline)(vkcmdbindpipeline))
#define vkCmdBindVertexBuffers ((PFN_vkCmdBindVertexBuffers)(vkcmdbindvertexbuffers))
#define vkCmdBindIndexBuffer ((PFN_vkCmdBindIndexBuffer)(vkcmdbindindexbuffer))
#define vkCmdSetViewport ((PFN_vkCmdSetViewport)(vkcmdsetviewport))
#define vkCmdSetScissor ((PFN_vkCmdSetScissor)(vkcmdsetscissor))
#define vkCmdSetLineWidth ((PFN_vkCmdSetLineWidth)(vkcmdsetlinewidth))
#define vkCmdSetDepthBias ((PFN_vkCmdSetDepthBias)(vkcmdsetdepthbias))
#define vkCmdPushConstants ((PFN_vkCmdPushConstants)(vkcmdpushconstants))
#define vkCmdDrawIndexed ((PFN_vkCmdDrawIndexed)(vkcmddrawindexed))
#define vkCmdDraw ((PFN_vkCmdDraw)(vkcmddraw))
#define vkCmdDrawIndexedIndirect ((PFN_vkCmdDrawIndexedIndirect)(vkcmddrawindexedindirect))
#define vkCmdDrawIndirect ((PFN_vkCmdDrawIndirect)(vkcmddrawindirect))
#define vkCmdDispatch ((PFN_vkCmdDispatch)(vkcmddispatch))
#define vkDestroyPipeline ((PFN_vkDestroyPipeline)(vkdestroypipeline))
#define vkDestroyPipelineLayout ((PFN_vkDestroyPipelineLayout)(vkdestroypipelinelayout))
#define vkDestroyDescriptorSetLayout ((PFN_vkDestroyDescriptorSetLayout)(vkdestroydescriptorsetlayout))
#define vkDestroyDevice ((PFN_vkDestroyDevice)(vkdestroydevice))
#define vkDestroyInstance ((PFN_vkDestroyInstance)(vkdestroyinstance))
#define vkDestroyDescriptorPool ((PFN_vkDestroyDescriptorPool)(vkdestroydescriptorpool))
#define vkFreeCommandBuffers ((PFN_vkFreeCommandBuffers)(vkfreecommandbuffers))
#define vkDestroyRenderPass ((PFN_vkDestroyRenderPass)(vkdestroyrenderpass))
#define vkDestroyFramebuffer ((PFN_vkDestroyFramebuffer)(vkdestroyframebuffer))
#define vkDestroyShaderModule ((PFN_vkDestroyShaderModule)(vkdestroyshadermodule))
#define vkDestroyPipelineCache ((PFN_vkDestroyPipelineCache)(vkdestroypipelinecache))
#define vkCreateQueryPool ((PFN_vkCreateQueryPool)(vkcreatequerypool))
#define vkDestroyQueryPool ((PFN_vkDestroyQueryPool)(vkdestroyquerypool))
#define vkGetQueryPoolResults ((PFN_vkGetQueryPoolResults)(vkgetquerypoolresults))
#define vkCmdBeginQuery ((PFN_vkCmdBeginQuery)(vkcmdbeginquery))
#define vkCmdEndQuery ((PFN_vkCmdEndQuery)(vkcmdendquery))
#define vkCmdResetQueryPool ((PFN_vkCmdResetQueryPool)(vkcmdresetquerypool))
#define vkCmdCopyQueryPoolResults ((PFN_vkCmdCopyQueryPoolResults)(vkcmdcopyquerypoolresults))
#define vkCreateXlibSurfaceKHR ((PFN_vkCreateXlibSurfaceKHR)(vkcreate_xlib_wayland_win32surfacekhr))
#define vkCreateWaylandSurfaceKHR ((PFN_vkCreateWaylandSurfaceKHR)(vkcreate_xlib_wayland_win32surfacekhr))


#define vkCreateWin32SurfaceKHR ((PFN_vkCreateWin32SurfaceKHR)(vkcreate_xlib_wayland_win32surfacekhr))


#define vkDestroySurfaceKHR ((PFN_vkDestroySurfaceKHR)(vkdestroysurfacekhr))

#define vkCmdFillBuffer ((PFN_vkCmdFillBuffer)vkcmdfillbuffer)
#define vkAcquireNextImageKHR ((PFN_vkAcquireNextImageKHR)vkacquirenextimagekhr)
#define vkGetFenceStatus ((PFN_vkGetFenceStatus)vkgetfencestatus)
#define vkCreateSwapchainKHR ((PFN_vkCreateSwapchainKHR)vkcreateswapchainkhr)
#define vkGetSwapchainImagesKHR ((PFN_vkGetSwapchainImagesKHR)vkgetswapchainimageskhr)
#define vkQueuePresentKHR ((PFN_vkQueuePresentKHR)vkqueuepresentkhr)

#define vkGetPhysicalDeviceXlibPresentationSupportKHR ((PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr)

#define vkGetPhysicalDeviceWaylandPresentationSupportKHR ((PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr)

#define vkGetPhysicalDeviceWin32PresentationSupportKHR ((PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr)

#define vkGetPhysicalDeviceSurfaceSupportKHR ((PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkgetphysicaldevicesurfacesupportkhr)

#define vkCmdClearColorImage ((PFN_vkCmdClearColorImage)vkcmdclearcolorimage)
#define vkGetPhysicalDeviceImageFormatProperties ((PFN_vkGetPhysicalDeviceImageFormatProperties)vkgetphysicaldeviceimageformatproperties)

#define vkCmdCopyImageToBuffer ((PFN_vkCmdCopyImageToBuffer)vkcmdcopyimagetobuffer)

#define vkGetPipelineCacheData ((PFN_vkGetPipelineCacheData)vkgetpipelinecachedata)


//#define vkGetPhysicalDeviceSparseImageFormatProperties ((PFN_vkGetPhysicalDeviceSparseImageFormatProperties)vkgetphysicaldevicesparseimageformatproperties)

//vulkan 1.1
#define vkEnumeratePhysicalDeviceGroups ((PFN_vkEnumeratePhysicalDeviceGroups)vkenumeratephysicaldevicegroups)
#define vkEnumerateInstanceVersion ((PFN_vkEnumerateInstanceVersion)vkenumerateinstanceversion)
#define vkBindBufferMemory2 ((PFN_vkBindBufferMemory2)vkbindbuffermemory2)
#define vkBindImageMemory2 ((PFN_vkBindImageMemory2)vkbindimagememory2)

#define vkGetBufferMemoryRequirements2 ((PFN_vkGetBufferMemoryRequirements2)vkgetbuffermemoryrequirements2)
#define vkGetImageMemoryRequirements2 ((PFN_vkGetImageMemoryRequirements2)vkgetimagememoryrequirements2)
#define vkGetImageSparseMemoryRequirements2 ((PFN_vkGetImageSparseMemoryRequirements2)vkgetimagesparsememoryrequirements2)

#define vkGetPhysicalDeviceProperties2 ((PFN_vkGetPhysicalDeviceProperties2)vkgetphysicaldeviceproperties2)
#define vkGetPhysicalDeviceFeatures2 ((PFN_vkGetPhysicalDeviceFeatures2)vkgetphysicaldevicefeatures2)
#define vkGetPhysicalDeviceFormatProperties2 ((PFN_vkGetPhysicalDeviceFormatProperties2)vkgetphysicaldeviceformatproperties2)
#define vkGetPhysicalDeviceImageFormatProperties2 ((PFN_vkGetPhysicalDeviceImageFormatProperties2)vkgetphysicaldeviceimageformatproperties2)
#define vkGetPhysicalDeviceMemoryProperties2 ((PFN_vkGetPhysicalDeviceMemoryProperties2)vkgetphysicaldevicememoryproperties2)
#define vkGetPhysicalDeviceQueueFamilyProperties2 ((PFN_vkGetPhysicalDeviceQueueFamilyProperties2)vkgetphysicaldevicequeuefamilyproperties2)
//#define vkGetPhysicalDeviceSparseImageFormatProperties2 ((PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)vkgetphysicaldevicesparseimageformatproperties2)



extern VkPhysicalDeviceProperties (*VGetPhysicalDeviceProperties)(VkPhysicalDevice);
