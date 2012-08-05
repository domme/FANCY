#include "GLVolumeRenderer.h"

#include "GLRenderer.h"
#include "../Engine.h"
#include "../Services/FBOservice.h"
#include "../Services/GLDebug.h"

#include "../Geometry/VolumeMesh.h"
#include "Materials/VolumeMaterial.h"
#include "Materials/MAT_VolCube_Raycast_Simple.h"
#include "../Scene/VolumeEntity.h"
#include "../Scene/SceneNode.h"
#include "Managers/TextureManager.h"
#include "../Light/PointLight.h"

GLuint uDSM_drawBuffers[ 8 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
									GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };

GLVolumeRenderer:: GLVolumeRenderer() : 
	m_uScreenWidth( 1 ),
	m_uScreenHeight( 1 ),
	m_pGLrenderer( NULL ),
	m_uFinalOutFBO( GLUINT_HANDLE_INVALID ),
	m_uFinalOutTex( GLUINT_HANDLE_INVALID ),
	m_uRayEndTex( GLUINT_HANDLE_INVALID ),
	m_uRayEndFBO( GLUINT_HANDLE_INVALID ),
	m_uDepthStencilTex( GLUINT_HANDLE_INVALID ),
	m_uDeepShadowMapFBO( GLUINT_HANDLE_INVALID ),
	m_uDeepShadowMapTex( GLUINT_HANDLE_INVALID ),
	m_uRayEndFBO_DSM( GLUINT_HANDLE_INVALID ),
	m_uRayEndTex_DSM( GLUINT_HANDLE_INVALID ),
	m_uDSM_width( 512 ),
	m_uDSM_height( 512 ),
	m_uDSM_depth( 8 ),
	m_pDSMmaterial( NULL )
	{

	}

GLVolumeRenderer::~GLVolumeRenderer()
{
	deleteResolutionDependentResources();

	SAFE_DELETE( m_pDSMmaterial );
}

void GLVolumeRenderer::deleteResolutionDependentResources()
{
	glDeleteTextures( 1, &m_uFinalOutTex );
	glDeleteTextures( 1, &m_uRayEndTex );
	glDeleteTextures( 1, &m_uRayEndTex_DSM );
	glDeleteTextures( 1, &m_uDepthStencilTex );
	glDeleteTextures( 1, &m_uDeepShadowMapTex );
	glDeleteFramebuffers( 1, &m_uFinalOutFBO );
	glDeleteFramebuffers( 1, &m_uRayEndFBO );
	glDeleteFramebuffers( 1, &m_uRayEndFBO_DSM );
	glDeleteFramebuffers( 1, &m_uDeepShadowMapFBO );
}


void GLVolumeRenderer::Init( uint uScreenWidth, uint uScreenHeight, GLRenderer* pGLrenderer )
{
	m_pEngine = &Engine::GetInstance();
	m_pGLrenderer = pGLrenderer;
	m_pFSquad = &FullscreenQuad::getInstance();
	m_pDSMmaterial = new MAT_VolCube_Raycast_DSM();
	m_pDSMmaterial->Init();

	SetResolution( uScreenWidth, uScreenHeight );
}

