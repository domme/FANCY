
RWTexture2D<float4> testTexture	: register(u0);	// UAV

#define ROOT_SIGNATURE  "DescriptorTable(UAV(u0))"

[RootSignature(ROOT_SIGNATURE)]
[numthreads(1, 1, 1)]
void main(uint3 aGroupID : SV_GroupID, 
          uint3 aDispatchThreadID : SV_DispatchThreadID, 
          uint3 aGroupThreadID : SV_GroupThreadID, 
          uint aGroupIndex : SV_GroupIndex)
{
  testTexture[aDispatchThreadID.xy] = float4(1, 0, 0, 1);
}
