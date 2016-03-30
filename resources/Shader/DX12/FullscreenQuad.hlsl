
 // #include "Shader/TextureSemantics.shader_include"
 //#include "Shader/Common.inc"
 #include "inc_RootSignatures.hlsl"

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
    
    [RootSignature(RS_EMPTY)]
    VS_OUT main(VS_IN v)
    {
      VS_OUT vs_out = (VS_OUT)0;
      vs_out.pos = float4(v.position, 1.0f);

      return vs_out;
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_FRAGMENT)
    
    [RootSignature(RS_EMPTY)]
    float4 main(VS_OUT fs_in) : SV_TARGET
    {
      float3 albedo = float3(1.0, 1.0, 1.0);
      return float4(albedo, 1.0);
    }
  #endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  
