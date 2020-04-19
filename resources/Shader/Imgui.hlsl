#include "Vulkan_Support.h"

struct VS_OUT
{
  float4 pos : SV_POSITION;
  float2 uv  : TEXCOORD0;
  float4 col : COLOR0;
};

struct IMGUI_VS_CBUFFER
{
  float4x4 ProjectionMatrix;
};

VK_BINDING_SET(0, 0) ConstantBuffer<IMGUI_VS_CBUFFER> cbVSImgui : register(b0);

#define ROOT_SIGNATURE "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), \
                        CBV(b0), \
                        DescriptorTable(SRV(t0)), \
                        DescriptorTable(Sampler(s0))"

//---------------------------------------------------------------------------//
#if defined(PROGRAM_TYPE_VERTEX)
    struct VS_IN
    {
      float2 pos : POSITION;
      float2 uv  : TEXCOORD0;
      uint   col : COLOR0;
    };

    float4 UnpackColor(uint aColorU)
    {
      float4 rgba = float4((aColorU & 0x000000FF) * (1.0f / 255.0f),
                           ((aColorU & 0x0000FF00) >> 8) * (1.0f / 255.0f),
                           ((aColorU & 0x00FF0000) >> 16) * (1.0f / 255.0f),
                           ((aColorU & 0xFF000000) >> 24) * (1.0f / 255.0f));
      return rgba;
    }
    
    [RootSignature(ROOT_SIGNATURE)]
    VS_OUT main(VS_IN input)
    {
      VS_OUT output = (VS_OUT)0;
      output.pos = mul( cbVSImgui.ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
      output.col = UnpackColor(input.col);
      output.uv  = input.uv;
      return output;
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
#if defined(PROGRAM_TYPE_FRAGMENT)  
  VK_BINDING_SET(1, 0) Texture2D texture0 : register(t0);
  VK_BINDING_SET(2, 0) SamplerState sampler_default : register(s0);
 
  [RootSignature(ROOT_SIGNATURE)]
  void main(VS_OUT input, out float4 colorOut : SV_Target0)
  {
    float2 uv = input.uv;
    float4 tex = texture0.Sample(sampler_default, uv);
    tex.xyz *= tex.w;
    colorOut = input.col * tex;
  }
#endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//