
 #define RS_EMPTY "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT )"

#define RS_TEXTURED "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT )," \
                    "DescriptorTable(SRV(t0)), " \
                    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, addressV = TEXTURE_ADDRESS_CLAMP, filter = FILTER_MIN_MAG_MIP_LINEAR)"

  struct VS_OUT
  {
    float4 pos : SV_POSITION;
  };
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_VERTEX)
    struct VS_IN
    {
      float3 position : POSITION;
    };
    
    [RootSignature(RS_EMPTY)]
    VS_OUT main(VS_IN v)
    {
      VS_OUT vs_out = (VS_OUT)0;
      vs_out.pos = float4(v.position.xy, 0.0f, 1.0f);

      return vs_out;
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_FRAGMENT)
    
    [RootSignature(RS_EMPTY)]
    float4 main(VS_OUT fs_in) : SV_TARGET
    {
      return float4(1.0, 1.0, 1.0, 1.0);
    }

    Texture2D tex : register(t0);
    SamplerState sampler_default : register(s0);

    [RootSignature(RS_TEXTURED)]
    float4 main_textured(VS_OUT fs_in) : SV_TARGET
    {
      float2 uv = fs_in.pos.xy * 0.5 + 0.5;
      float3 color = tex.Sample(sampler_default, uv).xyz;
      return float4(color, 1.0);
    }
  #endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  