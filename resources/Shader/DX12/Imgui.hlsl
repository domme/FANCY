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

VK_BINDING(0, 0) ConstantBuffer<IMGUI_VS_CBUFFER> cbVSImgui : register(b0);

#define ROOT_SIGNATURE "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), CBV(b0), DescriptorTable(SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_PIXEL), StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_CLAMP, filter = FILTER_MIN_MAG_MIP_LINEAR )"

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
  VK_BINDING(0, 1) Texture2D texture0 : register(t0);
  VK_BINDING(1, 1) SamplerState sampler_default : register(s0);
 
  [RootSignature(ROOT_SIGNATURE)]
  float4 main(VS_OUT input) : SV_Target
  {
    float4 texCol = texture0.Sample(sampler_default, input.uv) * cbVSImgui.ProjectionMatrix[0];
    float4 out_col = input.col * texCol;
    return out_col;
  }
#endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//