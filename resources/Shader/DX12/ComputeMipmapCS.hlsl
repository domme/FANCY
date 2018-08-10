
Texture2D<float> ParentMipTexture : register(t0);
RWTexture2D<float4> MipTexture: register(u0);

cbuffer : register(b0)
{
  float myParentTexelSizeInv;
  int myMipLevel;
  float2 _unused;
};

SamplerState sampler_linear : register(s0);

#define ROOT_SIGNATURE  "DescriptorTable(SRV(t0), UAV(u0))" \
                        "CBV(b0)" \
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
    uint2 targetTexel = aDispatchThreadID.xy;
    
    float2 uv = float2(targetTexel * 2) + float2(myParentTexelSizeInv * 0.5);
    MipTexture[targetTexel] = ParentMipTexture.SampleLevel(sampler_linear, uv, myMipLevel - 1);
}
