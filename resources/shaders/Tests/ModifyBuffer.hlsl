#include "fancy/resources/shaders/GlobalResources.h"

cbuffer CB0 : register(b0, Space_LocalCBuffer)
{
	uint myValue;
	uint myDstBufferIndex;
	uint mySrcBufferIndex;
};

[numthreads(64, 1, 1)]
void main_increment(uint3 aDTid : SV_DispatchThreadID)
{
    uint val = theRwBuffers[myDstBufferIndex].Load<uint>(aDTid.x * sizeof(uint));
    theRwBuffers[myDstBufferIndex].Store<uint>(aDTid.x * sizeof(uint), val + 1);
}

[numthreads(64, 1, 1)]
void main_set(uint3 aDTid : SV_DispatchThreadID)
{
    theRwBuffers[myDstBufferIndex].Store<uint>(aDTid.x * sizeof(uint), myValue);
}

[numthreads(64, 1, 1)]
void main_copy(uint3 aDTid : SV_DispatchThreadID)
{
	uint val = theBuffers[mySrcBufferIndex].Load<uint>(aDTid.x * sizeof(uint));
    theRwBuffers[myDstBufferIndex].Store<uint>(aDTid.x * sizeof(uint), val); 
}