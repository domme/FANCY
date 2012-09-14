#include "SpotLight.h"
//#include "MATSimpleColor.h"
#include "../Scene/Camera.h"
#include "../Services/NameRegistry.h"


SpotLight::SpotLight() : Light(),
	m_fExponent( 0.0f ),
	m_fPenumbraAngle( 0.0f ),
	m_fUmbraAngle( 0.0f ),
	m_v3Direction( 0.0f, 0.0f, 1.0f )
{
	m_eLightType = Light::LIGHTTYPE_SPOT;
}

SpotLight::~SpotLight()
{

}


void SpotLight::update()
{
	
}


void SpotLight::renderShadowMap()
{	
	
}