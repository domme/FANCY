#define ROOT_SIGNATURE  "CBV(b0),
						 DescriptorTable(UAV(u0))"

cbuffer CB0 : register(b0)
{
	uint myValue;
	uint3 _padding;
};

RWBuffer<uint> Buffer : register(u0);

[RootSignature(ROOT_SIGNATURE)]
[numthreads(64, 1, 1)]
void main_increment(uint3 aDTid : SV_DispatchThreadID)
{
    uint val = Buffer[aDTid.x];
    Buffer[aDTid.x] = val + 1;
}

[RootSignature(ROOT_SIGNATURE)]
[numthreads(64, 1, 1)]
void main_set(uint3 aDTid : SV_DispatchThreadID)
{
    Buffer[aDTid.x] = myValue;
}