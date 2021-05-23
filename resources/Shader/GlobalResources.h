#ifndef INC_BINDLESS_H
#define INC_BINDLESS_H

#include "RootSignature.h"

Texture2D global_texture2D[] : register(t0, _TexSpace);
RWTexture2D global_rw_texture2D[] : register(u0, _RWTexSpace);
ByteAddressBuffer global_buffer[] : register(t0, _BufSpace);
RWByteAddressBuffer global_rw_buffer[] : register(u0, _RWBufSpace);
SamplerState global_sampler[] : register(s0, _SamplerSpace);

#endif  // INC_BINDLESS_H

