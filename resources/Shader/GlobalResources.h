#ifndef INC_GLOBAL_RESOURCES_H
#define INC_GLOBAL_RESOURCES_H

// All local resources will be bound through this descriptor set
#define DescSet_Local 1 

// Keep in sync with RootSignatureDX12.cpp
// SRVs (t-register)
[[vk::binding(0, 0)]]	Texture1D<float4>	theTextures1D[] 	: register(t0, space0);
[[vk::binding(1, 0)]]	Texture1D<uint4> 	theUintTextures1D[] : register(t0, space1);
[[vk::binding(2, 0)]]	Texture1D<int4> 	theIntTextures1D[] 	: register(t0, space2);
[[vk::binding(3, 0)]]	Texture2D<float4> 	theTextures2D[] 	: register(t0, space3);
[[vk::binding(4, 0)]]	Texture2D<uint4>	theUintTextures2D[] : register(t0, space4);
[[vk::binding(5, 0)]]	Texture2D<int4> 	theIntTextures2D[] 	: register(t0, space5);
[[vk::binding(6, 0)]]	Texture3D<float4> 	theTextures3D[] 	: register(t0, space6);
[[vk::binding(7, 0)]]	Texture3D<uint4>	theUintTextures3D[] : register(t0, space7);
[[vk::binding(8, 0)]]	Texture3D<int4> 	theIntTextures3D[] 	: register(t0, space8);
[[vk::binding(9, 0)]]	TextureCube<float4>	theCubemaps[]		: register(t0, space9);
[[vk::binding(10, 0)]]	TextureCube<uint4>	theUintCubemaps[]	: register(t0, space10);
[[vk::binding(11, 0)]]	TextureCube<int4>	theIntCubemaps[]	: register(t0, space11);
[[vk::binding(12, 0)]]	ByteAddressBuffer 	theBuffers[] 		: register(t0, space12);
#define Space_LocalBuffer space13

// UAVs (u-register)
[[vk::binding(13, 0)]]	RWTexture1D<float4> theRwTextures1D[] 		: register(u0, space0);
[[vk::binding(14, 0)]]	RWTexture1D<uint4> 	theRwUintTextures1D[] 	: register(u0, space1);
[[vk::binding(15, 0)]]	RWTexture1D<int4> 	theRwIntTextures1D[] 	: register(u0, space2);
[[vk::binding(16, 0)]]	RWTexture2D<float4> theRwTextures2D[] 		: register(u0, space3);
[[vk::binding(17, 0)]]	RWTexture2D<uint4>	theRwUintTextures2D[] 	: register(u0, space4);
[[vk::binding(18, 0)]]	RWTexture2D<int4> 	theRwIntTextures2D[] 	: register(u0, space5);
[[vk::binding(19, 0)]]	RWTexture3D<float4> theRwTextures3D[] 		: register(u0, space6);
[[vk::binding(20, 0)]]	RWTexture3D<uint4>	theRwUintTextures3D[] 	: register(u0, space7);
[[vk::binding(21, 0)]]	RWTexture3D<int4> 	theRwIntTextures3D[] 	: register(u0, space8);
[[vk::binding(22, 0)]]	RWByteAddressBuffer	theRwBuffers[] 			: register(u0, space9);
#define Space_LocalRWBuffer space10

// Sampler (s-register)
[[vk::binding(23, 0)]] SamplerState theSamplers[] : register(s0, space0);

// Cbuffer (b-register)
#define Space_LocalCBuffer space0

#endif  // INC_GLOBAL_RESOURCES_H

