#include "Common.hlsl"
#include "GlobalResources.h"

/*
cbuffer Constants : register(b0, space0)
{
  uint4 myOutTexIndex;
};

RWTexture2D<float4> theRwTextures2D[] : register(u0, space1);
*/

cbuffer Constants : register(b0, Space_LocalCBuffer)
{
  uint4 myOutTexIndex;
};

[shader("raygeneration")] 
void RayGen() {
  // Initialize the ray payload
  HitInfo payload;
  payload.colorAndDistance = float4(0.9, 0.6, 0.2, 1);

  // Get the location within the dispatched 2D grid of work items
  // (often maps to pixels, so this could represent a pixel coordinate).
  uint2 launchIndex = DispatchRaysIndex().xy;

  // This works
  // uint4 texIdxData = theBuffers[0].Load<uint4>(0);
  // theRwTextures2D[NonUniformResourceIndex(texIdxData.x)][launchIndex] = float4(payload.colorAndDistance.rgb, 1.f);

  // Doesn't work
  theRwTextures2D[myOutTexIndex.x][launchIndex] = float4(payload.colorAndDistance.rgb, 1.f);
}
