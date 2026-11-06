#include "fancy/resources/shaders/GlobalResources.h"

// Shaders of the FrameGraph test (Tests/Test_FrameGraph.cpp):
// - STORE_COLORS_CS: writes the quadrant colors into a buffer through a UAV
// - FILL_QUADRANT_CS: fills a rectangle of a texture through a UAV, with its color read from the buffer
// - FILL_COLOR: fullscreen triangle with its color read from the buffer. The rectangle is set through viewport and clip rect.
// - BLIT: fullscreen triangle sampling a texture

cbuffer CB0 : register(b0, Space_LocalCBuffer)
{
  float4 myColors[4];   // STORE_COLORS_CS
  uint2 myOffset;       // FILL_QUADRANT_CS: first texel of the rectangle
  uint myTextureIndex;  // FILL_QUADRANT_CS: UAV, BLIT: SRV
  uint mySamplerIndex;  // BLIT
  float2 myPixelToUv;   // BLIT
  uint myBufferIndex;   // STORE_COLORS_CS: UAV, FILL_*: SRV
  uint myQuadrantIndex; // FILL_*
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
#if defined(FILL_COLOR) || defined(BLIT)

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

float4 main(VS_OUT fs_in) : SV_TARGET
{
#if defined(FILL_COLOR)
  return theBuffers[myBufferIndex].Load<float4>(myQuadrantIndex * 16);
#else
  SamplerState linearClampSampler = theSamplers[mySamplerIndex];
  float2 uv = myPixelToUv * fs_in.pos.xy;
  return float4(theTextures2D[myTextureIndex].SampleLevel(linearClampSampler, uv, 0).rgb, 1.0f);
#endif
}

#endif // PROGRAM_TYPE_FRAGMENT

#endif // FILL_COLOR || BLIT
//---------------------------------------------------------------------------//
