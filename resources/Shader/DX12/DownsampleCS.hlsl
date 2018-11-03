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
  float2 mySrcSize;
  float2 myDestSize;
  int myIsSRGB;   
};

Texture2D<float4> SrcTexture : register(t0);
RWTexture2D<float4> DestTexture: register(u0);

SamplerState sampler_linear : register(s0);
SamplerState sampler_point : register(s1);

[RootSignature(ROOT_SIGNATURE)]
[numthreads(1, 1, 1)]
void main_linear(uint3 aGroupID : SV_GroupID, 
          uint3 aDispatchThreadID : SV_DispatchThreadID, 
          uint3 aGroupThreadID : SV_GroupThreadID, 
          uint aGroupIndex : SV_GroupIndex)
{
    int2 destTexelIndex = (int2) aDispatchThreadID.xy;

    float2 numSrcTexelsPerDest = float2(myDestSize) / float2(mySrcSize);
    int2 srcTexelBase = int2(float2(destTexelIndex) * numSrcTexelsPerDest);

    int2 filterSize = int2(numSrcTexelsPerDest);
    float2 additionalTexelWeights = frac(numSrcTexelsPerDest);
    float sampleWeight = 1.0 / float2(filterSize);

    float4 filteredCol = float4(0,0,0,0);
    float weightSum = 0;
    for (int y = 0; y < filterSize; ++y)
    {
      for (int x = 0; x < filterSize; ++x)
      {
        int2 texelCoord = srcTexelBase + int2(x,y);
        float4 col = SrcTexture.Load(int3(texelCoord, 0));

        if (all(texelCoord >= 0) && all(texelCoord < mySrcSize))
        {
          weightSum += sampleWeight;
          filteredCol += sampleWeight * col;
        }
      }
    }

    filteredCol /= max(0.0001, weightSum);
    
    if (myIsSRGB)
      filteredCol = pow(filteredCol, 1.0 / 2.2);

    DestTexture[destTexelIndex] = filteredCol;
}

float Lanczos(float x, float a)
{
  const float PI = 3.14159265359;
  
  if (x == 0)
  {
    return 1.0;
  }
  else if (x >= -a && x <= a)
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
    int2 targetTexel = (int2) aDispatchThreadID.xy;
    int2 srcTexel_floor = targetTexel * 2;
    float2 tergetTexelCenter = (float2(targetTexel) / myTargetSize) * (myTargetSize * 2);

    const int a = 3;

    float4 avgColor = float4(0,0,0,0);
    float weightSum = 0;
    for (int y = -a + 1; y <= a; ++y)
    {
      for (int x = -a + 1; x <= a; ++x)
      { 
         int2 texelCoord = int2(x,y);
         float2 contSrcTexelPos = float2(srcTexel) + float2(0.5, 0.5);
         float lx = abs(float(texelCoord.x) - contSrcTexelPos.x);
         float ly = abs(float(texelCoord.y) - contSrcTexelPos.y);

         float4 col = ParentMipTexture.Load(int3(texelCoord, 0));
         float weight = Lanczos(lx, a) * Lanczos(ly, a);

        if (texelCoord.x >= 0 && texelCoord.y >= 0 && texelCoord.x < myTargetSize.x * 2 && texelCoord.y < myTargetSize.y * 2)
        {
          weightSum += weight;
          avgColor += col * weight;
        }
      }
    }
    avgColor /= weightSum;

    if (myIsSRGB)
      avgColor = pow(avgColor, 1.0 / 2.2);

    MipTexture[targetTexel] = avgColor;
}