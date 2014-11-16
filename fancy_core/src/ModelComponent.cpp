#include "ModelComponent.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  ModelComponent::ModelComponent(SceneNode* pOwner)
    : SceneNodeComponent(pOwner),
    m_pModel(nullptr)
  {

  }
//---------------------------------------------------------------------------//
  ModelComponent::~ModelComponent()
  {

  }
//---------------------------------------------------------------------------//
  void ModelComponent::update()
  {
    
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene