#include "GlobalResources.h"

cbuffer CB0 : register(b0, _LocalCBufferSpace)
{
	uint myValue;
	uint myDstBufferIndex;
	uint mySrcBufferIndex;
};

[numthreads(64, 1, 1)]
void main_increment(uint3 aDTid : SV_DispatchThreadID)
{
    uint val = global_rw_buffers[myDstBufferIndex].Load<uint>(aDTid.x);
    global_rw_buffers[myDstBufferIndex].Store<uint>(aDTid.x, val + 1);
}

[numthreads(64, 1, 1)]
void main_set(uint3 aDTid : SV_DispatchThreadID)
{
    global_rw_buffers[myDstBufferIndex].Store<uint>(aDTid.x, myValue);
}

[numthreads(64, 1, 1)]
void main_copy(uint3 aDTid : SV_DispatchThreadID)
{
	uint val = global_buffers[mySrcBufferIndex].Load<uint>(aDTid.x);
    global_rw_buffers[myDstBufferIndex].Store<uint>(aDTid.x, val); 
}