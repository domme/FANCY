#ifndef INC_BINDLESS_H
#define INC_BINDLESS_H

#include "RootSignature.h"

Texture2D bindless_textures[] : register(t0, _TexSpace);
RWTexture2D bindless_rw_textures[] : register(u0, _RWTexSpace);
ByteAddressBuffer bindless_buffers[] : register(t0, _BufSpace);
RWByteAddressBuffer bindless_rw_buffers[] : register(u0, _RWBufSpace);
Sampler bindless_samplers[] : register(s0, _SamplerSpace);

#endif  // INC_BINDLESS_H

