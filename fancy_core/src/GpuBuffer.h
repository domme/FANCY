#pragma once

#include "RendererPrerequisites.h"
#include "GpuResource.h"
#include "GpuBufferDesc.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class GpuBuffer : public GpuResource
  {
  public:
    GpuBuffer();
    virtual ~GpuBuffer();

    bool operator==(const GpuBufferDesc& aDesc) const;
    GpuBufferDesc GetDescription() const;

    uint GetSizeBytes() const { return myParameters.uNumElements * myParameters.uElementSizeBytes; }
    uint32 GetNumElements() const { return myParameters.uNumElements; }
    GpuBufferCreationParams GetParameters() const { return myParameters; }
    uint32 GetAlignment() const { return myAlignment; }

    virtual void Create(const GpuBufferCreationParams& clParameters, void* pInitialData = nullptr) = 0;
    virtual void* Lock(GpuResoruceLockOption eLockOption, uint uOffsetElements = 0u, uint uNumElements = 0u) = 0;
    virtual void Unlock() = 0;
    
  protected:
    struct LockState {
      LockState()
        : isLocked(false)
        , myLockedRange_Begin(0u)
        , myLockedRange_End(0u)
        , myCachedLockDataPtr(nullptr)
      {}

      bool isLocked;
      uint64 myLockedRange_Begin;
      uint64 myLockedRange_End;
      void* myCachedLockDataPtr;
    };

    LockState myState;
    uint32 myAlignment;
    GpuBufferCreationParams myParameters;

  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering
