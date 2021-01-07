#include "Vulkan_Support.h"

VK_BINDING_SET(0, 0) cbuffer CB0 : register(b0)
{
	uint myValue;
	uint3 _padding;
};

VK_BINDING_SET(1, 0) RWBuffer<uint> DstBuffer : register(u0);
VK_BINDING_SET(2, 0) Buffer<uint> SrcBuffer : register(t0);

[RootSignature("DescriptorTable(UAV(u0))")]
[numthreads(64, 1, 1)]
void main_increment(uint3 aDTid : SV_DispatchThreadID)
{
    uint val = DstBuffer[aDTid.x];
    DstBuffer[aDTid.x] = val + 1;
}

[RootSignature("CBV(b0), DescriptorTable(UAV(u0))")]
[numthreads(64, 1, 1)]
void main_set(uint3 aDTid : SV_DispatchThreadID)
{
    DstBuffer[aDTid.x] = myValue;
}

[RootSignature("DescriptorTable(UAV(u0), SRV(t0))")]
[numthreads(64, 1, 1)]
void main_copy(uint3 aDTid : SV_DispatchThreadID)
{
    DstBuffer[aDTid.x] = SrcBuffer[aDTid.x];
}