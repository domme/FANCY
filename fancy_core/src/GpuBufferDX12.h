#pragma once

#include "GpuBuffer.h"
#include "GpuResourceDX12.h"
#include "DescriptorDX12.h"

namespace Fancy { namespace Rendering {	namespace DX12 {
//---------------------------------------------------------------------------//
	class GpuBufferDX12 : public GpuBuffer, public GpuResourceDX12
	{
	public:
    GpuBufferDX12();
    ~GpuBufferDX12() override;
     
    void Create(const GpuBufferCreationParams& clParameters, void* pInitialData = nullptr) override;
    void* Lock(GpuResoruceLockOption eLockOption, uint uOffsetElements = 0u, uint uNumElements = 0u) override;
    void Unlock() override;
    bool IsValid() const override { return myResource != nullptr; }
    
    const DescriptorDX12& GetSrvDescriptor() const { return mySrvDescriptor; }
    const DescriptorDX12& GetUavDescriptor() const { return myUavDescriptor; }
    const DescriptorDX12& GetCbvDescriptor() const { return myCbvDescriptor; }
    
    const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const 
    { ASSERT(myParameters.myUsageFlags & (uint32)GpuBufferUsage::VERTEX_BUFFER); return myVertexBufferView; }

    const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const
    { ASSERT(myParameters.myUsageFlags & (uint32)GpuBufferUsage::INDEX_BUFFER); return myIndexBufferView; }

  protected:
    void Destroy();
    
    DescriptorDX12 mySrvDescriptor;
    DescriptorDX12 myUavDescriptor;
    DescriptorDX12 myCbvDescriptor;
    D3D12_VERTEX_BUFFER_VIEW myVertexBufferView;
    D3D12_INDEX_BUFFER_VIEW myIndexBufferView;
	};
//---------------------------------------------------------------------------//
} } }