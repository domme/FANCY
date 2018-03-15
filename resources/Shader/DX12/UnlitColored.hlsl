
#define ROOT_SIGNATURE  "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT )," \
                        "CBV(b0), " \
                        "DescriptorTable(SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_PIXEL)," \
                        "StaticSampler(s0, " \
                        "addressU = TEXTURE_ADDRESS_WRAP, " \
                        "addressV = TEXTURE_ADDRESS_WRAP, " \
                        "filter = FILTER_MIN_MAG_MIP_POINT )"

  struct CBUFFER
  {
    float4x4 c_WorldViewProjectionMatrix; 
  };

  ConstantBuffer<CBUFFER> cbPerObject : register(b0);

  struct VS_OUT
  {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD2;
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
      vs_out.uv = v.texcoord0;
      return vs_out;
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_FRAGMENT)  
    Texture2D tex_diffuse : register(t0);
    SamplerState sampler_default : register(s0);

    [RootSignature(ROOT_SIGNATURE)]
    float4 main(VS_OUT fs_in) : SV_TARGET
    {
      return tex_diffuse.Sample(sampler_default, fs_in.uv);
    }
  #endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  
