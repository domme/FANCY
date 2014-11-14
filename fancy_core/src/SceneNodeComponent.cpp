#include "SceneNodeComponent.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  SceneNodeComponent::SceneNodeComponent(SceneNode* pOwner) 
    : m_Owner(pOwner)
  {
   
  }
//---------------------------------------------------------------------------//
  SceneNodeComponent::~SceneNodeComponent()
  {

  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene