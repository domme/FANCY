
#define RS "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT )," \
           "DescriptorTable(SRV(t0)), " \
           "DescriptorTable(Sampler(s0))"

struct VS_OUT
{
  float4 pos : SV_POSITION;
  float2 uv : TEXCOORD0;
};
//---------------------------------------------------------------------------//
#if defined(PROGRAM_TYPE_VERTEX)

struct VS_IN
{
  float3 position : POSITION;
};

[RootSignature(RS)]
VS_OUT main(VS_IN v)
{
  VS_OUT vs_out = (VS_OUT)0;
  vs_out.pos = float4(v.position.xy, 0.0f, 1.0f);
  vs_out.uv = v.position.xy * 0.5 + 0.5;

  return vs_out;
}

#endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
#if defined(PROGRAM_TYPE_FRAGMENT)

[RootSignature(RS)]
float4 main(VS_OUT fs_in) : SV_TARGET
{
  return float4(1.0, 1.0, 1.0, 1.0);
}

Texture2D tex : register(t0);
SamplerState sampler_default : register(s0);

[RootSignature(RS)]
float4 main_textured(VS_OUT fs_in) : SV_TARGET
{
  float3 color = tex.SampleLevel(sampler_default, fs_in.uv, 0).xyz;
  return float4(color, 1.0);
}

#endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//