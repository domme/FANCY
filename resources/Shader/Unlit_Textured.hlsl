#include "GlobalResources.h"

[[vk::binding(0, DescSet_Local)]] 
cbuffer Constants : register(b0, Space_LocalCBuffer)
{
  float4x4 myWVP;
  uint myTextureIndex;
  uint mySamplerIndex;
};

struct VS_OUT
{
  float4 pos : SV_POSITION;
  float2 uv : TEXCOORD2;
};
//---------------------------------------------------------------------------//
#if defined(PROGRAM_TYPE_VERTEX)
struct VS_IN
{
  float3 position : POSITION0;
  float2 texcoord0 : TEXCOORD0;
#if defined(INSTANCED)
  float3 instance_position : POSITION1;
#endif
};

VS_OUT main(VS_IN v)
{
  VS_OUT vs_out = (VS_OUT)0;
  float3 pos = v.position;
#if defined(INSTANCED)
  pos += v.instance_position;
#endif
  vs_out.pos = mul(myWVP, float4(pos, 1.0f));
  vs_out.uv = v.texcoord0;
  return vs_out;
}
#endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
#if defined(PROGRAM_TYPE_FRAGMENT)  
float4 main(VS_OUT fs_in) : SV_TARGET
{
  if (myTextureIndex != 0xFFFFFFFF)
    return theTextures2D[myTextureIndex].Sample(theSamplers[mySamplerIndex], fs_in.uv);
  else
    return float4(0,0,0,0);
}
#endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  
