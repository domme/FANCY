#pragma once

#include "GpuResourceView.h"

namespace Fancy
{
  struct GpuResourceViewSetElement
  {
    const GpuResourceView* myView;
    GpuResourceViewType myType;
  };

  class GpuResourceViewSet
  {
  public:
    GpuResourceViewSet(const eastl::span<GpuResourceViewSetElement>& someResources);
    virtual ~GpuResourceViewSet();

    int GetResourceViewIndex(const GpuResourceView* aView) const;
    bool HasResourceView(const GpuResourceView* aView) const;
    void SetResourceView(int anIndex, const GpuResourceView* aView);
    bool IsDirty() const { return myIsDirty; }
    const eastl::fixed_vector<GpuResourceViewSetElement, 32>& GetResources() const { return myResources; }

    virtual void UpdateDescriptors() const = 0;

  protected:
    void OnViewDestroyed(GpuResourceView* aView);
    eastl::fixed_vector<GpuResourceViewSetElement, 32> myResources;
    mutable bool myIsDirty;
  };
}
