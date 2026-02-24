
void* vkenumerateinstanceextensionproperties;
void* vkenumerateinstancelayerproperties;
void* vkcreateinstance;
void* vkgetinstanceprocaddress;
void* vkgetdeviceprocaddress;

void* vkenumeratephysicaldevices;
void* vkgetphysicaldeviceproperties;
void* vkenumeratedevicelayerproperties;
void* vkenumeratedeviceextensionproperties;
void* vkgetphysicaldevicequeuefamilyproperties;
void* vkgetphysicaldevicefeatures;
void* vkcreatedevice;
void* vkgetphysicaldeviceformatproperties;
void* vkgetphysicaldevicememoryproperties;
void* vkcmdpipelinebarrier;
void* vkcreateshadermodule;
void* vkcreatebuffer;
void* vkgetbuffermemoryrequirements;
void* vkmapmemory;
void* vkunmapmemory;
void* vkflushmappedmemoryranges;
void* vkinvalidatemappedmemoryranges;
void* vkbindbuffermemory;
void* vkdestroybuffer;
void* vkallocatememory;
void* vkfreememory;
void* vkcreaterenderpass;
void* vkcmdbeginrenderpass;
void* vkcmdendrenderpass;
void* vkcmdnextsubpass;
void* vkcmdexecutecommands;
void* vkcreateimage;
void* vkgetimagememoryrequirements;
void* vkcreateimageview;
void* vkdestroyimageview;
void* vkbindimagememory;
void* vkgetimagesubresourcelayout;
void* vkcmdcopyimage;
void* vkcmdblitimage;
void* vkdestroyimage;
void* vkcmdclearattachments;
void* vkcmdcopybuffer;
void* vkcmdcopybuffertoimage;
void* vkcreatesampler;
void* vkdestroysampler;
void* vkcreatesemaphore;
void* vkdestroysemaphore;
void* vkcreatefence;
void* vkdestroyfence;
void* vkwaitforfences;
void* vkresetfences;
void* vkcreatecommandpool;
void* vkdestroycommandpool;
void* vkallocatecommandbuffers;
void* vkbegincommandbuffer;
void* vkendcommandbuffer;
void* vkgetdevicequeue;
void* vkqueuesubmit;
void* vkqueuewaitidle;
void* vkdevicewaitidle;
void* vkcreateframebuffer;
void* vkcreatepipelinecache;
void* vkcreatepipelinelayout;
void* vkcreategraphicspipelines;
void* vkcreatecomputepipelines;
void* vkcreatedescriptorpool;
void* vkcreatedescriptorsetlayout;
void* vkallocatedescriptorsets;
void* vkupdatedescriptorsets;
void* vkcmdbinddescriptorsets;
void* vkcmdbindpipeline;
void* vkcmdbindvertexbuffers;
void* vkcmdbindindexbuffer;
void* vkcmdsetviewport;
void* vkcmdsetscissor;
void* vkcmdsetlinewidth;
void* vkcmdsetdepthbias;
void* vkcmdpushconstants;
void* vkcmddrawindexed;
void* vkcmddraw;
void* vkcmddrawindexedindirect;
void* vkcmddrawindirect;
void* vkcmddispatch;
void* vkdestroypipeline;
void* vkdestroypipelinelayout;
void* vkdestroydescriptorsetlayout;
void* vkdestroydevice;
void* vkdestroyinstance;
void* vkdestroydescriptorpool;
void* vkfreecommandbuffers;
void* vkdestroyrenderpass;
void* vkdestroyframebuffer;
void* vkdestroyshadermodule;
void* vkdestroypipelinecache;
void* vkcreatequerypool;
void* vkdestroyquerypool;
void* vkgetquerypoolresults;
void* vkcmdbeginquery;
void* vkcmdendquery;
void* vkcmdresetquerypool;
void* vkcmdcopyquerypoolresults;
void* vkcreate_xlib_wayland_win32surfacekhr;
void* vkdestroysurfacekhr;
void* vkcmdfillbuffer;
void* vkacquirenextimagekhr;
void* vkgetfencestatus;
void* vkcreateswapchainkhr;
void* vkgetswapchainimageskhr;
void* vkqueuepresentkhr;
void* vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr;
void* vkgetphysicaldevicesurfacesupportkhr;
void* vkcmdclearcolorimage;
void* vkgetphysicaldeviceimageformatproperties;
void* vkcmdcopyimagetobuffer;
void* vkgetpipelinecachedata;
void* vkgetimagesparsememoryrequirements;
void* vkgetphysicaldevicesparseimageformatproperties;

