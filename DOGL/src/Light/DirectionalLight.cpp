#include "DirectionalLight.h"
#include "../Engine.h"
#include "../Scene/Camera.h"
#include "../Scene/AABoundingBox.h"

DirectionalLight::DirectionalLight() : Light()
{
	m_eLightType = Light::LIGHTTYPE_DIRECTIONAL;
}

DirectionalLight::~DirectionalLight()
{

}

void DirectionalLight::update()
{
	
}


void DirectionalLight::renderShadowMap()
{	
	
}