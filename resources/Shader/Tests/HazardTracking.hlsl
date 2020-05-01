#include "Vulkan_Support.h"

#if defined(BUFFER_TO_TEXTURE_MIP)

VK_BINDING_SET(0, 0) Buffer<uint> SrcBuffer : register(t0);
VK_BINDING_SET(1, 0) RWTexture2D<uint> DstTexture : register(u0);

[RootSignature("DescriptorTable(SRV(t0), UAV(u0))")]
[numthreads(8, 8, 1)]
void main(uint3 aDTid : SV_DispatchThreadID)
{
    uint val = SrcBuffer[0];
    DstTexture[aDTid.xy] = val + 1;
}

#endif

#if defined(TEXUTRE_MIP_TO_BUFFER)

VK_BINDING_SET(0, 0) Texture2D<uint> SrcTexture : register(t0);
VK_BINDING_SET(1, 0) RWBuffer<uint> DstBuffer : register(u0);

[RootSignature("DescriptorTable(SRV(t0), UAV(u0))")]
[numthreads(8, 8, 1)]
void main(uint3 aDTid : SV_DispatchThreadID)
{
    uint val = SrcTexture[aDTid.xy];
    DstBuffer[0] = val + 1;
}

#endif
