#include "fancy_core_precompile.h"
#include "GpuResourceViewSet.h"
#include "RenderCore.h"
#include "Slot.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuResourceViewSet::GpuResourceViewSet(const eastl::span<GpuResourceViewSetElement>& someResources)
    : myResources(someResources.begin(), someResources.end())
    , myIsDirty(true)
  {
    for (GpuResourceViewSetElement& element : myResources)
      if (element.myView != nullptr)
        element.myView->myOnDestroyed.Connect(this, &GpuResourceViewSet::OnViewDestroyed);
  }
//---------------------------------------------------------------------------//
  GpuResourceViewSet::~GpuResourceViewSet()
  {
    for (GpuResourceViewSetElement& element : myResources)
      if (element.myView != nullptr)
        element.myView->myOnDestroyed.DetachObserver(this);
  }
//---------------------------------------------------------------------------//
  int GpuResourceViewSet::GetResourceViewIndex(const GpuResourceView* aView) const
  {
    const GpuResourceViewSetElement* const it = eastl::find_if(myResources.begin(), myResources.end(),
[aView](const GpuResourceViewSetElement& anElement)
        {
           return aView == anElement.myView;
        });

    return it != myResources.end() ? static_cast<int>(it - myResources.begin()) : -1;
  }
//---------------------------------------------------------------------------//
  bool GpuResourceViewSet::HasResourceView(const GpuResourceView* aView) const
  {
    return GetResourceViewIndex(aView) >= 0;
  }
//---------------------------------------------------------------------------//
  void GpuResourceViewSet::SetResourceView(int anIndex, const GpuResourceView* aView)
  {
    ASSERT((int)myResources.size() > anIndex);
    ASSERT(myResources[anIndex].myType == aView->myType);

    GpuResourceViewSetElement& element = myResources[anIndex];
    if (element.myView == aView)
      return;

    if (element.myView != nullptr)
      element.myView->myOnDestroyed.DetachObserver(this);

    if (aView != nullptr)
      aView->myOnDestroyed.Connect(this, &GpuResourceViewSet::OnViewDestroyed);

    element.myView = aView;
    myIsDirty = true;
  }
//---------------------------------------------------------------------------//
  void GpuResourceViewSet::OnViewDestroyed(GpuResourceView* aView)
  {
    const int index = GetResourceViewIndex(aView);
    ASSERT(index >= 0);
    SetResourceView(index, nullptr);
  }
//---------------------------------------------------------------------------//
}


