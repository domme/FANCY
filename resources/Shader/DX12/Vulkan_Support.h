#ifndef VULKAN_SUPPORT_H
#define VULKAN_SUPPORT_H

#ifdef VULKAN
	#define VK_BINDING(aBinding, aSet) [[vk::binding(aBinding, aSet)]]
#else
	#define VK_BINDING(aBinding, aSet) 
#endif

#endif  // VULKAN_SUPPORT_H

