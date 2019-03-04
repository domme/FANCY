#include "fancy_core_precompile.h"
#include "GpuQueryHeap.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuQueryHeap::GpuQueryHeap(GpuQueryType aQueryType, uint aNumQueries)
    : myType(aQueryType)
    , myNumQueries(aNumQueries)
  {
    
  }
//---------------------------------------------------------------------------//
  GpuQueryHeap::~GpuQueryHeap()
  {
  }
//---------------------------------------------------------------------------//
  uint GpuQueryHeap::Allocate(uint aNumQueries)
  {
    ASSERT(myNextFreeQuery + aNumQueries <= myNumQueries);
    uint firstQuery = myNextFreeQuery;
    myNextFreeQuery += aNumQueries;
    return firstQuery;
  }
//---------------------------------------------------------------------------//
}