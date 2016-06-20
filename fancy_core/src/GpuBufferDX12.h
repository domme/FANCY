#pragma once
#include "GpuResource.h"

#if defined (RENDERER_DX12)

#include "GpuBufferDesc.h"
#include "GpuResourceDX12.h"
#include "DescriptorDX12.h"

namespace Fancy { namespace Rendering {	namespace DX12 {
//---------------------------------------------------------------------------//
	class GpuBufferDX12 : public GpuResource
	{
	public:
    GpuBufferDX12();
    ~GpuBufferDX12();
    bool operator==(const GpuBufferDesc& aDesc) const;
    GpuBufferDesc GetDescription() const;

    bool isLocked() const { return myState.isLocked; }
    bool isLockedPersistent() const { return myState.isLocked; }
    bool isValid() const { return myResource != nullptr; }  // TODO: Implement
    uint GetSizeBytes() const { return myParameters.uNumElements * myParameters.uElementSizeBytes; }
    uint32 getNumElements() const { return myParameters.uNumElements; }
    GpuBufferCreationParams GetParameters() const { return myParameters; }
    uint32 GetAlignment() const { return myAlignment; }

    void create(const GpuBufferCreationParams& clParameters, void* pInitialData = nullptr);
    void destroy();
    void* Lock(GpuResoruceLockOption eLockOption, uint uOffsetElements = 0u, uint uNumElements = 0u);
    void Unlock();
 
    const DescriptorDX12& GetSrvDescriptor() const { return mySrvDescriptor; }
    const DescriptorDX12& GetUavDescriptor() const { return myUavDescriptor; }
    const DescriptorDX12& GetCbvDescriptor() const { return myCbvDescriptor; }
    
    const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const 
    { ASSERT(myParameters.myUsageFlags & (uint32)GpuBufferUsage::VERTEX_BUFFER); return myVertexBufferView; }

    const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const
    { ASSERT(myParameters.myUsageFlags & (uint32)GpuBufferUsage::INDEX_BUFFER); return myIndexBufferView; }

  protected:
    struct BufferState {
      BufferState() 
        : isLocked(false)
        , myCachedLockDataPtr(nullptr)
      {}

      bool isLocked : 1;

      D3D12_RANGE myLockedRange;
      void* myCachedLockDataPtr;
    };

    BufferState myState;

    uint32 myAlignment;

    GpuBufferCreationParams myParameters;

    DescriptorDX12 mySrvDescriptor;
    DescriptorDX12 myUavDescriptor;
    DescriptorDX12 myCbvDescriptor;
    D3D12_VERTEX_BUFFER_VIEW myVertexBufferView;
    D3D12_INDEX_BUFFER_VIEW myIndexBufferView;
	};
//---------------------------------------------------------------------------//
} } }

#endif

