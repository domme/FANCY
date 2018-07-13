#pragma once

#include "DX12Prerequisites.h"
#include "GpuBuffer.h"
#include "DescriptorDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
	class GpuBufferDX12 : public GpuBuffer
	{
	public:
    GpuBufferDX12();
    ~GpuBufferDX12() override;
        
    void Create(const GpuBufferProperties& clParameters, const void* pInitialData = nullptr) override;
    void* Lock(GpuResoruceLockOption eLockOption, uint uOffsetElements = 0u, uint uNumElements = 0u) const override;
    void Unlock(uint anOffsetElements = 0u, uint aNumElements = 0u) const override;

  protected:
    void Destroy();
	};
//---------------------------------------------------------------------------//
}