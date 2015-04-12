#include "LightComponent.h"
#include "SceneNode.h"
#include "InputManager.h"

namespace Fancy { namespace Scene { namespace Components {
  //---------------------------------------------------------------------------//
  namespace Internal
  {

  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  LightComponent::LightComponent(SceneNode* _pOwner) :
    SceneNodeComponent(_pOwner)
  {

  }
//---------------------------------------------------------------------------//
  LightComponent::~LightComponent()
  {

  }
//---------------------------------------------------------------------------//
  void LightComponent::init()
  {

  }
//---------------------------------------------------------------------------//
  void LightComponent::update()
  {
    
  }
//---------------------------------------------------------------------------//
  void LightComponent::gatherRenderItems(SceneRenderDescription* pRenderDesc)
  {
    // TODO: Render debug geometry
  }
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Scene::Components
