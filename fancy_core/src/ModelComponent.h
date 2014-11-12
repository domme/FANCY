#ifndef INCLUDE_MODELCOMPONENT_H
#define INCLUDE_MODELCOMPONENT_H

#include "SceneNodeComponent.h"

namespace Fancy { namespace Scene {
  //---------------------------------------------------------------------------//
  class DLLEXPORT ModelComponent : public SceneNodeComponent
  {
  public:
    ModelComponent();
    virtual ~ModelComponent();

    virtual void update(float fDt) = 0;
  };
  //---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_MODELCOMPONENT_H