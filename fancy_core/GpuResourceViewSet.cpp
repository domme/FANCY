#include "fancy_core_precompile.h"
#include "GpuResourceViewSet.h"
#include "RenderCore.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuResourceViewSet::GpuResourceViewSet(const eastl::span<GpuResourceViewRange>& someRanges)
    : myRanges(someRanges.begin(), someRanges.end())
  {
  }
//---------------------------------------------------------------------------//
  bool GpuResourceViewSet::HasResourceView(const GpuResourceView* aView) const
  {
    for (const GpuResourceViewRange& range : myRanges)
    {
      for (const SharedPtr<GpuResourceView>& view : range.myResources)
      {
        if (view.get() == aView)
          return true;
      }
    }

    return false;
  }
//---------------------------------------------------------------------------//
}


