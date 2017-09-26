 #include "inc_ConstantBuffers.hlsl"

#define ROOT_SIGNATURE  "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT )," \
                        "CBV(b0)"

ConstantBuffer<PER_OBJECT> cbPerObject : register(b0);

  struct VS_OUT
  {
    float4 pos : SV_POSITION;
  };

//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_VERTEX)
    struct VS_IN
    {
      float3 position : POSITION;
      float3 normal : NORMAL;
      float3 tangent : TANGENT;
      float3 bitangent : BINORMAL;
      float2 texcoord0 : TEXCOORD0;
    };
    
    [RootSignature(ROOT_SIGNATURE)]
    VS_OUT main(VS_IN v)
    {
      VS_OUT vs_out = (VS_OUT)0;
      vs_out.pos = mul(cbPerObject.c_WorldViewProjectionMatrix, float4(v.position, 1.0f));
      return vs_out;
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_FRAGMENT)  

    [RootSignature(ROOT_SIGNATURE)]
    float4 main(VS_OUT fs_in) : SV_TARGET
    {
      return float4(0.0, 0.0, 0.0, 1.0);
    }
  #endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  
