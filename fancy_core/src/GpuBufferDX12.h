#pragma once

#if defined (RENDERER_DX12)

#include "GpuBufferDesc.h"

namespace Fancy { namespace Rendering {	namespace DX12 {
//---------------------------------------------------------------------------//
	class GpuBufferDX12
	{
	public:
    GpuBufferDX12();
    ~GpuBufferDX12();
    bool operator==(const GpuBufferDesc& aDesc) const;
    GpuBufferDesc GetDescription() const;

    bool isLocked() const { return myState.isLocked; }
    bool isLockedPersistent() const { return myState.isLocked; }
    bool isValid() const { return false; }  // TODO: Implement
    uint getTotalSizeBytes() const { return myTotalSizeBytes; }
    uint32 getNumElements() const { return myNumElements; }
    GpuBufferCreationParams getParameters() const { return myParameters; }

    void setBufferData(void* pData, uint uOffsetElements = 0, uint uNumElements = 0);
    void create(const GpuBufferCreationParams& clParameters, void* pInitialData = nullptr);
    void destroy();
    void* lock(GpuResoruceLockOption eLockOption, uint uOffsetElements = 0u, uint uNumElements = 0u);
    void unlock();

  private:

    struct BufferState {
      BufferState() 
        : isLocked(false)
        , isLockedForWrite(false) 
        , myCachedLockDataPtr(nullptr)
      {}

      bool isLocked : 1;
      bool isLockedForWrite : 1;

      D3D12_RANGE myLockedRange;
      void* myCachedLockDataPtr;
    };

    BufferState myState;

    GpuBufferCreationParams myParameters;
    uint myTotalSizeBytes;
    uint myNumElements;

    ComPtr<ID3D12Resource> myResource;
    ComPtr<ID3D12Resource> myStagingResource;  //< Optional shadow-resource for CPU-writes
	};
//---------------------------------------------------------------------------//
} } }

#endif

