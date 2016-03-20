
 // #include "Shader/TextureSemantics.shader_include"
 //#include "Shader/Common.inc"
 #include "inc_ConstantBuffers.hlsl"
 #include "inc_Lighting.hlsl"
 #include "inc_RootSignatures.hlsl"

  struct VS_OUT
  {
    float4 pos : SV_POSITION;
    float3 posWS : TEXCOORD0;
    float3 normalWS : TEXCOORD1;
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
    
    [RootSignature(RS_FORWARD_COLORPASS)]
    VS_OUT main(VS_IN v)
    {
      VS_OUT vs_out = (VS_OUT)0;
      vs_out.pos = mul(float4(v.position, 1.0f), cbPerObject.c_WorldViewProjectionMatrix);

      vs_out.posWS = mul(float4(v.position, 1.0f), cbPerObject.c_WorldMatrix).xyz;
      vs_out.normalWS = normalize(mul(float4(v.normal, 0.0), cbPerObject.c_WorldMatrix).xyz);
      vs_out.uv = v.texcoord0;

      return vs_out;
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_FRAGMENT)
  
    Texture2D tex_diffuse : register(t0);
    Texture2D tex_normal : register(t1);
    Texture2D tex_specular : register(t2);
    SamplerState sampler_default : register(s0);
    
    [RootSignature(RS_FORWARD_COLORPASS)]
    float4 main(VS_OUT fs_in) : SV_Target
    {
      float3 albedo = float3(1.0, 1.0, 1.0);
      albedo *= tex_diffuse.Sample(sampler_default, fs_in.uv).xyz;

      float3 lightIntensity = ShadeLight(cbPerLight.c_LightParameters, cbPerLight.c_LightDirWS, fs_in.posWS, normalize(fs_in.normalWS));
      
      return float4(albedo, 1.0);
    }
  #endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  
