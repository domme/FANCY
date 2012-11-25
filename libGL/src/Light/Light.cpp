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
m_bDirty( true ),
m_uShadowmapFBO( GLUINT_HANDLE_INVALID )
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

void Light::PrepareShadowmapPass( int uPassIndex )
{
	//DUMMY in base class
}

void Light::initShadowmap()
{
	//DUMMY in base class
}

void Light::destroyShadowmap()
{
	glDeleteFramebuffers( 1, &m_uShadowmapFBO );
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

void Light::SetPosition( const glm::vec3& rPos )
{
	 m_v3Position = rPos;
	 onPositionChanged();
}

void Light::onPositionChanged()
{
	//dummy in base class
}


