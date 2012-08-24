#include "Light.h"
#include "../Scene/AABoundingBox.h"
#include "../Services/NameRegistry.h"
#include "../Engine.h"

#include "../Services/FBOservice.h"
#include "../Rendering/Materials/Material.h"

Light::Light() :
m_v3Color( 1.0f, 1.0f, 1.0f ),
m_fIntensity( 1.0f ),
m_vChachedPosition( glm::vec3( 0.0f, 0.0f, 0.0f ) )
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
	m_pEngine = &Engine::GetInstance();
}

void Light::renderShadowMap()
{
	//Dummy - will be implemented in the Lights which cast shadows
}

void Light::onTransformChanged( const glm::mat4& newTransform )
{
	//dummy in base class
}


void Light::render()
{
	
}
