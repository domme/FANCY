#include "fancy/resources/shaders/GlobalResources.h"

cbuffer CB0 : register(b0, Space_LocalCBuffer)
{
    uint myBufferIndex;
    uint myTextureIndex;
};

#if defined(BUFFER_TO_TEXTURE_MIP)

[numthreads(8, 8, 1)]
void main(uint3 aDTid : SV_DispatchThreadID)
{
    uint val = theBuffers[myBufferIndex].Load<uint>(0);
    theRwUintTextures2D[myTextureIndex][aDTid.xy] = val + 1;
}

#endif

#if defined(TEXUTRE_MIP_TO_BUFFER)

[numthreads(8, 8, 1)]
void main(uint3 aDTid : SV_DispatchThreadID)
{
    uint val = theUintTextures2D[myTextureIndex][aDTid.xy].x;
    theRwBuffers[myBufferIndex].Store<uint>(0, val + 1);
}

#endif
