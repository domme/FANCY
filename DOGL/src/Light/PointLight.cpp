#include "PointLight.h"

PointLight::PointLight() : Light(),
m_fFalloffStart( 20.0f ),
m_fFalloffEnd( 100.0f )
{
	m_eLightType = Light::LIGHTTYPE_POINT;
}

PointLight::~PointLight()
{

}

void PointLight::update()
{
	
}
