#define ROOT_SIGNATURE  "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT )," \
                        "CBV(b0), " \                      
                        "SRV(t0), " \
                        "StaticSampler(s0, " \
                        "addressU = TEXTURE_ADDRESS_WRAP, " \
                        "addressV = TEXTURE_ADDRESS_WRAP, " \
                        "addressW = TEXTURE_ADDRESS_WRAP, " \
                        "filter = FILTER_MIN_MAG_MIP_LINEAR )"

  struct VS_OUT
  {
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
  };

  struct IMGUI_VS_CBUFFER
  {
    float4x4 ProjectionMatrix; 
  };

  ConstantBuffer<IMGUI_VS_CBUFFER> cbVSImgui : register(b0);
  
//---------------------------------------------------------------------------//
#if defined(PROGRAM_TYPE_VERTEX)
    struct VS_IN
    {
      float2 pos : POSITION;
      float4 col : COLOR0;
      float2 uv  : TEXCOORD0;
    };
    
    [RootSignature(ROOT_SIGNATURE)]
    VS_OUT main(VS_IN input)
    {
      VS_OUT output (VS_OUT)0;
      output.pos = mul( cbVSImgui.ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
      output.col = input.col;
      output.uv  = input.uv;
      return output;
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
#if defined(PROGRAM_TYPE_FRAGMENT)  
  Texture2D texture0 : register(t0);
  SamplerState sampler_default : register(s0);
 
  [RootSignature(ROOT_SIGNATURE)]
  float4 main(PS_INPUT input) : SV_Target
  {
    float4 out_col = input.col * texture0.Sample(sampler_default, input.uv); 
    return out_col; 
  }
#endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//