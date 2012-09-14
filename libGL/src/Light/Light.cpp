#include "Light.h"
#include "../Scene/AABoundingBox.h"
#include "../Services/NameRegistry.h"

#include "../Services/FBOservice.h"
#include "../Rendering/Materials/Material.h"

Light::Light() :
m_v3Color( 1.0f, 1.0f, 1.0f ),
m_fIntensity( 1.0f )
{
	Init();
}

Light::~Light()
{
}

void Light::update()
{
	//dummy in base class
}

void Light::prepareRender()
{
	//dummy in Base class
}

void Light::Init()
{
	
}

void Light::renderShadowMap()
{
	//Dummy - will be implemented in the Lights which cast shadows
}


void Light::render()
{
	
}