//vulkan 1.1
void* vkenumerateinstanceversion;
void* vkenumeratephysicaldevicegroups;

void* vkbindbuffermemory2;
void* vkbindimagememory2;

void* vkgetbuffermemoryrequirements2;
void* vkgetimagememoryrequirements2;
void* vkgetimagesparsememoryrequirements2;

void* vkgetphysicaldeviceproperties2;
void* vkgetphysicaldevicefeatures2;
void* vkgetphysicaldeviceformatproperties2;
void* vkgetphysicaldeviceimageformatproperties2;
void* vkgetphysicaldevicememoryproperties2;
void* vkgetphysicaldevicequeuefamilyproperties2;
void* vkgetphysicaldevicesparseimageformatproperties2;

void* vktrimcommandpool;

void* vkgetdescriptorlayoutsupport;

void* vkdestroydescriptorupdatetemplate;
void* vkcreatedescriptorupdatetemplate;
void* vkupdatedescriptorsetwithtemplate;

#define _instproc(fptr,inst,entrypoint)				\
{									\
    fptr = (void*)vkGetInstanceProcAddr(inst, ""#entrypoint); \
    if (!fptr)								\
    {									\
        _kill("failed to get function\n",1);							\
    }									\
}

// Macro to get a procedure address based on a vulkan device
#define _deviceproc(fptr,dev,entrypoint)				\
{									\
    fptr = (void*)vkGetDeviceProcAddr(dev, ""#entrypoint); \
    if (!fptr)								\
    {									\
        _kill("failed to get function\n",1)							\
    }								\
}

#define _isdeprecated(var) ((void*)var == (void*)-1)
#define _deprecate_func(var) var = (void*)-1


//TODO: add ability to extend

VkPhysicalDeviceProperties VGetPhysicalDeviceProperties1(VkPhysicalDevice device){
	VkPhysicalDeviceProperties  prop = {};
	vkGetPhysicalDeviceProperties(device,&prop);

	return prop;
}


VkPhysicalDeviceProperties VGetPhysicalDeviceProperties2(VkPhysicalDevice device){
	VkPhysicalDeviceProperties2  prop = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
	vkGetPhysicalDeviceProperties2(device,&prop);

	return prop.properties;
}


VkPhysicalDeviceFeatures VGetPhysicalDeviceFeatures1(VkPhysicalDevice device){
	VkPhysicalDeviceFeatures feat = {};
	vkGetPhysicalDeviceFeatures(device,&feat);

	return feat;
}

VkPhysicalDeviceFeatures VGetPhysicalDeviceFeatures2(VkPhysicalDevice device){
	VkPhysicalDeviceFeatures2 feat = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
	vkGetPhysicalDeviceFeatures2(device,&feat);
	return feat.features;
}


VkFormatProperties VGetPhysicalDeviceFormatProperties1(VkPhysicalDevice device, VkFormat format){
	VkFormatProperties prop = {};
	vkGetPhysicalDeviceFormatProperties(device,format,&prop);

	return prop;
}

VkFormatProperties VGetPhysicalDeviceFormatProperties2(VkPhysicalDevice device, VkFormat format){
	VkFormatProperties2 prop = {VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
	vkGetPhysicalDeviceFormatProperties2(device,format,&prop);

	return prop.formatProperties;
}


VkImageFormatProperties VGetPhysicalDeviceImageFormatProperties1(VkPhysicalDevice device,VkFormat format,VkImageType type,
		VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags){
	VkImageFormatProperties prop = {};
	_vktest(vkGetPhysicalDeviceImageFormatProperties(device,format,type,tiling,usage,flags,&prop));

	return prop;
}


VkImageFormatProperties VGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice device,VkFormat format,VkImageType type,
		VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags){

	VkPhysicalDeviceImageFormatInfo2 f = {
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,0,format,type,tiling,usage,flags
	};
	VkImageFormatProperties2 prop = {VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2};

	_vktest(vkGetPhysicalDeviceImageFormatProperties2(device,&f,&prop));

	return prop.imageFormatProperties;
}


VkPhysicalDeviceMemoryProperties VGetPhysicalDeviceMemoryProperties1(VkPhysicalDevice device){
	VkPhysicalDeviceMemoryProperties prop = {}; 
	vkGetPhysicalDeviceMemoryProperties(device,&prop);

	return prop;
}

VkPhysicalDeviceMemoryProperties VGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice device){
	VkPhysicalDeviceMemoryProperties2 prop = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2}; 
	vkGetPhysicalDeviceMemoryProperties2(device,&prop);

	return prop.memoryProperties;
}

