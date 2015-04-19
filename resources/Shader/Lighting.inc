#if !defined (LIGHTING_INCLUDE_)
#define LIGHTING_INCLUDE_

vec3 ShadeLight(vec4 _lightParams, vec3 _lightDirWS, vec3 _posWS, vec3 _normalWS)
{
  float nl = max(0, dot(_lightDirWS, _normalWS));
  return _lightParams.xyz * nl;
}

#endif  // LIGHTING_INCLUDE_