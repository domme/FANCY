#ifndef INCLUDE_MODELCOMPONENT_H
#define INCLUDE_MODELCOMPONENT_H

#include "SceneNodeComponent.h"

namespace Fancy { namespace Scene {
  //---------------------------------------------------------------------------//
  class DLLEXPORT ModelComponent : 
    public SceneNodeComponent, public BaseCreator<ModelComponent, SceneNode*>
  {
  public:
    ModelComponent(SceneNode* pOwner);
    virtual ~ModelComponent();

    virtual ObjectName getTypeName() override { return _N(Model); }
    virtual void update();
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(ModelComponent)
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_MODELCOMPONENT_H