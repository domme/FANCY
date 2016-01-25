#pragma once

#if defined (RENDERER_DX12)

namespace Fancy { namespace Rendering {	namespace DX12 {
//---------------------------------------------------------------------------//
	class GpuBufferDX12
	{
	public:
    GpuBufferDX12();
    ~GpuBufferDX12();

    bool isLocked() const { return myState.isLocked; }
    bool isLockedPersistent() const { return myState.isLockedPersistent; }
    bool isValid() const { return false; }  // TODO: Implement
    uint getTotalSizeBytes() const { return myTotalSizeBytes; }
    uint32 getNumElements() const { return myNumElements; }
    GpuBufferCreationParams getParameters() const { return myParameters; }

    void setBufferData(void* pData, uint uOffsetElements = 0, uint uNumElements = 0);
    void create(const GpuBufferCreationParams& clParameters, void* pInitialData = nullptr);
    void destroy();
    void* lock(GpuResoruceLockOption eLockOption, uint uOffsetElements = 0u, uint uNumElements = 0u);
    void unlock() {}

  private:

    struct BufferState {
      BufferState() :
        isLocked(0u),
        isLockedPersistent(0u) {}

      uint isLocked : 1;
      uint isLockedPersistent : 1;
    };

    BufferState myState;
    GpuBufferCreationParams myParameters;
    uint myTotalSizeBytes;
    uint myNumElements;
	};
//---------------------------------------------------------------------------//
} } }

#endif
