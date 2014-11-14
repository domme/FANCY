#include "TransformComponent.h"
#include "SceneNodeComponentFactory.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  TransformComponent::TransformComponent(SceneNode* pOwner) :
    SceneNodeComponent(pOwner),
    m_LocalTransform(1.0f),
    m_CachedWorldTransform(1.0f)
  {
    
  }
//---------------------------------------------------------------------------//
  TransformComponent::~TransformComponent()
  {

  }
//---------------------------------------------------------------------------//
  void TransformComponent::update()
  {

  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene