#include "Common.hlsl"
#include "GlobalResources.h"

cbuffer Constants : register(b0, Space_LocalRootSig_LocalCbuffer)
{
  uint myOutTexIndex;
};

[shader("raygeneration")] 
void RayGen() {
  // Initialize the ray payload
  HitInfo payload;
  payload.colorAndDistance = float4(0.9, 0.6, 0.2, 1);

  // Get the location within the dispatched 2D grid of work items
  // (often maps to pixels, so this could represent a pixel coordinate).
  uint2 launchIndex = DispatchRaysIndex().xy;

  theRwTextures2D[myOutTexIndex][launchIndex] = float4(payload.colorAndDistance.rgb, 1.f);
}
