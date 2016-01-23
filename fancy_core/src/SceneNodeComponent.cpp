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
} }  // end of namespace Fancy::Scene