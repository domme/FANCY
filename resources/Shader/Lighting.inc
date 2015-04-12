#if !defined (LIGHTING_INCLUDE_)
#define LIGHTING_INCLUDE_

vec3 ShadeDirectionalLight(vec4 _dirlightParams, vec4 _materialLightingParams, vec3 _lightDirWS, vec3 _lightColorIntensity, vec3 _posWS, vec3 _normalWS)
{
  return _lightColorIntensity * _materialLightingParams.x * max(0, dot(-_lightDirWS, _normalWS));
}

#endif  // LIGHTING_INCLUDE_