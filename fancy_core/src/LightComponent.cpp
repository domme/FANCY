#include "LightComponent.h"
#include "SceneNode.h"
#include "InputManager.h"

namespace Fancy { namespace Scene {
  //---------------------------------------------------------------------------//
  namespace Internal
  {

  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  LightComponent::LightComponent(SceneNode* _pOwner) :
    SceneNodeComponent(_pOwner),
    m_eType(ELightType::DIRECTIONAL),
    m_colorIntensity(1.0f, 1.0f, 1.0f),
    m_castsShadows(false),
    m_falloffStart(0.0f),
    m_falloffEnd(10.0f),
    m_coneAngle(45.0f)
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
} } // end of namespace Fancy::Scene::Components
