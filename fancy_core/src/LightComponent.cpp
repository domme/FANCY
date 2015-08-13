#include "LightComponent.h"
#include "SceneNode.h"
#include "InputManager.h"
#include "Serializer.h"

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
  void LightComponent::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_eType, "m_eType");
    aSerializer->serialize(&m_colorIntensity, "m_colorIntensity");
    aSerializer->serialize(&m_castsShadows, "m_castsShadows");
    aSerializer->serialize(&m_falloffStart, "m_falloffStart");
    aSerializer->serialize(&m_falloffEnd, "m_falloffEnd");
    aSerializer->serialize(&m_coneAngle, "m_coneAngle");
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
