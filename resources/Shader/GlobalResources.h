#ifndef INC_GLOBAL_RESOURCES_H
#define INC_GLOBAL_RESOURCES_H

// Keep in sync with RenderCore_PlatformDX12::InitRootSignatures()
// SRVs (t-register)
Texture1D 			theTextures1D[] 	: register(t0, space0);
Texture1D<uint4> 	theUintTextures1D[] : register(t0, space1);
Texture1D<int4> 	theIntTextures1D[] 	: register(t0, space2);
Texture2D 			theTextures2D[] 	: register(t0, space3);
Texture2D<uint4>	theUintTextures2D[] : register(t0, space4);
Texture2D<int4> 	theIntTextures2D[] 	: register(t0, space5);
Texture3D 			theTextures3D[] 	: register(t0, space6);
Texture3D<uint4>	theUintTextures3D[] : register(t0, space7);
Texture3D<int4> 	theIntTextures3D[] 	: register(t0, space8);
TextureCube			theCubemaps[]		: register(t0, space9);
TextureCube<uint4>	theUintCubemaps[]	: register(t0, space10);
TextureCube<int4>	theIntCubemaps[]	: register(t0, space11);
ByteAddressBuffer 	theBuffers[] 		: register(t0, space12);
#define Space_LocalBuffer space13

// UAVs (u-register)
RWTexture1D 		theRwTextures1D[] 		: register(u0, space0);
RWTexture1D<uint4> 	theRwUintTextures1D[] 	: register(u0, space1);
RWTexture1D<int4> 	theRwIntTextures1D[] 	: register(u0, space2);
RWTexture2D 		theRwTextures2D[] 		: register(u0, space3);
RWTexture2D<uint4>	theRwUintTextures2D[] 	: register(u0, space4);
RWTexture2D<int4> 	theRwIntTextures2D[] 	: register(u0, space5);
RWTexture3D 		theRwTextures3D[] 		: register(u0, space6);
RWTexture3D<uint4>	theRwUintTextures3D[] 	: register(u0, space7);
RWTexture3D<int4> 	theRwIntTextures3D[] 	: register(u0, space8);
RWByteAddressBuffer	theRwBuffers[] 			: register(u0, space9);
#define Space_LocalRWBuffer space10

// Sampler (s-register)
SamplerState theSamplers[] : register(s0, space0);

// Cbuffer (b-register)
#define Space_LocalCBuffer space0

#endif  // INC_GLOBAL_RESOURCES_H

