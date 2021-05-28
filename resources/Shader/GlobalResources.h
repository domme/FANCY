#ifndef INC_GLOBAL_RESOURCES_H
#define INC_GLOBAL_RESOURCES_H

#include "RootSignature.h"

Texture2D global_textures2D[] : register(t0, _TexSpace);
RWTexture2D<float4> global_rw_textures2D[] : register(u0, _RWTexSpace);
ByteAddressBuffer global_buffers[] : register(t0, _BufSpace);
RWByteAddressBuffer global_rw_buffers[] : register(u0, _RWBufSpace);
SamplerState global_samplers[] : register(s0, _SamplerSpace);

#endif  // INC_GLOBAL_RESOURCES_H

