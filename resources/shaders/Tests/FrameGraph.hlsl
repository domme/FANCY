#include "fancy/resources/shaders/GlobalResources.h"

// Shaders of the FrameGraph test (Tests/Test_FrameGraph.cpp):
// - STORE_COLORS_CS: writes the quadrant colors into a buffer through a UAV
// - FILL_QUADRANT_CS: fills a rectangle of a texture through a UAV, with its color read from the buffer
// - FILL_COLOR: fullscreen triangle with its color read from the buffer. The rectangle is set through viewport and clip rect.
// - BLIT: fullscreen triangle sampling a texture. myDisplayMode selects what is shown (see below).
// - DOWNSAMPLE_CS: 2x2 box filter from the SRV of mip n-1 into the UAV of mip n
// - FILL_STENCIL: fullscreen triangle without color output, for drawing a stencil pattern with the stencil state
// - COPY_STENCIL_CS: copies the stencil plane of a depth-stencil texture into an R8_UINT texture

cbuffer CB0 : register(b0, Space_LocalCBuffer)
{
  float4 myColors[4];   // STORE_COLORS_CS
  uint2 myOffset;       // FILL_QUADRANT_CS: first texel of the rectangle
  uint myTextureIndex;  // FILL_QUADRANT_CS: UAV, BLIT: SRV
  uint mySamplerIndex;  // BLIT
  float2 myPixelToUv;   // BLIT
  uint myBufferIndex;   // STORE_COLORS_CS: UAV, FILL_*: SRV
  uint myQuadrantIndex; // FILL_*
  uint2 myDstSize;      // DOWNSAMPLE_CS, COPY_STENCIL_CS
  uint mySrcIndex;      // DOWNSAMPLE_CS: SRV, COPY_STENCIL_CS: uint SRV
  uint myDstIndex;      // DOWNSAMPLE_CS: UAV, COPY_STENCIL_CS: uint UAV
  uint myDisplayMode;   // BLIT: 0 = texture, 1 = mip level, 2 = stencil, 3 = depth
  uint myMipLevel;      // BLIT mode 1
  uint2 myDisplayOrigin;// BLIT: first pixel of the rectangle
  float myDisplayScale; // BLIT modes 1-3: texels per pixel
};

//---------------------------------------------------------------------------//
#if defined(STORE_COLORS_CS)

[numthreads(4, 1, 1)]
void main(uint3 aDTid : SV_DispatchThreadID)
{
  theRwBuffers[myBufferIndex].Store<float4>(aDTid.x * 16, myColors[aDTid.x]);
}

#endif // STORE_COLORS_CS
//---------------------------------------------------------------------------//
#if defined(FILL_QUADRANT_CS)

[numthreads(8, 8, 1)]
void main(uint3 aDTid : SV_DispatchThreadID)
{
  theRwTextures2D[myTextureIndex][myOffset + aDTid.xy] = theBuffers[myBufferIndex].Load<float4>(myQuadrantIndex * 16);
}

#endif // FILL_QUADRANT_CS
//---------------------------------------------------------------------------//
#if defined(DOWNSAMPLE_CS)

[numthreads(8, 8, 1)]
void main(uint3 aDTid : SV_DispatchThreadID)
{
  if (any(aDTid.xy >= myDstSize))
    return;

  int2 src = (int2) aDTid.xy * 2;
  float4 sum = theTextures2D[mySrcIndex][src] + theTextures2D[mySrcIndex][src + int2(1, 0)]
             + theTextures2D[mySrcIndex][src + int2(0, 1)] + theTextures2D[mySrcIndex][src + int2(1, 1)];
  theRwTextures2D[myDstIndex][aDTid.xy] = sum * 0.25;
}

#endif // DOWNSAMPLE_CS
//---------------------------------------------------------------------------//
#if defined(COPY_STENCIL_CS)

[numthreads(8, 8, 1)]
void main(uint3 aDTid : SV_DispatchThreadID)
{
  if (any(aDTid.xy >= myDstSize))
    return;

#ifdef VULKAN
  uint stencil = theUintTextures2D[mySrcIndex][aDTid.xy].r;
#else
  uint stencil = theUintTextures2D[mySrcIndex][aDTid.xy].g;
#endif
  theRwUintTextures2D[myDstIndex][aDTid.xy] = uint4(stencil, 0, 0, 0);
}

#endif // COPY_STENCIL_CS
//---------------------------------------------------------------------------//
#if defined(FILL_COLOR) || defined(BLIT) || defined(FILL_STENCIL)

struct VS_OUT
{
  float4 pos : SV_POSITION;
};

#if defined(PROGRAM_TYPE_VERTEX)

struct VS_IN
{
  float2 position : POSITION;
};

VS_OUT main(VS_IN v)
{
  VS_OUT vs_out = (VS_OUT)0;
  vs_out.pos = float4(v.position.x, v.position.y, 1, 1);
  return vs_out;
}

#endif // PROGRAM_TYPE_VERTEX

#if defined(PROGRAM_TYPE_FRAGMENT)

#if defined(FILL_STENCIL)
// No color output: only the stencil state writes
void main(VS_OUT fs_in)
{
}
#else
float4 main(VS_OUT fs_in) : SV_TARGET
{
#if defined(FILL_COLOR)
  return theBuffers[myBufferIndex].Load<float4>(myQuadrantIndex * 16);
#else
  float2 localPixel = fs_in.pos.xy - float2(myDisplayOrigin);
  int2 texel = (int2) (localPixel * myDisplayScale);
  if (myDisplayMode == 1)
  {
    return float4(theTextures2D[myTextureIndex].Load(int3(texel, myMipLevel)).rgb, 1.0f);
  }
  else if (myDisplayMode == 2)
  {
    float v = theUintTextures2D[myTextureIndex].Load(int3(texel, 0)).r / 255.0f;
    return float4(v, v, v, 1.0f);
  }
  else if (myDisplayMode == 3)
  {
    float d = theTextures2D[myTextureIndex].Load(int3(texel, 0)).r;
    return float4(d, d, d, 1.0f);
  }
  SamplerState linearClampSampler = theSamplers[mySamplerIndex];
  float2 uv = myPixelToUv * localPixel;
  return float4(theTextures2D[myTextureIndex].SampleLevel(linearClampSampler, uv, 0).rgb, 1.0f);
#endif
}
#endif // FILL_STENCIL

#endif // PROGRAM_TYPE_FRAGMENT

#endif // FILL_COLOR || BLIT || FILL_STENCIL
//---------------------------------------------------------------------------//
