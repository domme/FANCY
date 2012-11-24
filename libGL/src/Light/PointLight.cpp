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

	glBindTexture( GL_TEXTURE_CUBE_MAP, m_uShadowCubeDepthTex );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0 );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0 );
	/*glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL); */
	
	//for( uint i = 0; i < 6; ++i )
		//glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32, m_iv2ShadowmapResolution.x, m_iv2ShadowmapResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );

	/*byte* faceData = new byte[ 1024 * 1024 * 4 ];

	for( int i = 0; i < 1024 * 1024 * 4; i += 4 )
	{
		faceData[ i ] = 255;
		faceData[ i + 1] = 0;
		faceData[ i + 2 ] = 0;
		faceData[ i + 3 ] = 255;
	} */
	
	for( int i = 0; i < 6; ++i )
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, m_iv2ShadowmapResolution.x, m_iv2ShadowmapResolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );

	//delete[] faceData;
	
	glBindFramebuffer( GL_FRAMEBUFFER, m_uShadowmapFBO );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_uShadowCubeDepthTex, 0 ); 
	//glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_uShadowCubeDepthTex, 0 );

	//glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_uShadowCubeDepthTex, 0 ); 
	
	//glDrawBuffer( GL_NONE );
	FBOservice::checkFBOErrors();
	//////////////////////////////////////////////////////////////////////////

	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 ); 


	//DEBUG:
	//////////////////////////////////////////////////////////////////////////
	/*glGenFramebuffers( 1, &m_uDebugFBO );
	glGenTextures( 1, &m_uDebugTex );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uDebugFBO );
	glBindTexture( GL_TEXTURE_2D, m_uDebugTex );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, m_iv2ShadowmapResolution.x, m_iv2ShadowmapResolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uDebugTex, 0 );
	FBOservice::checkFBOErrors();

	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 ); */
	//////////////////////////////////////////////////////////////////////////
}



void PointLight::initShadowCamera()
{
	m_clLightViewCamera.SetView( glm::mat4( 1.0f ) ); //Set the identity as a dummy here because it will be set in the renderShadowMap-method
	m_clLightViewCamera.SetProjection( glm::perspective( glm::radians( 90.0f ), 1.0f, 0.1f, m_fFalloffEnd ) );

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

	glm::mat4 matView( 1.0f );
	matView[ 3 ][ 0 ] = m_v3Position.x;
	matView[ 3 ][ 1 ] = m_v3Position.y;
	matView[ 3 ][ 2 ] = m_v3Position.z;

	glm::mat3 matRot = glm::toMat3( m_quatShadowCamOrientations[ uPassIndex ] ); //TODO: store matrices instead of quats to get rid of this conversion
	matView[ 0 ][ 0 ] = matRot[ 0 ][ 0 ];
	matView[ 1 ][ 0 ] = matRot[ 1 ][ 0 ];
	matView[ 2 ][ 0 ] = matRot[ 2 ][ 0 ];

	matView[ 0 ][ 1 ] = matRot[ 0 ][ 1 ];
	matView[ 1 ][ 1 ] = matRot[ 1 ][ 1 ];
	matView[ 2 ][ 1 ] = matRot[ 2 ][ 1 ];

	matView[ 0 ][ 2 ] = matRot[ 0 ][ 2 ];
	matView[ 1 ][ 2 ] = matRot[ 1 ][ 2 ];
	matView[ 2 ][ 2 ] = matRot[ 2 ][ 2 ];

	matView = glm::inverse( matView );

	m_clLightViewCamera.SetView( matView );

	//glActiveTexture( GL_TEXTURE0 );
	//glBindTexture( GL_TEXTURE_CUBE_MAP, m_uShadowCubeDepthTex );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + uPassIndex, m_uShadowCubeDepthTex, 0 );
	//glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_uShadowCubeDepthTex, uPassIndex );
	//glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + uPassIndex, m_uShadowCubeDepthTex, 0 );
	
	//glDrawBuffer( GL_NONE );
	
	FBOservice::checkFBOErrors();
	
	//DEBUG:
	//glBindFramebuffer( GL_FRAMEBUFFER, m_uDebugFBO );

}

void PointLight::PostprocessShadowmap()
{

}
