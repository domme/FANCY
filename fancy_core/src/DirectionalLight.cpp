#include "DirectionalLight.h"
#include "../Scene/Camera.h"
#include "../Scene/AABoundingBox.h"

DirectionalLight::DirectionalLight() : Light()
{
	m_eLightType = Light::LIGHTTYPE_DIRECTIONAL;
}

DirectionalLight::~DirectionalLight()
{

}

void DirectionalLight::Update()
{
	
}


void DirectionalLight::RenderShadowMap()
{	
	
}