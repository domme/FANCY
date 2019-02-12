#include "fancy_core_precompile.h"
#include "GpuQueryHeap.h"

namespace Fancy
{
  GpuQueryHeap::GpuQueryHeap(QueryType aQueryType, uint aNumQueries)
    : myType(aQueryType)
    , myNumQueries(aNumQueries)
  {
  }
}