#include "SceneNodeComponent.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  SceneNodeComponent::SceneNodeComponent(SceneNode* pOwner) 
    : m_pOwner(pOwner)
  {
   
  }
//---------------------------------------------------------------------------//
  SceneNodeComponent::~SceneNodeComponent()
  {

  }
//---------------------------------------------------------------------------//
   const ObjectName& SceneNodeComponent::getName() const
   {
     return ObjectName::blank;
   }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene