#if !defined (LIGHTING_INCLUDE_)
#define LIGHTING_INCLUDE_

float3 ShadeLight(float4 _lightParams, float3 _lightDirWS, float3 _posWS, float3 _normalWS)
{
  float nl = max(0, dot(_lightDirWS, _normalWS));
  return _lightParams.xyz * nl;
}

#endif  // LIGHTING_INCLUDE_