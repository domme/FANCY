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
    uint locGetQueryReadbackSizeBytes(GpuQueryType aQueryType)
    {
      switch (aQueryType)
      {
      case GpuQueryType::TIMESTAMP: return sizeof(uint64);
      case GpuQueryType::OCCLUSION: return sizeof(uint64);
      case GpuQueryType::NUM:
      default: ASSERT(false); return sizeof(uint64);
      }
    }
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
    bufferProps.myElementSizeBytes = locGetQueryReadbackSizeBytes(aQueryType);
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
