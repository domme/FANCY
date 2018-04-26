
#define ROOT_SIGNATURE  "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT )," \
                        "CBV(b0)"

struct CBUFFER
{
  float4x4 c_WorldViewProjectionMatrix;
  float4 c_Color;
};

ConstantBuffer<CBUFFER> cbPerObject : register(b0);

struct VS_OUT
{
  float4 pos : SV_POSITION;
  float4 color : COLOR;
};
//---------------------------------------------------------------------------//
#if defined(PROGRAM_TYPE_VERTEX)
struct VS_IN
{
  float3 position : POSITION;
  float4 color : COLOR;
};

[RootSignature(ROOT_SIGNATURE)]
VS_OUT main(VS_IN v)
{
  VS_OUT vs_out = (VS_OUT)0;
  vs_out.pos = mul(cbPerObject.c_WorldViewProjectionMatrix, float4(v.position, 1.0f));
  vs_out.color = v.color;
  return vs_out;
}
#endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
#if defined(PROGRAM_TYPE_FRAGMENT)  

[RootSignature(ROOT_SIGNATURE)]
float4 main(VS_OUT fs_in) : SV_TARGET
{
  return fs_in.color * cbPerObject.c_Color;
}
#endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------// 