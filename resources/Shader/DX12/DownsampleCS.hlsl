#define ROOT_SIGNATURE  "CBV(b0)," \
                        "DescriptorTable(SRV(t0), UAV(u0))," \
                        "StaticSampler(s0, " \
                        "addressU = TEXTURE_ADDRESS_WRAP, " \
                        "addressV = TEXTURE_ADDRESS_WRAP, " \
                        "filter = FILTER_MIN_MAG_LINEAR_MIP_POINT )," \
                        "StaticSampler(s1, " \
                        "addressU = TEXTURE_ADDRESS_WRAP, " \
                        "addressV = TEXTURE_ADDRESS_WRAP, " \
                        "filter = FILTER_MIN_MAG_MIP_POINT )"

cbuffer CB0 : register(b0)
{
  float2 mySizeOnMipInv;
  int myMip;
  int myIsSRGB;   
};

Texture2D<float4> ParentMipTexture : register(t0);
RWTexture2D<float4> MipTexture: register(u0);

SamplerState sampler_linear : register(s0);
SamplerState sampler_point : register(s1);

[RootSignature(ROOT_SIGNATURE)]
[numthreads(1, 1, 1)]
void main_linear(uint3 aGroupID : SV_GroupID, 
          uint3 aDispatchThreadID : SV_DispatchThreadID, 
          uint3 aGroupThreadID : SV_GroupThreadID, 
          uint aGroupIndex : SV_GroupIndex)
{
    float2 targetTexel = float2(aDispatchThreadID.xy);
    float2 srcUv = (targetTexel + 0.5) * mySizeOnMipInv; // Gathers the neighboring 4x4 texels
    
    float4 avgColor = ParentMipTexture.SampleLevel(sampler_linear, srcUv, 0);
    if (myIsSRGB)
      avgColor = pow(avgColor, 1.0 / 2.2);

    MipTexture[targetTexel] = avgColor;
}


float Lancozos(float x, float a)
{
  const float PI = 3.14159265359;
  
  if (x == 0)
  {
    return 1.0;
  }
  else if (x > -a && x < a)
  {
    return (a* sin(PI * x) * sin((PI*x)/a)) / ((PI*PI) * (x*x));
  }
  else
  {
    return 0.0;
  }
}

[RootSignature(ROOT_SIGNATURE)]
[numthreads(1, 1, 1)]
void main_lanczos(uint3 aGroupID : SV_GroupID, 
          uint3 aDispatchThreadID : SV_DispatchThreadID, 
          uint3 aGroupThreadID : SV_GroupThreadID, 
          uint aGroupIndex : SV_GroupIndex)
{
    float2 targetTexel = float2(aDispatchThreadID.xy);
    float2 srcUv = (targetTexel + 0.5) * mySizeOnMipInv;
    float2 srcUvStep = mySizeOnMipInv * 0.5;

    const int a = 3;

    float4 avgColor = float4(0,0,0,0);
    for (int y = -a; y < a; ++y)
    {
      float ly = Lancozos(y, a);
      for (int x = -a; x < a; ++x)
      {
        float4 col = ParentMipTexture.SampleLevel(sampler_linear, srcUv + float2(x, y) * srcUvStep, 0);
        float lx = Lancozos(x, a);

        avgColor += col * lx * ly;
      }
    }
    
    if (myIsSRGB)
      avgColor = pow(avgColor, 1.0 / 2.2);

    MipTexture[targetTexel] = avgColor;
}