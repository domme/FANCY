#include "Light.h"
#include "../Scene/AABoundingBox.h"
#include "../Services/NameRegistry.h"

#include "../Services/FBOservice.h"
#include "../Rendering/Materials/Material.h"

Light::Light() :
m_v3Color( 1.0f, 1.0f, 1.0f ),
m_fIntensity( 1.0f ),
m_bCastShadows( true ),
m_bEnabled( true ),
m_v3Position( 0.0f, 0.0f, 0.0f ),
m_iv2ShadowmapResolution( 1024, 1024 ),
m_bShadowmapInitialized( false ),
m_uNumShadowmapPasses( 0 ),
m_bDirty( true )
{
	
}

Light::~Light()
{
}

void Light::Update()
{
	//dummy in base class
}


void Light::Init()
{
	
}

void Light::PrepareShadowmapPass( uint uPassIndex )
{
	//DUMMY in base class
}

void Light::initShadowmap()
{
	//DUMMY in base class
}

void Light::destroyShadowmap()
{
	//DUMMY in base class
}

void Light::SetShadowmapResolution( const glm::ivec2& v2Res )
{
	 m_iv2ShadowmapResolution = v2Res; 
	 
	 //Re-initialize shadowmap
	 initShadowmap();
}

void Light::PostprocessShadowmap()
{
	//DUMMY in base class
}
