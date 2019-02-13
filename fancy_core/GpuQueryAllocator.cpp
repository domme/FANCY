#include "fancy_core_precompile.h"
#include "GpuQueryAllocator.h"
#include "GpuQueryHeap.h"
#include "TimeManager.h"


namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuQueryAllocator::GpuQueryAllocator(GpuQueryHeap* aHeap)
    : myHeap(aHeap)
  {
  }
//---------------------------------------------------------------------------//
  void GpuQueryAllocator::BeginFrame()
  {
    myNextFree = 0u;
  }
//---------------------------------------------------------------------------//
  GpuQuery GpuQueryAllocator::Allocate()
  {
    ASSERT(myHeap->myNumQueries < myNextFree);
    return { myHeap->myType, myNextFree++, Time::ourFrameIdx, myHeap.get() };
  }
//---------------------------------------------------------------------------//
}