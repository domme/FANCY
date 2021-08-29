#ifndef INC_GLOBAL_RESOURCES_H
#define INC_GLOBAL_RESOURCES_H

#include "Vulkan_Support.h"

// Keep in sync with RootSignatureDX12.cpp and PipelineLayoutVk.cpp

// Local buffers. 
// Placed in different spaces so that DXC automatically assigns them to different descriptor sets
// without having to specify any [[vk::binding()]] at every usage.
#define Space_LocalBuffer space0
#define Space_LocalRWBuffer space1
#define Space_LocalCBuffer space2

#define Space_LocalRootSig_LocalCbuffer space100

// Global resources. Will be placed in descriptor set 3 in vulkan
// SRVs (t-register)
VK_BINDING_SET(0, 3)	Texture1D<float4>				theTextures1D[] 				: register(t0, space3);
VK_BINDING_SET(1, 3)	Texture1D<uint4> 				theUintTextures1D[] 			: register(t0, space4);
VK_BINDING_SET(2, 3)	Texture1D<int4> 				theIntTextures1D[] 				: register(t0, space5);
VK_BINDING_SET(3, 3)	Texture2D<float4> 				theTextures2D[] 				: register(t0, space6);
VK_BINDING_SET(4, 3)	Texture2D<uint4>				theUintTextures2D[] 			: register(t0, space7);
VK_BINDING_SET(5, 3)	Texture2D<int4> 				theIntTextures2D[] 				: register(t0, space8);
VK_BINDING_SET(6, 3)	Texture3D<float4> 				theTextures3D[] 				: register(t0, space9);
VK_BINDING_SET(7, 3)	Texture3D<uint4>				theUintTextures3D[] 			: register(t0, space10);
VK_BINDING_SET(8, 3)	Texture3D<int4> 				theIntTextures3D[] 				: register(t0, space11);
VK_BINDING_SET(9, 3)	TextureCube<float4>				theCubemaps[]					: register(t0, space12);
VK_BINDING_SET(10, 3)	TextureCube<uint4>				theUintCubemaps[]				: register(t0, space13);
VK_BINDING_SET(11, 3)	TextureCube<int4>				theIntCubemaps[]				: register(t0, space14);
VK_BINDING_SET(12, 3)	ByteAddressBuffer 				theBuffers[] 					: register(t0, space15);
VK_BINDING_SET(13, 3)	RaytracingAccelerationStructure theRtAccelerationStructures[] 	: register(t0, space16);

// UAVs (u-register)
VK_BINDING_SET(13, 3)	RWTexture1D<float4> theRwTextures1D[] 		: register(u0, space3);
VK_BINDING_SET(14, 3)	RWTexture1D<uint4> 	theRwUintTextures1D[] 	: register(u0, space4);
VK_BINDING_SET(15, 3)	RWTexture1D<int4> 	theRwIntTextures1D[] 	: register(u0, space5);
VK_BINDING_SET(16, 3)	RWTexture2D<float4> theRwTextures2D[] 		: register(u0, space6);
VK_BINDING_SET(17, 3)	RWTexture2D<uint4>	theRwUintTextures2D[] 	: register(u0, space7);
VK_BINDING_SET(18, 3)	RWTexture2D<int4> 	theRwIntTextures2D[] 	: register(u0, space8);
VK_BINDING_SET(19, 3)	RWTexture3D<float4> theRwTextures3D[] 		: register(u0, space9);
VK_BINDING_SET(20, 3)	RWTexture3D<uint4>	theRwUintTextures3D[] 	: register(u0, space10);
VK_BINDING_SET(21, 3)	RWTexture3D<int4> 	theRwIntTextures3D[] 	: register(u0, space11);
VK_BINDING_SET(22, 3)	RWByteAddressBuffer	theRwBuffers[] 			: register(u0, space12);

// Sampler (s-register)
VK_BINDING_SET(23, 3) SamplerState theSamplers[] : register(s0, space3);

#endif  // INC_GLOBAL_RESOURCES_H