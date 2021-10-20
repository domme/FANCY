#include "Common.hlsl"
#include "GlobalResources.h"

cbuffer Constants : register(b0, Space_LocalCBuffer)
{
  float3 myCamCenter;
  float _pad;

  float4 myPixelToWorldScaleOffset;

  uint myOutTexIndex;
  uint myAsIndex;
};

[shader("raygeneration")] 
void RayGen() 
{
  uint2 pixel = DispatchRaysIndex().xy;
  float2 normPixel = (float2(pixel) / DispatchRaysDimensions().xy) * 2 - 1; // -0.5 .. 0.5
  float2 p = normPixel * 2;

  RayDesc rayDesc;
  rayDesc.Origin = myCamCenter + float3(p.x, p.y, 0);
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

  theRwTextures2D[myOutTexIndex][pixel] = float4(payload.colorAndDistance.rgb, 1.f);
}
