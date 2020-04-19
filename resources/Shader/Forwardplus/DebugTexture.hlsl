#define ROOT_SIGNATURE "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT )," \
                       "CBV(b0)," \
                       "DescriptorTable(SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_PIXEL)," \
                       "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, addressV = TEXTURE_ADDRESS_CLAMP, filter = FILTER_MIN_MAG_MIP_POINT)"

  struct TEXTURE_PARAMS
  {
    float4 myParams;
    float4 _padding;
  };
  ConstantBuffer<TEXTURE_PARAMS> cbParams : register(b0);

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
    
    [RootSignature(ROOT_SIGNATURE)]
    VS_OUT main(VS_IN v)
    {
      VS_OUT vs_out = (VS_OUT)0;
      vs_out.pos = float4(v.position.xy, 0.0f, 1.0f);

      return vs_out;
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_FRAGMENT)
    Texture2D debugTex : register(t0);
    SamplerState sampler_default : register(s0);

    [RootSignature(ROOT_SIGNATURE)]
    float4 main(VS_OUT fs_in) : SV_TARGET
    {
      float2 uv = fs_in.pos.xy * 0.5 + 0.5;
      float4 color = debugTex.Sample(sampler_default, uv);
      return color;
    }

    [RootSignature(ROOT_SIGNATURE)]
    float4 main_depthBuffer(VS_OUT fs_in) : SV_TARGET
    {
      const float minDepth = cbParams.myParams.x;
      const float maxDepth = cbParams.myParams.y;

      float2 uv = fs_in.pos.xy * 0.5 + 0.5;
      float depth = debugTex.Sample(sampler_default, uv).x;
      depth = clamp(depth, minDepth, maxDepth);
      depth = (depth - minDepth) / (maxDepth - minDepth);
      return float4(depth, 0, 0, 1);
    }
  #endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  