void _ainline VGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device,u32* count,VkQueueFamilyProperties2* props){
	if(vkGetPhysicalDeviceQueueFamilyProperties2){
		vkGetPhysicalDeviceQueueFamilyProperties2(device,count,props);
	}

	else{

		_kill("",*count >= 1024);
		VkQueueFamilyProperties p[1024] = {};
		vkGetPhysicalDeviceQueueFamilyProperties(device,count,p);

		if(count && props){
			for(u32 i = 0; i < (*count); i++){
				props[i].queueFamilyProperties = p[i];
			}
		}

	}
}

// NOTE: Memory req
VkMemoryRequirements VGetBufferMemoryRequirements1(VkDevice device,VkBuffer buffer,void* m_next,void* info_next){
	VkMemoryRequirements memreq = {};
	vkGetBufferMemoryRequirements(device,buffer,&memreq);

	return memreq;
}


VkMemoryRequirements VGetBufferMemoryRequirements2(VkDevice device,VkBuffer buffer,void* m_next,void* info_next){
	VkMemoryRequirements memreq = {};
	VkBufferMemoryRequirementsInfo2 b_info = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,info_next,buffer};
	VkMemoryRequirements2 memreq2 = {
		VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
		m_next,
		{}
	};
	vkGetBufferMemoryRequirements2(device,&b_info,&memreq2);

	memreq = memreq2.memoryRequirements;

	return memreq;
}


VkMemoryRequirements VGetImageMemoryRequirements1(VkDevice device,VkImage image,void* m_next,void* info_next){
	VkMemoryRequirements memreq = {};
	vkGetImageMemoryRequirements(device,image,&memreq);

	return memreq;
}


VkMemoryRequirements VGetImageMemoryRequirements2(VkDevice device,VkImage image,void* m_next,void* info_next){
	VkMemoryRequirements memreq = {};
	VkImageMemoryRequirementsInfo2 i_info = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,info_next,image};
	VkMemoryRequirements2 memreq2 = {
		VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
		m_next,
		{}
	};
	vkGetImageMemoryRequirements2(device,&i_info,&memreq2);

	memreq = memreq2.memoryRequirements;

	return memreq;
}

//MARK: Implementation

_global LibHandle vklib = 0;


_global VkInstance global_instance = 0;
_global u32 global_version_no = 0;

#ifdef DEBUG 
_global VkDevice global_device = 0;
#endif


_intern VkMemoryRequirements (*VGetBufferMemoryRequirements)(VkDevice,VkBuffer,void*,void*) = 0;
_intern VkMemoryRequirements (*VGetImageMemoryRequirements)(VkDevice,VkImage,void*,void*) = 0;

VkPhysicalDeviceProperties (*VGetPhysicalDeviceProperties)(VkPhysicalDevice) = 0;
_intern VkPhysicalDeviceFeatures (*VGetPhysicalDeviceFeatures)(VkPhysicalDevice) = 0;
_intern VkFormatProperties (*VGetPhysicalDeviceFormatProperties)(VkPhysicalDevice,VkFormat) = 0;
_intern VkImageFormatProperties (*VGetPhysicalDeviceImageFormatProperties)(VkPhysicalDevice,VkFormat,VkImageType,VkImageTiling,VkImageUsageFlags,VkImageCreateFlags) = 0;
_intern VkPhysicalDeviceMemoryProperties (*VGetPhysicalDeviceMemoryProperties)(VkPhysicalDevice) = 0;


