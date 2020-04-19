#include "Vulkan_Support.h"

#define ROOT_SIGNATURE  "CBV(b0)," \
                        "DescriptorTable(SRV(t0), UAV(u0))"

VK_BINDING_SET(0, 0) cbuffer CB0 : register(b0)
{
  float2 mySrcSize;
  float2 myDestSize;
  
  float2 mySrcScale;
  float2 myDestScale;

  int myIsSRGB;   
  int myFilterMethod;
  float2 myAxis;
};

VK_BINDING_SET(1, 0) Texture2D<float4> SrcTexture : register(t0);
VK_BINDING_SET(2, 0) RWTexture2D<float4> DestTexture: register(u0);

struct SampleParams
{
  float2 myFilterCenter;
  float2 myFilterSize;
  float myFilterCenterAxis;
  float myFilterSizeAxis;
  int2 myCenterTexel;
  int myCenterTexelAxis;
  int mySampleRange;
  int mySrcSizeOnAxis;
};

SampleParams PrecomputeSampleParams(int2 aDestTexel, float aFilterSize)
{
  SampleParams params;

  params.myFilterCenter = (aDestTexel + 0.5) * myDestScale;
  params.myFilterSize = aFilterSize * max(mySrcScale, myDestScale) * 0.5;
  params.myFilterCenterAxis = dot(myAxis, params.myFilterCenter);
  params.myFilterSizeAxis = dot(myAxis, params.myFilterSize);
  params.myCenterTexel = (int2) params.myFilterCenter;
  params.myCenterTexelAxis = (int) (dot(myAxis, params.myCenterTexel));
  params.mySampleRange = (int) ceil(params.myFilterSizeAxis);
  params.mySrcSizeOnAxis = (int) (dot(myAxis, mySrcSize));

  return params;
}

float Weight_Lanczos(float x, float a)
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

float Weight_Linear(float x, float width)
{
  return max(0, 1.0 - (x / width));
}

float4 Resize_Linear(int2 aDestTexel)
{
  SampleParams params = PrecomputeSampleParams(aDestTexel, 1.0);  

  float4 valSum = float4(0,0,0,0);
  float norm = 0;
  for (int s = -params.mySampleRange; s <= params.mySampleRange; ++s)
  {
    int texelPosAxis = params.myCenterTexelAxis + s; 
    if (texelPosAxis >= 0 && texelPosAxis < params.mySrcSizeOnAxis)
    {
      int2 texelPos = params.myCenterTexel + int2(myAxis * s);
      float sampleDist = abs(float(texelPosAxis + 0.5) - params.myFilterCenterAxis);

      float weight = Weight_Linear(sampleDist, params.myFilterSizeAxis);
      valSum += weight * SrcTexture.Load(int3(texelPos, 0));
      norm += weight;
    }
  }

  return valSum / norm;
}

float4 Resize_Lanczos(int2 aDestTexel)
{
  SampleParams params = PrecomputeSampleParams(aDestTexel, 3.0);

  float4 valSum = float4(0,0,0,0);
  float norm = 0;

  for (int s = -params.mySampleRange; s <= params.mySampleRange; ++s)
  {
    int texelPosAxis = params.myCenterTexelAxis + s; 
    if (texelPosAxis >= 0 && texelPosAxis < params.mySrcSizeOnAxis)
    {
      int2 texelPos = params.myCenterTexel + int2(myAxis * s);
      float sampleDist = abs(float(texelPosAxis + 0.5) - params.myFilterCenterAxis);

      float weight = Weight_Lanczos(sampleDist, params.myFilterSizeAxis);
      valSum += weight * SrcTexture.Load(int3(texelPos, 0));
      norm += weight;
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
      filteredCol = Resize_Linear(destTexel);
    else if (myFilterMethod == 1)
      filteredCol = Resize_Lanczos(destTexel);
    
    if (myIsSRGB)
      filteredCol = pow(filteredCol, 1.0 / 2.2);

    DestTexture[destTexel] = filteredCol;
}