cbuffer CB0 : register(b0)
{
  float2 mySizeOnMipInv;
  int myMip;
  int myIsSRGB;   
};

Texture2D<float> ParentMipTexture : register(t0);
RWTexture2D<float4> MipTexture: register(u0);

SamplerState sampler_linear : register(s0);

#define ROOT_SIGNATURE  "CBV(b0)," \
                        "DescriptorTable(SRV(t0), UAV(u0))," \
                        "StaticSampler(s0, " \
                        "addressU = TEXTURE_ADDRESS_CLAMP, " \
                        "addressV = TEXTURE_ADDRESS_CLAMP, " \
                        "filter = FILTER_MIN_MAG_LINEAR_MIP_POINT )"

[RootSignature(ROOT_SIGNATURE)]
[numthreads(1, 1, 1)]
void main(uint3 aGroupID : SV_GroupID, 
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