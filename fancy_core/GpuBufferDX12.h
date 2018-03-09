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
        
    void Create(const GpuBufferCreationParams& clParameters, const void* pInitialData = nullptr) override;
    void* Lock(GpuResoruceLockOption eLockOption, uint uOffsetElements = 0u, uint uNumElements = 0u) override;
    void Unlock() override;

    const DescriptorDX12* GetSrv() const { return &mySrvDescriptor; }
    const DescriptorDX12* GetUav() const { return &myUavDescriptor; }
    const DescriptorDX12* GetCbv() const { return &myCbvDescriptor; }
	  const DescriptorDX12* GetDescriptor(DescriptorType aType, uint anIndex = 0u) const override;
    
    const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const 
    { ASSERT(myParameters.myUsageFlags & (uint)GpuBufferUsage::VERTEX_BUFFER); return myVertexBufferView; }

    const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const
    { ASSERT(myParameters.myUsageFlags & (uint)GpuBufferUsage::INDEX_BUFFER); return myIndexBufferView; }

  protected:
    void Destroy();
    
    DescriptorDX12 mySrvDescriptor;
    DescriptorDX12 myUavDescriptor;
    DescriptorDX12 myCbvDescriptor;
    D3D12_VERTEX_BUFFER_VIEW myVertexBufferView;
    D3D12_INDEX_BUFFER_VIEW myIndexBufferView;
	};
//---------------------------------------------------------------------------//
}