#include "SpotLight.h"
//#include "MATSimpleColor.h"
#include "../Engine.h"
#include "../Scene/Camera.h"
#include "../Services/NameRegistry.h"
#include "../Scene/SceneNode.h"

SpotLight::SpotLight() : Light(),
	m_fExponent( 0.0f ),
	m_fPenumbraAngle( 0.0f ),
	m_fUmbraAngle( 0.0f ),
	m_v3CachedDirection( 0.0f, 0.0f, 1.0f )
{
	m_eLightType = Light::LIGHTTYPE_SPOT;
}

SpotLight::~SpotLight()
{

}

void SpotLight::onTransformChanged( const glm::mat4& newTransform )
{
	m_clLightViewCamera.SetView( glm::inverse( newTransform ) );
}

void SpotLight::update()
{
	
}

void SpotLight::onAttatchedToNode()
{
	m_clLightViewCamera.SetView( glm::inverse( m_pNode->getGlobalTransformMAT() ) );
	float fSideOffset = glm::tan( m_fUmbraAngle );
	fSideOffset *= 0.9;
	m_clLightViewCamera.InitPerspectiveProjection( -fSideOffset, fSideOffset, -fSideOffset, fSideOffset, 1.0f, 1000.0f );
}

void SpotLight::renderShadowMap()
{	
	
}