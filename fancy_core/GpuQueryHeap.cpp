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
}