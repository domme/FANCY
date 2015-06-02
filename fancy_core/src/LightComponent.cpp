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
  void LightComponent::serialize(IO::Serializer& aSerializer)
  {
    aSerializer.beginType(getTypeName(), 0u);

    aSerializer & m_eType;
    aSerializer & m_colorIntensity;
    aSerializer & m_castsShadows;
    aSerializer & m_falloffStart;
    aSerializer & m_falloffEnd;
    aSerializer & m_coneAngle;
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
