#include "PointLight.h"
#include "../Services/FBOservice.h"

static enum ECubeSides 
{
	//NOTE: enum-entries ALWAYS have to match this order:
	/*
	#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
	#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
	#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
	#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
	#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
	#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
	*/

	CUBE_RIGHT = 0,
	CUBE_LEFT,
	CUBE_TOP,
	CUBE_BOTTOM,
	CUBE_FAR,
	CUBE_NEAR
};

PointLight::PointLight() : Light(),
m_fFalloffStart( 20.0f ),
m_fFalloffEnd( 100.0f ),
m_uShadowCubeDepthTex( GLUINT_HANDLE_INVALID ) 
//m_uShadowFBOs()
{
	m_eLightType = Light::LIGHTTYPE_POINT;

	m_uNumShadowmapPasses = 6;
}

PointLight::~PointLight()
{
	destroyShadowmap();
}

void PointLight::destroyShadowmap()
{
	Light::destroyShadowmap();
	
	glDeleteTextures( 1, &m_uShadowCubeDepthTex );

	glDeleteFramebuffers( 1, &m_uDebugFBO );
	glDeleteTextures( 1, &m_uDebugTex );
}

void PointLight::Update()
{
	
}



void PointLight::Init()
{
	initShadowmap();
	initShadowCamera();
}

void PointLight::initShadowmap()
{
	if( m_bShadowmapInitialized )
	{
		destroyShadowmap();
	}

	m_bShadowmapInitialized = true;

	glEnable(GL_TEXTURE_CUBE_MAP);

	//glGenFramebuffers( 6, m_uShadowFBOs );
	//glGenTextures( 6, m_uShadowDepthTextures );
	//glGenTextures( 6, m_uShadowColorTextures );

	glGenFramebuffers( 1, &m_uShadowmapFBO );
	glGenTextures( 1, &m_uShadowCubeDepthTex );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uShadowmapFBO );
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_uShadowCubeDepthTex );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	/*glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);*/

	
	GLuint depthbuffer;
	glGenRenderbuffers(1, &depthbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_iv2ShadowmapResolution.x, m_iv2ShadowmapResolution.y );
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer );
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	
	
	

	/*byte* faceData = new byte[ 1024 * 1024 * 4 ];

	for( int i = 0; i < 1024 * 1024 * 4; i += 4 )
	{
		faceData[ i ] = 255;
		faceData[ i + 1] = 0;
		faceData[ i + 2 ] = 0;
		faceData[ i + 3 ] = 255;
	} */
	
	for( int i = 0; i < 6; ++i )
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32F, m_iv2ShadowmapResolution.x, m_iv2ShadowmapResolution.y, 0, GL_RED, GL_FLOAT, NULL );

	//for( uint i = 0; i < 6; ++i )
		//glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, m_iv2ShadowmapResolution.x, m_iv2ShadowmapResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );

	//delete[] faceData;
	
	
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_uShadowCubeDepthTex, 0 ); 
	
	//glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_uShadowCubeDepthTex, 0 ); 
	//glDrawBuffer( GL_NONE );
	FBOservice::checkFBOErrors();
	//////////////////////////////////////////////////////////////////////////

	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}



void PointLight::initShadowCamera()
{
	//m_clLightViewCamera.SetProjection( glm::perspective( glm::radians( 90.0f ), 1.0f, 1.0f, 100.0f /*m_fFalloffEnd */ ) );
	//m_clLightViewCamera.InitPerspectiveProjection( glm::radians( 90.0f ), 1.0f, 1.0f, 100.0f );
	m_clLightViewCamera.InitPerspectiveProjection( 90.0f, 1.0f, 1.0f, m_fFalloffEnd );
	
	//////////////////////////////////////////////////////////////////////////
	//Create and store the depth-camera orientations
	m_quatShadowCamOrientations[ CUBE_LEFT ]    = glm::angleAxis( glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ); //Rotate 90 degrees left
	m_quatShadowCamOrientations[ CUBE_RIGHT ]	= glm::angleAxis( glm::radians( -90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ); //Rotate 90 degrees right
	m_quatShadowCamOrientations[ CUBE_TOP ]		= glm::angleAxis( glm::radians( 90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ); //Rotate 90 degrees up
	m_quatShadowCamOrientations[ CUBE_BOTTOM ]	= glm::angleAxis( glm::radians( -90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ); //Rotate 90 degrees down
	m_quatShadowCamOrientations[ CUBE_FAR ]		= glm::angleAxis( glm::radians( 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ); //Don't rotate - just create a valid roatation matrix
	m_quatShadowCamOrientations[ CUBE_NEAR ]	= glm::angleAxis( glm::radians( 180.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ); //Rotate 180 degrees around
	//////////////////////////////////////////////////////////////////////////
}

void PointLight::PrepareShadowmapPass( int uPassIndex )
{
	if( uPassIndex >= m_uNumShadowmapPasses )
		return;

	glm::vec3 v3At;
	glm::vec3 v3Up;

	switch( GL_TEXTURE_CUBE_MAP_POSITIVE_X + uPassIndex )
	{
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
			v3At = m_v3Position + glm::vec3( 1.0f, 0.0f, 0.0f );
			v3Up = glm::vec3( 0.0f, -1.0f, 0.0f );
		break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
			v3At = m_v3Position + glm::vec3( -1.0f, 0.0f, 0.0f );
			v3Up = glm::vec3( 0.0f, -1.0f, 0.0f );
		break;

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
			v3At = m_v3Position + glm::vec3( 0.0f, 1.0f, 0.0f );
			v3Up = glm::vec3( 0.0f, 0.0f, 1.0f ); //Possibly wrong here
		break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			v3At = m_v3Position + glm::vec3( 0.0f, -1.0f, 0.0f );
			v3Up = glm::vec3( 0.0f, 0.0f, -1.0f );
		break;

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
			v3At = m_v3Position + glm::vec3( 0.0f, 0.0f, 1.0f );
			v3Up = glm::vec3( 0.0f, -1.0f, 0.0f );
		break;
		
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			v3At = m_v3Position + glm::vec3( 0.0f, 0.0f, -1.0f );
			v3Up = glm::vec3( 0.0f, -1.0f, 0.0f );
		break;

		default:
			LOG( "ERROR: Undefined Cubemap face specified!" );
		break;
	}

	m_clLightViewCamera.InitView( m_v3Position, v3At, v3Up );
	
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + uPassIndex, m_uShadowCubeDepthTex, 0 );
	
	//glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + uPassIndex, m_uShadowCubeDepthTex, 0 );
	//glDrawBuffer( GL_NONE );

	FBOservice::checkFBOErrors();
	
	//DEBUG:
	//glBindFramebuffer( GL_FRAMEBUFFER, m_uDebugFBO );

}

void PointLight::PostprocessShadowmap()
{

}

void PointLight::onPositionChanged()
{
	m_clLightViewCamera.InitView( m_v3Position, m_v3Position + glm::vec3( 0.0f, 0.0f, 1.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
}