void GLVolumeRenderer::SetResolution( uint uScreenWidth, uint uScreenHeight )
{
	if( m_uScreenWidth == uScreenWidth && m_uScreenHeight == uScreenHeight )
		return;

	m_uScreenWidth = uScreenWidth;
	m_uScreenHeight = uScreenHeight;

	RUN_NOT_FIRST( deleteResolutionDependentResources() );

	glGenTextures( 1, &m_uDepthStencilTex );
	glBindTexture( GL_TEXTURE_2D, m_uDepthStencilTex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, m_uScreenWidth, m_uScreenHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL );

	
	//////////////////////////////////////////////////////////////////////////
	// RayEnd FBO
	//////////////////////////////////////////////////////////////////////////
	GLDebug::GL_ErrorCheckStart();
	glGenTextures( 1, &m_uRayEndTex );
	glGenFramebuffers( 1, &m_uRayEndFBO );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uRayEndFBO );
	glBindTexture( GL_TEXTURE_2D, m_uRayEndTex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_HALF_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uRayEndTex, 0 );

	glBindTexture( GL_TEXTURE_2D, m_uDepthStencilTex );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_uDepthStencilTex, 0 );
	GLDebug::GL_ErrorCheckFinish();

	FBOservice::checkFBOErrors();

	//////////////////////////////////////////////////////////////////////////
	// RayEnd FBO - DSM
	//////////////////////////////////////////////////////////////////////////
	GLDebug::GL_ErrorCheckStart();
	glGenTextures( 1, &m_uRayEndTex_DSM );
	glGenFramebuffers( 1, &m_uRayEndFBO_DSM );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uRayEndFBO_DSM );
	glBindTexture( GL_TEXTURE_2D, m_uRayEndTex_DSM );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, m_uDSM_width, m_uDSM_height, 0, GL_RGB, GL_HALF_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uRayEndTex_DSM, 0 );

	glBindTexture( GL_TEXTURE_2D, m_uDepthStencilTex );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_uDepthStencilTex, 0 );
	GLDebug::GL_ErrorCheckFinish();

	FBOservice::checkFBOErrors();



	//////////////////////////////////////////////////////////////////////////
	// Final out FBO
	//////////////////////////////////////////////////////////////////////////
	GLDebug::GL_ErrorCheckStart();
	glGenTextures( 1, &m_uFinalOutTex );
	glGenFramebuffers( 1, &m_uFinalOutFBO );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalOutFBO );
	glBindTexture( GL_TEXTURE_2D, m_uFinalOutTex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, m_uScreenWidth, m_uScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uFinalOutTex, 0 );

	glBindTexture( GL_TEXTURE_2D, m_uDepthStencilTex );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_uDepthStencilTex, 0 );
	GLDebug::GL_ErrorCheckFinish();

	FBOservice::checkFBOErrors();


	//////////////////////////////////////////////////////////////////////////
	//	DeepShadowMap FBO
	//////////////////////////////////////////////////////////////////////////
	
	GLDebug::GL_ErrorCheckStart();
	glGenTextures( 1, &m_uDeepShadowMapTex );
	glGenFramebuffers( 1, &m_uDeepShadowMapFBO );
	
	glBindTexture( GL_TEXTURE_3D, m_uDeepShadowMapTex );
	glBindFramebuffer( GL_FRAMEBUFFER, m_uDeepShadowMapFBO );

	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
	
	//glTexImage3D( GL_TEXTURE_3D, 0, GL_RGB8, m_uDSM_width, m_uDSM_height, m_uDSM_depth, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA32F, m_uDSM_width, m_uDSM_height, m_uDSM_depth, 0, GL_RGBA, GL_FLOAT, NULL );
	glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_uDeepShadowMapTex, 0, 0 );
	glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_uDeepShadowMapTex, 0, 1 );
	glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_uDeepShadowMapTex, 0, 2 );
	glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, m_uDeepShadowMapTex, 0, 3 );
	glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, m_uDeepShadowMapTex, 0, 4 );
	glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, m_uDeepShadowMapTex, 0, 5 );
	glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, m_uDeepShadowMapTex, 0, 6 );
	glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT7, m_uDeepShadowMapTex, 0, 7 );

	FBOservice::checkFBOErrors();
	GLDebug::GL_ErrorCheckFinish();
	 
	//Notify materials
	VolumeMaterial::SetRaystartTexture( m_uRayEndTex );
	

	TextureManager::getInstance().DeclareTexture( TextureSemantics::TEX_3D_DSM, m_uDeepShadowMapTex );
}

void GLVolumeRenderer::renderDeepShadowMap( const PointLight* pPointLight, const VolumeEntity* pVolumeEntity, SceneManager* pScene )
{
	const std::unique_ptr<VolumeMesh>& pVolumeMesh = pVolumeEntity->GetVolumeMesh();
	const VolumeMaterial* pVolumeMaterial = pVolumeMesh->GetMaterial();
	const Mesh* pCubeMesh = pVolumeMesh->GetVolumeBoundingMesh();
	Material* pRasterizeMat = pVolumeMesh->GetVolumeBoundingRasterizeMaterial();

	const glm::mat4& rVolumeModelWorldMat = pVolumeEntity->getNode()->getGlobalTransformMAT();
	const Camera* pLightCamera = &pPointLight->m_clLightViewCamera; //m_pEngine->GetCurrentCamera();

	
	//Configure DSM-Material!
	m_pDSMmaterial->SetVolumeTexture( pVolumeMaterial->GetVolumeTexture() );
	m_pDSMmaterial->SetTransferFunctionTexture( pVolumeMaterial->GetTransferFunctionTexture() );
	m_pDSMmaterial->SetVolumeTextureSize( pVolumeMaterial->GetVolumeTextureSize() );
	m_pDSMmaterial->SetSamplingRate( pVolumeMaterial->GetSamplingRate() );
	m_pDSMmaterial->SetWindowValue( pVolumeMaterial->GetWindowValue() );

	VolumeMaterial::SetRaystartTexture( m_uRayEndTex_DSM );

	m_pGLrenderer->saveViewport();
	m_pGLrenderer->setViewport( 0, 0, m_uDSM_width, m_uDSM_height );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uRayEndFBO_DSM );
	glClear( GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	
	//////////////////////////////////////////////////////////////////////////
	//Rasterize Back faces into Raystart texture.
	//m_pGLrenderer->setStencilTest( false );
	m_pGLrenderer->setCulling( true );
	m_pGLrenderer->setCullFace( GL_FRONT );
	m_pGLrenderer->RenderMesh( pCubeMesh, pRasterizeMat->GetForwardShader(), pLightCamera, pScene, rVolumeModelWorldMat );
	//////////////////////////////////////////////////////////////////////////

	glBindFramebuffer( GL_FRAMEBUFFER, m_uDeepShadowMapFBO ); 
	glDrawBuffers( 8, uDSM_drawBuffers );	

	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |GL_STENCIL_BUFFER_BIT );
	glClearColor( m_pEngine->GetClearColor().x, m_pEngine->GetClearColor().y, m_pEngine->GetClearColor().z, 1.0f );
	
	
	//Apply the actual DSM-Material by rendering the front faces
	m_pGLrenderer->setCullFace( GL_BACK );
	m_pGLrenderer->RenderMesh( pCubeMesh, m_pDSMmaterial->GetForwardShader(), pLightCamera, pScene, rVolumeModelWorldMat, m_pDSMmaterial );

	m_pGLrenderer->restoreViewport();
}

