#include "GlobalResources.h"

[[vk::binding(0, DescSet_Local)]] 
cbuffer CB0 : register(b0, Space_LocalCBuffer)
{
  int2 mySrcTextureSize;
  int myIsSRGB;   
  int mySrcTextureIdx;
  int myDstTextureIdx;
};

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
      if (texel.x >= mySrcTextureSize.x )
        texel.x = mySrcTextureSize.x - 1 - texel.x;
      if (texel.y >= mySrcTextureSize.y )
        texel.y = mySrcTextureSize.y - 1 - texel.y;

      outCol += theTextures2D[mySrcTextureIdx][texel];
    }

    outCol *= 0.25;

    if (myIsSRGB)
      outCol = pow(outCol, 1.0 / 2.2);

    theRwTextures2D[myDstTextureIdx][destTexel] = outCol;
}