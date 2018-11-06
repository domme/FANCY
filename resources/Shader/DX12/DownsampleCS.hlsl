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
  int myFilterMethod;
};

Texture2D<float4> SrcTexture : register(t0);
RWTexture2D<float4> DestTexture: register(u0);

SamplerState sampler_linear : register(s0);
SamplerState sampler_point : register(s1);

float Weight_Linear(float x, float width)
{
  return max(0, (1.0 - (x / width)));
}

float4 Resample_Linear(int2 aDestTexel)
{
  float2 destScale = mySrcSize / myDestSize;
  
  float2 filterCenter = (aDestTexel + 0.5) * destScale;
  float2 filterSize = destScale * 0.5;

  int2 sampleRange = (int) ceil(filterSize);
  int2 centerTexel = (int2) filterCenter;

  float4 valSum = float4(0,0,0,0);
  float norm = 0;
  for (int y = -sampleRange.y; y <= sampleRange.y; y++)
  {
    for (int x = -sampleRange.x; x <= sampleRange.x; x++)
    {
      int2 texelCoord = centerTexel + int2(x,y);
      if(all(texelCoord >= int2(0,0) && all(texelCoord < int2(mySrcSize)))) 
      {
        float2 sampleDist = abs(float2(texelCoord + 0.5) - filterCenter);
        float weight = Weight_Linear(sampleDist.x, filterSize.x) * Weight_Linear(sampleDist.y, filterSize.y);

        valSum += weight * SrcTexture.Load(int3(texelCoord, 0));
        norm += weight;
      }
    }
  }

  return valSum / norm;
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

float4 Resample_Lanczos(int2 aDestTexel)
{
  const int a = 3;
  float2 destTexelSize = mySrcSize / myDestSize;
  float2 srcTexelSize = myDestSize / mySrcSize;
  float2 destTexelCenter = (aDestTexel + 0.5) * destTexelSize;
  float2 filterSize = a * max(destTexelSize, srcTexelSize);

  float4 valSum = float4(0,0,0,0);
  float norm = 0;

  for (float y = -filterSize.y; y <= filterSize.y; y += 1.0 / filterSize.y)
  {
    for (float x = -filterSize.x; x <= filterSize.x; x += 1.0 / filterSize.x)
    {
      float2 samplePos = destTexelCenter + float2(x,y);
      int2 texelCoord = int2(samplePos);

      if(all(texelCoord >= int2(0,0) && all(texelCoord < int2(mySrcSize)))) 
      {
        float weight = Lanczos(x, filterSize.x) * Lanczos(y, filterSize.y);
        norm += weight;

        valSum += weight * SrcTexture.SampleLevel(sampler_linear, samplePos / mySrcSize, 0);
      }
    }
  }

  return valSum / norm;
}

[RootSignature(ROOT_SIGNATURE)]
[numthreads(1, 1, 1)]
void main(uint3 aGroupID : SV_GroupID, 
          uint3 aDispatchThreadID : SV_DispatchThreadID, 
          uint3 aGroupThreadID : SV_GroupThreadID, 
          uint aGroupIndex : SV_GroupIndex)
{
    int2 destTexel = (int2) aDispatchThreadID.xy;

    float4 filteredCol = float4(0,0,0,0);
    if (myFilterMethod == 0)
      filteredCol = Resample_Linear(destTexel);
    else if (myFilterMethod == 1)
      filteredCol = Resample_Lanczos(destTexel);
    
    if (myIsSRGB)
      filteredCol = pow(filteredCol, 1.0 / 2.2);

    DestTexture[destTexel] = filteredCol;
}

/*
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
*/