void VInitVulkan(){
    
    //MARK: expand this list
    const s8* vklib_array[] = {
        
#ifdef _WIN32
        "vulkan-1.dll",
        "vulkan.dll",
#else
        "libvulkan.so.1",
        "libvulkan.so",
#endif
        
    };
    
    for(u32 i = 0; i < _arraycount(vklib_array); i++){
        
        vklib = LLoadLibrary(vklib_array[i]);
        
        if(vklib){
            _dprint("using vkloader %s\n",vklib_array[i]);
            break;
        }
        
    }
    
    _kill("faild to load vulkan lib\n",!vklib);


    vkgetinstanceprocaddress = LGetLibFunction(vklib,"vkGetInstanceProcAddr");
    vkgetdeviceprocaddress = LGetLibFunction(vklib,"vkGetDeviceProcAddr");
    
    vkenumerateinstanceextensionproperties =
        LGetLibFunction(vklib,"vkEnumerateInstanceExtensionProperties");
    
    vkenumerateinstancelayerproperties =
        LGetLibFunction(vklib,"vkEnumerateInstanceLayerProperties");
    
    vkcreateinstance = LGetLibFunction(vklib,"vkCreateInstance");
    
    vkdestroyinstance = LGetLibFunction(vklib,"vkDestroyInstance");
    
    

    // vulkan 1.1
    vkenumerateinstanceversion = LGetLibFunction(vklib,"vkEnumerateInstanceVersion");
    
}


void InternalLoadVulkanInstanceLevelFunctions(){
    
    _instproc(vkenumeratephysicaldevices,global_instance,vkEnumeratePhysicalDevices);
    
    
    _instproc(vkcreatedevice,global_instance,vkCreateDevice);
    
    _instproc(vkgetphysicaldevicesurfacesupportkhr,global_instance,vkGetPhysicalDeviceSurfaceSupportKHR);
    
    _instproc(vkenumeratedevicelayerproperties,global_instance,vkEnumerateDeviceLayerProperties);
    
    _instproc(vkenumeratedeviceextensionproperties,global_instance,vkEnumerateDeviceExtensionProperties);
    
    
    _instproc(vkdestroysurfacekhr,global_instance,vkDestroySurfaceKHR);


    _instproc(vkgetphysicaldevicequeuefamilyproperties,global_instance,vkGetPhysicalDeviceQueueFamilyProperties);
    _instproc(vkgetphysicaldeviceproperties,global_instance,vkGetPhysicalDeviceProperties);
    _instproc(vkgetphysicaldevicememoryproperties,global_instance,vkGetPhysicalDeviceMemoryProperties);
    _instproc(vkgetphysicaldevicefeatures,global_instance,vkGetPhysicalDeviceFeatures);
    _instproc(vkgetphysicaldeviceformatproperties,global_instance,vkGetPhysicalDeviceFormatProperties);
    _instproc(vkgetphysicaldeviceimageformatproperties,global_instance,vkGetPhysicalDeviceImageFormatProperties);
    
    //vulkan 1.1 here
    
    if(VK_VERSION_MINOR(global_version_no) >= 1){
        
        //TODO: deprecate 1.0 functions (set them to -1)
        
	    _instproc(vkenumeratephysicaldevicegroups,global_instance,vkEnumeratePhysicalDeviceGroups);
	    _instproc(vkgetphysicaldeviceproperties2,global_instance,vkGetPhysicalDeviceProperties2);
	    _instproc(vkgetphysicaldevicefeatures2,global_instance,vkGetPhysicalDeviceFeatures2 );
	    _instproc(vkgetphysicaldeviceformatproperties2,global_instance,vkGetPhysicalDeviceFormatProperties2 );
	    _instproc(vkgetphysicaldeviceimageformatproperties2,global_instance,vkGetPhysicalDeviceImageFormatProperties2 );
	    _instproc(vkgetphysicaldevicememoryproperties2,global_instance,vkGetPhysicalDeviceMemoryProperties2 );
	    _instproc(vkgetphysicaldevicequeuefamilyproperties2,global_instance,vkGetPhysicalDeviceQueueFamilyProperties2 );
	    _instproc(vkgetphysicaldevicesparseimageformatproperties2,global_instance,vkGetPhysicalDeviceSparseImageFormatProperties2 );

	    _deprecate_func(vkgetphysicaldeviceproperties);
	    _deprecate_func(vkgetphysicaldevicefeatures);
	    _deprecate_func(vkgetphysicaldeviceformatproperties);
	    _deprecate_func(vkgetphysicaldeviceimageformatproperties);
	    _deprecate_func(vkgetphysicaldevicememoryproperties);
	    _deprecate_func(vkgetphysicaldevicequeuefamilyproperties);
	    _deprecate_func(vkgetphysicaldevicesparseimageformatproperties);

	    VGetPhysicalDeviceProperties = 
		    _isdeprecated(vkGetPhysicalDeviceProperties) ? VGetPhysicalDeviceProperties2 : VGetPhysicalDeviceProperties1;

	    VGetPhysicalDeviceFeatures = 
		    _isdeprecated(vkGetPhysicalDeviceFeatures) ? VGetPhysicalDeviceFeatures2 : VGetPhysicalDeviceFeatures1;

	    VGetPhysicalDeviceFormatProperties = 
		    _isdeprecated(vkGetPhysicalDeviceFormatProperties) ? VGetPhysicalDeviceFormatProperties2 : VGetPhysicalDeviceFormatProperties1;

	    VGetPhysicalDeviceImageFormatProperties = 
		    _isdeprecated(vkGetPhysicalDeviceImageFormatProperties) ? VGetPhysicalDeviceImageFormatProperties2 : VGetPhysicalDeviceImageFormatProperties1;

	    VGetPhysicalDeviceMemoryProperties = 
		    _isdeprecated(vkGetPhysicalDeviceMemoryProperties) ? VGetPhysicalDeviceMemoryProperties2 : VGetPhysicalDeviceMemoryProperties1;

    }
}

