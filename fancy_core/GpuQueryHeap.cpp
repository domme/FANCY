#include "fancy_core_precompile.h"
#include "GpuQueryHeap.h"
#include "TimeManager.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuQueryHeap::GpuQueryHeap(QueryType aQueryType, uint aNumQueries)
    : myType(aQueryType)
    , myNumQueries(aNumQueries)
  {
  }
//---------------------------------------------------------------------------//
  GpuQuery GpuQueryHeap::Allocate()
  {
    ASSERT(myNextFree != myNumQueries);
    return { myType, myNextFree++, Time::ourFrameIdx, this };
  }
//---------------------------------------------------------------------------//
  void GpuQueryHeap::Reset()
  {
    myNextFree = 0u;
  }
//---------------------------------------------------------------------------//
}
