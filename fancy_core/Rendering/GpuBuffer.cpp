#include "fancy_core_precompile.h"
#include "GpuBuffer.h"
#include "RenderCore.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuBuffer::GpuBuffer()
    : GpuResource(GpuResourceType::BUFFER)
    , myAlignment(0u)
  {
  }
//---------------------------------------------------------------------------//
  void* GpuBuffer::Map(GpuResourceMapMode aMapMode, uint64 anOffset, uint64 aSize) const
  {
    ASSERT(IsValid());

    const uint64 bufferSize = myProperties.myNumElements * myProperties.myElementSizeBytes;
    aSize = glm::min(bufferSize, aSize);
    ASSERT(anOffset + aSize <= bufferSize);

    const bool isCpuWritable = myProperties.myCpuAccess == CpuMemoryAccessType::CPU_WRITE;
    const bool isCpuReadable = myProperties.myCpuAccess == CpuMemoryAccessType::CPU_READ;

    const bool wantsWrite = aMapMode == GpuResourceMapMode::WRITE_UNSYNCHRONIZED || aMapMode == GpuResourceMapMode::WRITE;
    const bool wantsRead = aMapMode == GpuResourceMapMode::READ_UNSYNCHRONIZED || aMapMode == GpuResourceMapMode::READ;

    if ((wantsWrite && !isCpuWritable) || (wantsRead && !isCpuReadable))
      return nullptr;

    // TODO: Ideally this should only wait for the last fence per queue this resource written last. But we don't have this data (yet?)
    const bool needsWait = aMapMode == GpuResourceMapMode::READ || aMapMode == GpuResourceMapMode::WRITE;
    if (needsWait)
    {
      for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
        RenderCore::WaitForIdle((CommandListType)i);
    }

    return Map_Internal(anOffset, aSize);
  }
//---------------------------------------------------------------------------//
  void GpuBuffer::Unmap(GpuResourceMapMode aMapMode, uint64 anOffset, uint64 aSize) const
  {
    const uint64 bufferSize = myProperties.myNumElements * myProperties.myElementSizeBytes;
    aSize = glm::min(bufferSize, aSize);
    ASSERT(anOffset + aSize <= bufferSize);

    Unmap_Internal(aMapMode, anOffset, aSize);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  GpuBufferView::GpuBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aName) :
    GpuResourceView(eastl::static_pointer_cast<GpuResource>(aBuffer), aName)
    , myProperties(someProperties)
  {
    if (someProperties.myIsConstantBuffer)
    {
      myType = GpuResourceViewType::CBV;
    }
    else if (someProperties.myIsShaderWritable)
    {
      myType = GpuResourceViewType::UAV;
    }
    else
    {
      if (someProperties.myIsRtAccelerationStructure)
        myType = GpuResourceViewType::SRV_RT_AS;
      else
        myType = GpuResourceViewType::SRV;
    }
      
  }
//---------------------------------------------------------------------------//
}