void GLVolumeRenderer::RenderScene( SceneManager* pScene )
{
	Camera* pCamera = m_pEngine->GetCurrentCamera();
	const glm::vec4& clearColor = m_pEngine->GetClearColor();

	const std::vector<VolumeEntity*>& vVolumeObjects = pScene->GetVolumeObjects();

	/*
	const std::vector<DirectionalLight*>& vDirLights = pScene->getCachedDirectionalLights();


	const DirectionalLight* pDirLight = NULL;
	if( vDirLights.size() > 0 )
		pDirLight = vDirLights[ 0 ];
	*/
	
	const std::vector<PointLight*>& vPointLights = pScene->getCachedPointLights();
	
	const PointLight* pPointLight = NULL;
	if( vPointLights.size() > 0 )
		pPointLight = vPointLights[ 0 ];


	//TODO: Sort!
	glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalOutFBO );
	glClear( GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uRayEndFBO );
	glClear( GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	


	for( uint uIdx = 0; uIdx < vVolumeObjects.size(); ++uIdx )
	{	
		const VolumeEntity* pVolumeEntity = vVolumeObjects[ uIdx ];
		const std::unique_ptr<VolumeMesh>& pVolumeMesh = pVolumeEntity->GetVolumeMesh();
		VolumeMaterial* pVolumeMaterial = pVolumeMesh->GetMaterial();
		const Mesh* pCubeMesh = pVolumeMesh->GetVolumeBoundingMesh();
		Material* pRasterizeMat = pVolumeMesh->GetVolumeBoundingRasterizeMaterial();

		const glm::mat4& rVolumeModelWorldMat = pVolumeEntity->getNode()->getGlobalTransformMAT();

		//Render the deep shadow map
		if( pPointLight && pVolumeMaterial->GetUseShadows() )
			renderDeepShadowMap( pPointLight, pVolumeEntity, pScene );

		pVolumeMaterial->SetShadowTexture( m_uDeepShadowMapTex );
		
		//Debug the shadow map....
		//glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalOutFBO );
		//m_pFSquad->RenderTimeTexture3D( m_uDeepShadowMapTex );

		glBindFramebuffer( GL_FRAMEBUFFER, m_uRayEndFBO );
		glClear( GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
		
		
		//////////////////////////////////////////////////////////////////////////
		//Rasterize Back faces into Raystart texture.
		VolumeMaterial::SetRaystartTexture( m_uRayEndTex );
		m_pGLrenderer->setCulling( true );
		m_pGLrenderer->setCullFace( GL_FRONT );
		m_pGLrenderer->RenderMesh( pCubeMesh, pRasterizeMat->GetForwardShader(), pCamera, pScene, rVolumeModelWorldMat );
		//////////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////
		//Apply the actual volume Material (Raycasting) by rendering the front faces
		m_pGLrenderer->setCullFace( GL_BACK );
		
		//Rasterize Viewplane
		//TODO: Implement!

		//glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalOutFBO );
		glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalOutFBO );
		m_pGLrenderer->RenderMesh( pCubeMesh, pVolumeMaterial->GetForwardShader(), pCamera, pScene, rVolumeModelWorldMat, pVolumeMaterial );

		
	}	
}