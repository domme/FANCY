#ifndef INCLUDE_TRANSFORMCOMPONENT_H
#define INCLUDE_TRANSFORMCOMPONENT_H

#include "SceneNodeComponent.h"

namespace Fancy { namespace Scene {
  //---------------------------------------------------------------------------//
  class DLLEXPORT TransformComponent : public SceneNodeComponent
  {
  public:
    TransformComponent();
    virtual ~TransformComponent();

    virtual void update(float fDt) override;
  };
  //---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_TRANSFORMCOMPONENT_H