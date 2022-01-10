#include "Common.hlsl"
#include "GlobalResources.h"

cbuffer Constants : register(b0, Space_LocalCBuffer)
{
  float3 myCamCenter;
  bool myIsBGR;

  float4 myPixelToWorldScaleOffset;

  uint myOutTexIndex;
  uint myAsIndex;
};

[shader("raygeneration")] 
void RayGen() 
{
  uint2 pixel = DispatchRaysIndex().xy;
  float2 p = (float2(pixel) / DispatchRaysDimensions().xy) * 2 - 1; // -0.5 .. 0.5
  p *= 2;
  
  RayDesc rayDesc;
  rayDesc.Origin = float3(p.x, p.y, -5);
  rayDesc.TMin = 0.0;
  rayDesc.Direction = float3(0, 0, 1);
  rayDesc.TMax = 10.0;  

  HitInfo payload;
  payload.colorAndDistance = float4(0, 0, 0, 0);

  TraceRay( theRtAccelerationStructures[myAsIndex],
            0,
            0xFF,
            0,
            0,
            0,
            rayDesc,
            payload);

  theRwTextures2D[myOutTexIndex][pixel] = float4(myIsBGR ? payload.colorAndDistance.bgr : payload.colorAndDistance.rgb, 1.f);
}
