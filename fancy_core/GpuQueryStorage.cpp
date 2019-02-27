#include "fancy_core_precompile.h"
#include "GpuQueryStorage.h"
#include "RenderCore.h"
#include "TimeManager.h"
#include "GpuQueryHeap.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace
  {
 //---------------------------------------------------------------------------//
    const char* locGetQueryTypeName(GpuQueryType aQueryType)
    {
      switch (aQueryType)
      {
      case GpuQueryType::TIMESTAMP: return "Timestamp";
      case GpuQueryType::OCCLUSION: return "Occlusion";
      case GpuQueryType::NUM:
      default: ASSERT(false); return "";
      }
    }
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void GpuQueryStorage::Create(GpuQueryType aQueryType, uint aNumQueries)
  {
    myQueryHeap.reset(RenderCore::GetPlatform()->CreateQueryHeap(aQueryType, aNumQueries));
    ASSERT(myQueryHeap != nullptr);

    GpuBufferProperties bufferProps;
    bufferProps.myCpuAccess = CpuMemoryAccessType::CPU_READ;
    bufferProps.myUsage = GpuBufferUsage::STAGING_READBACK;
    bufferProps.myElementSizeBytes = RenderCore::GetQueryTypeDataSize(aQueryType);
    bufferProps.myNumElements = aNumQueries;
    String name = StringFormat("QueryHeap %", locGetQueryTypeName(aQueryType));
    myReadbackBuffer = RenderCore::CreateBuffer(bufferProps, name.c_str());
    ASSERT(myReadbackBuffer != nullptr);
  }
//---------------------------------------------------------------------------//
  uint GpuQueryStorage::GetNumFreeQueries() const
  {
    return myQueryHeap->myNumQueries - myNextFree;
  }
//---------------------------------------------------------------------------//
  bool GpuQueryStorage::AllocateQueries(uint aNumQueries, uint& aStartQueryIdxOut)
  {
    if (GetNumFreeQueries() < aNumQueries)
      return false;

    aStartQueryIdxOut = myNextFree;
    myNextFree += aNumQueries;
    return true;
  }
//---------------------------------------------------------------------------//
}
