#include "GpuBufferDX12.h"

#if defined (RENDERER_DX12)

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  GpuBufferDX12::GpuBufferDX12()
  {
  }
//---------------------------------------------------------------------------//
  GpuBufferDX12::~GpuBufferDX12()
  {
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::setBufferData(void* pData, uint uOffsetElements, uint uNumElements)
  {
  }

  void GpuBufferDX12::create(const GpuBufferParameters& clParameters, void* pInitialData)
  {
  }

  void GpuBufferDX12::destroy()
  {
  }

  void* GpuBufferDX12::lock(GpuResoruceLockOption eLockOption, uint uOffsetElements, uint uNumElements)
  {
    return nullptr;
  }


} } }

#endif
