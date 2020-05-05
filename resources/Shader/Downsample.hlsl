#include "Vulkan_Support.h"

VK_BINDING_SET(0, 0) cbuffer CB0 : register(b0)
{
  int2 mySrcTextureSize;
  int myIsSRGB;   
};

VK_BINDING_SET(1, 0) Texture2D<float4> SrcTexture : register(t0);
VK_BINDING_SET(2, 0) RWTexture2D<float4> DestTexture: register(u0);

[RootSignature("CBV(b0), DescriptorTable(SRV(t0), UAV(u0))")]
[numthreads(8, 8, 1)]
void main(uint3 aDTid : SV_DispatchThreadID)
{
    int2 destTexel = (int2) aDTid.xy;
    int2 srcTexel = destTexel * 2;

    float4 outCol = float4(0,0,0,0);

    int2 offsets[] = {
      int2(0, 0),
      int2(1, 0),
      int2(0, 1),
      int2(1, 1)
    };

    for (int i = 0; i < 4; ++i)
    {
      int2 texel = srcTexel + offsets[i];
      if (texel.x > mySrcTextureSize.x - 1)
        texel.x = mySrcTextureSize.x - 1 - texel.x;
      if (texel.y > mySrcTextureSize.y - 1)
        texel.y = mySrcTextureSize.y - 1 - texel.y;

      outCol += SrcTexture[texel];
    }

    outCol *= 0.25;

    if (myIsSRGB)
      outCol = pow(outCol, 1.0 / 2.2);

    DestTexture[destTexel] = outCol;
}