void InternalLoadVulkanFunctions(void* k,void* load_fptr){
    
    
    _kill("",!vkenumeratephysicaldevices);
    
    auto load = (void* (*)(void*,const s8*))load_fptr;
    
#define _initfunc(func,var) var = (void*)load(k,""#func); if(!var){printf("%s %s %d :: Failed to load function %s\n",__FUNCTION__,__FILE__,__LINE__,""#func);_kill("",1);}
    
    
    _initfunc(vkCmdPipelineBarrier,vkcmdpipelinebarrier);
    _initfunc(vkCreateShaderModule,vkcreateshadermodule);
    _initfunc(vkCreateBuffer,vkcreatebuffer);
    _initfunc(vkGetBufferMemoryRequirements,vkgetbuffermemoryrequirements);
    _initfunc(vkMapMemory,vkmapmemory);
    _initfunc(vkUnmapMemory,vkunmapmemory);
    _initfunc(vkFlushMappedMemoryRanges,vkflushmappedmemoryranges);
    _initfunc(vkInvalidateMappedMemoryRanges,vkinvalidatemappedmemoryranges);
    _initfunc(vkBindBufferMemory,vkbindbuffermemory);
    _initfunc(vkDestroyBuffer,vkdestroybuffer);
    _initfunc(vkAllocateMemory,vkallocatememory);
    _initfunc(vkFreeMemory,vkfreememory);
    _initfunc(vkCreateRenderPass,vkcreaterenderpass);
    _initfunc(vkCmdBeginRenderPass,vkcmdbeginrenderpass);
    _initfunc(vkCmdEndRenderPass,vkcmdendrenderpass);
    _initfunc(vkCmdNextSubpass,vkcmdnextsubpass);
    _initfunc(vkCmdExecuteCommands,vkcmdexecutecommands);
    _initfunc(vkCreateImage,vkcreateimage);
    _initfunc(vkGetImageMemoryRequirements,vkgetimagememoryrequirements);
    _initfunc(vkCreateImageView,vkcreateimageview);
    _initfunc(vkDestroyImageView,vkdestroyimageview);
    _initfunc(vkBindImageMemory,vkbindimagememory);
    _initfunc(vkGetImageSubresourceLayout,vkgetimagesubresourcelayout);
    _initfunc(vkCmdCopyImage,vkcmdcopyimage);
    _initfunc(vkCmdBlitImage,vkcmdblitimage);
    _initfunc(vkDestroyImage,vkdestroyimage);
    _initfunc(vkCmdClearAttachments,vkcmdclearattachments);
    _initfunc(vkCmdCopyBuffer,vkcmdcopybuffer);
    _initfunc(vkCmdCopyBufferToImage,vkcmdcopybuffertoimage);
    _initfunc(vkCreateSampler,vkcreatesampler);
    _initfunc(vkDestroySampler,vkdestroysampler);
    _initfunc(vkCreateSemaphore,vkcreatesemaphore);
    _initfunc(vkDestroySemaphore,vkdestroysemaphore);
    _initfunc(vkCreateFence,vkcreatefence);
    _initfunc(vkDestroyFence,vkdestroyfence);
    _initfunc(vkWaitForFences,vkwaitforfences);
    _initfunc(vkResetFences,vkresetfences);
    _initfunc(vkCreateCommandPool,vkcreatecommandpool);
    _initfunc(vkDestroyCommandPool,vkdestroycommandpool);
    _initfunc(vkAllocateCommandBuffers,vkallocatecommandbuffers);
    _initfunc(vkBeginCommandBuffer,vkbegincommandbuffer);
    _initfunc(vkEndCommandBuffer,vkendcommandbuffer);
    _initfunc(vkGetDeviceQueue,vkgetdevicequeue);
    _initfunc(vkQueueSubmit,vkqueuesubmit);
    _initfunc(vkQueueWaitIdle,vkqueuewaitidle);
    _initfunc(vkDeviceWaitIdle,vkdevicewaitidle);
    _initfunc(vkCreateFramebuffer,vkcreateframebuffer);
    _initfunc(vkCreatePipelineCache,vkcreatepipelinecache);
    _initfunc(vkCreatePipelineLayout,vkcreatepipelinelayout);
    _initfunc(vkCreateGraphicsPipelines,vkcreategraphicspipelines);
    _initfunc(vkCreateComputePipelines,vkcreatecomputepipelines);
    _initfunc(vkCreateDescriptorPool,vkcreatedescriptorpool);
    _initfunc(vkCreateDescriptorSetLayout,vkcreatedescriptorsetlayout);
    _initfunc(vkAllocateDescriptorSets,vkallocatedescriptorsets);
    _initfunc(vkUpdateDescriptorSets,vkupdatedescriptorsets);
    _initfunc(vkCmdBindDescriptorSets,vkcmdbinddescriptorsets);
    _initfunc(vkCmdBindPipeline,vkcmdbindpipeline);
    _initfunc(vkCmdBindVertexBuffers,vkcmdbindvertexbuffers);
    _initfunc(vkCmdBindIndexBuffer,vkcmdbindindexbuffer);
    _initfunc(vkCmdSetViewport,vkcmdsetviewport);
    _initfunc(vkCmdSetScissor,vkcmdsetscissor);
    _initfunc(vkCmdSetLineWidth,vkcmdsetlinewidth);
    _initfunc(vkCmdSetDepthBias,vkcmdsetdepthbias);
    _initfunc(vkCmdPushConstants,vkcmdpushconstants);
    _initfunc(vkCmdDrawIndexed,vkcmddrawindexed);
    _initfunc(vkCmdDraw,vkcmddraw);
    _initfunc(vkCmdDrawIndexedIndirect,vkcmddrawindexedindirect);
    _initfunc(vkCmdDrawIndirect,vkcmddrawindirect);
    _initfunc(vkCmdDispatch,vkcmddispatch);
    _initfunc(vkDestroyPipeline,vkdestroypipeline);
    _initfunc(vkDestroyPipelineLayout,vkdestroypipelinelayout);
    _initfunc(vkDestroyDescriptorSetLayout,vkdestroydescriptorsetlayout);
    _initfunc(vkDestroyDevice,vkdestroydevice);
    
    _initfunc(vkDestroyDescriptorPool,vkdestroydescriptorpool);
    _initfunc(vkFreeCommandBuffers,vkfreecommandbuffers);
    _initfunc(vkDestroyRenderPass,vkdestroyrenderpass);
    _initfunc(vkDestroyFramebuffer,vkdestroyframebuffer);
    _initfunc(vkDestroyShaderModule,vkdestroyshadermodule);
    _initfunc(vkDestroyPipelineCache,vkdestroypipelinecache);
    _initfunc(vkCreateQueryPool,vkcreatequerypool);
    _initfunc(vkDestroyQueryPool,vkdestroyquerypool);
    _initfunc(vkGetQueryPoolResults,vkgetquerypoolresults);
    _initfunc(vkCmdBeginQuery,vkcmdbeginquery);
    _initfunc(vkCmdEndQuery,vkcmdendquery);
    _initfunc(vkCmdResetQueryPool,vkcmdresetquerypool);
    _initfunc(vkCmdCopyQueryPoolResults,vkcmdcopyquerypoolresults);
    
    
    _initfunc(vkCmdFillBuffer,vkcmdfillbuffer);
    _initfunc(vkAcquireNextImageKHR,vkacquirenextimagekhr);
    _initfunc(vkGetFenceStatus,vkgetfencestatus);
    _initfunc(vkCreateSwapchainKHR,vkcreateswapchainkhr);
    _initfunc(vkGetSwapchainImagesKHR,vkgetswapchainimageskhr);
    _initfunc(vkQueuePresentKHR,vkqueuepresentkhr);
    
    _initfunc(vkCmdClearColorImage,vkcmdclearcolorimage);
    
    _initfunc(vkCmdCopyImageToBuffer,vkcmdcopyimagetobuffer);
    
    _initfunc(vkGetPipelineCacheData,vkgetpipelinecachedata);

    _initfunc(vkGetImageSparseMemoryRequirements,vkgetimagesparsememoryrequirements);


    
    //vulkan 1.1 here
    if(VK_VERSION_MINOR(global_version_no) >= 1){
	    _dprint("%s","Loading vk 1.1 functions\n");
	    _initfunc(vkBindBufferMemory2,vkbindbuffermemory2);
	    _initfunc(vkBindImageMemory2,vkbindimagememory2);
	    _initfunc(vkGetBufferMemoryRequirements2,vkgetbuffermemoryrequirements2);
	    _initfunc(vkGetImageMemoryRequirements2,vkgetimagememoryrequirements2);
	    _initfunc(vkGetImageSparseMemoryRequirements2,vkgetimagesparsememoryrequirements2);

	    _initfunc(vkTrimCommandPool,vktrimcommandpool);
	    _initfunc(vkGetDescriptorLayoutSupport,vkgetdescriptorlayoutsupport);


	    _initfunc(vkDestroyDescriptorUpdateTemplate,vkdestroydescriptorupdatetemplate);
	    _initfunc(vkCreateDescriptorUpdateTemplate,vkcreatedescriptorupdatetemplate);
	    _initfunc(vkUpdateDescriptorSetWithTemplate,vkupdatedescriptorsetwithtemplate);


	    _deprecate_func(vkgetbuffermemoryrequirements);//MARK: should we just alias these??
	    _deprecate_func(vkgetimagememoryrequirements);
	    _deprecate_func(vkgetimagesparsememoryrequirements);
	   

	    //set internal functions
	    VGetBufferMemoryRequirements = 
		    _isdeprecated(vkGetBufferMemoryRequirements) ? VGetBufferMemoryRequirements2 : VGetBufferMemoryRequirements1;

	    VGetImageMemoryRequirements = 
		    _isdeprecated(vkGetImageMemoryRequirements) ? VGetImageMemoryRequirements2 : VGetImageMemoryRequirements1;
    }
}


#undef _initfunc
#undef _deprecate_func
//#undef _instproc
#undef _deviceproc
