#include "GLDeferredRenderer.h"

#include "GLRenderer.h"
#include "../IO/FileReader.h"
#include <assert.h>
#include "Materials/Material.h"
#include "../Geometry/VertexDeclarations.h"
#include "Shader.h"
#include "../Engine.h"

#include "../Scene/Camera.h"
#include "GLTexture.h"
#include "../Services/FBOservice.h"
#include "../Services/GLDebug.h"
#include "../Light/Light.h"
#include "../Light/PointLight.h"
#include "../Light/DirectionalLight.h"
#include "../Light/SpotLight.h"
#include "../Geometry/Mesh.h"
#include "../Geometry/Model.h"
#include "../IO/ModelLoader.h"
#include "../Scene/AABoundingBox.h"
#include "Managers/TextureManager.h"

#include "Pass_GaussianBlur.h"

#include "../IO/TextureLoader.h"


#include "Materials/MAT_FSquad_FinalComposite.h"
#include "Materials/MAT_FSquad_DirLighting.h"
#include "Materials/MAT_FSquad_PointLighting.h"
#include "Materials/MAT_Pointlight_Illumination.h"


#ifdef __WINDOWS
#include <random>
#else ifdef __OSX
#include <time.h>
#endif

#include <stdio.h>

const float PI = 3.14159265358979323846f;

const uint uBLURTEX_WIDTH = 128;
const uint uBLURTEX_HEIGHT = 128;

#ifdef __WINDOWS

//uint64 getStartTime()
//{
//	LARGE_INTEGER iTicks;
//
//	QueryPerformanceCounter( &iTicks );
//	uint64 u64Ticks =iTicks.QuadPart;
//	
//	return  u64Ticks;
//}
//
//void logDuration( uint64 u64Start, const char* pMsg )
//{
//	LARGE_INTEGER iFreq;
//	
//	QueryPerformanceFrequency( &iFreq );
//
//	LARGE_INTEGER iTicks;
//
//	QueryPerformanceCounter( &iTicks );
//
//	uint64 u64End = iTicks.QuadPart;
//	uint64 u64Freq = iFreq.QuadPart;
//	
//	
//	uint64 u64duration = u64End - u64Start;
//	
//	double f64Delay = (double) u64duration / ( (double) u64Freq / 1000.0 );
//		
//	char cbuf[1024];
//
//	sprintf( cbuf, "%s %.20f \n", pMsg, f64Delay );
//	fprintf(stdout, "%s", cbuf);
//}

#endif


GLDeferredRenderer::GLDeferredRenderer() :
m_uDeferredDepthStencilTex( 0 ),
m_uDeferredFBO( 0 )
{
	
}

GLDeferredRenderer::~GLDeferredRenderer()
{
	deleteResoultionDependentResources();

	SAFE_DELETE( m_pMAT_FinalComposite );
	SAFE_DELETE( m_pMAT_Dirlight );
	SAFE_DELETE( m_pMAT_Pointlight );
	
}

void GLDeferredRenderer::deleteResoultionDependentResources()
{
	glDeleteFramebuffers( 1, &m_uDeferredFBO );
	
	glDeleteFramebuffers( 1, &m_uRGB32F_FBO_06 );
	glDeleteFramebuffers( 1, &m_uRGB32F_FBO_07 );

	glDeleteTextures( GBuffer::num, m_uGBuffer );
	glDeleteTextures( 1, &m_uDeferredDepthStencilTex );
	
	glDeleteTextures( 1, &m_uRGB32F_Tex_06 );
	glDeleteTextures( 1, &m_uRGB32F_Tex_07 );
}


void GLDeferredRenderer::Init( uint uWidth, uint uHeight, GLRenderer* glRenderer )
{
	m_pEngine = &Engine::GetInstance();
	m_pTextureManager = &TextureManager::getInstance();
	m_pFSquad = &FullscreenQuad::getInstance();

	m_pMAT_FinalComposite = new MAT_FSquad_FinalComposite();
	m_pMAT_FinalComposite->Init();

	m_pGLrenderer = glRenderer;
	
	m_pMAT_Dirlight = new MAT_FSquad_DirLighting();
	m_pMAT_Dirlight->Init();

	m_pMAT_Pointlight = new MAT_FSquad_PointLighting();
	m_pMAT_Pointlight->Init();
	
	Mesh* pPointlightMesh = ModelLoader::GetInstance().LoadSingleMeshGeometry( "Models/Helper/PointlightBounds.obj" );
	m_pPointlightMesh = unique_ptr<Mesh>( std::move( pPointlightMesh ) );

	MAT_Pointlight_Illumination* pMat = new MAT_Pointlight_Illumination();
	pMat->Init();
	m_pPointlightMesh->SetMaterial( pMat );

	//TEST
	//m_uDicomTestImage = TextureLoader::GetInstance().LoadTexture3D( "Textures/Volumes/HeadTorso/HeadTorso", 100, 460, "pbm" );	
	//m_uDicomTestImage = TextureLoader::GetInstance().LoadTexture2D( "Textures/Volumes/HeadTorso/HeadTorso001.pbm" );
}

void GLDeferredRenderer::SetResolution( uint uWidth, uint uHeight )
{
	m_uScreenWidth = uWidth;
	m_uScreenHeight = uHeight;

	updateTextures();
}

void GLDeferredRenderer::updateTextures()
{
	static bool bTexturesInit = false;
	
	if( bTexturesInit )
	{
		//Delete textures and FBOs to account for new resolution
		deleteResoultionDependentResources();
	}

	if( m_uScreenWidth == 0 )
		m_uScreenWidth = 1;

	if( m_uScreenHeight == 0 )
		m_uScreenHeight = 1;

	bTexturesInit = true;

	
	glGenFramebuffers( 1, &m_uDeferredFBO );
	
	glGenFramebuffers( 1, &m_uRGB32F_FBO_06 );
	glGenFramebuffers( 1, &m_uRGB32F_FBO_07 );
	


	glGenTextures( GBuffer::num, m_uGBuffer );
	glGenTextures( 1, &m_uDeferredDepthStencilTex );
	
	glGenTextures( 1, &m_uRGB32F_Tex_06 );
	glGenTextures( 1, &m_uRGB32F_Tex_07 );
	

	glBindFramebuffer( GL_FRAMEBUFFER, m_uDeferredFBO );
	
	glBindTexture( GL_TEXTURE_2D, m_uGBuffer[ GBuffer::ColorGloss ] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, m_uScreenWidth, m_uScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uGBuffer[ GBuffer::ColorGloss ], 0 );

	glBindTexture( GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Spec ] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, m_uScreenWidth, m_uScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Spec ], 0 );

	glBindTexture( GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Normal ] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Normal ], 0 );

	glBindTexture( GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Depth ] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F, m_uScreenWidth, m_uScreenHeight, 0, GL_RED, GL_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Depth ], 0 );

	glBindTexture( GL_TEXTURE_2D, m_uDeferredDepthStencilTex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, m_uScreenWidth, m_uScreenHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_uDeferredDepthStencilTex, 0 );

	FBOservice::checkFBOErrors();

		

	//////////////////////////////////////////////////////////////////////////
	// Lighting FBO
	//////////////////////////////////////////////////////////////////////////
	glBindFramebuffer( GL_FRAMEBUFFER, m_uRGB32F_FBO_06 );
	glBindTexture( GL_TEXTURE_2D, m_uRGB32F_Tex_06 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uRGB32F_Tex_06, 0 );

	//Use the same depth/Stencil texture as for the G-Buffer FBO
	glBindTexture( GL_TEXTURE_2D, m_uDeferredDepthStencilTex );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_uDeferredDepthStencilTex, 0 );

	FBOservice::checkFBOErrors();

	//////////////////////////////////////////////////////////////////////////
	// Final Composite FBO
	//////////////////////////////////////////////////////////////////////////
	glBindFramebuffer( GL_FRAMEBUFFER, m_uRGB32F_FBO_07 );
	glBindTexture( GL_TEXTURE_2D, m_uRGB32F_Tex_07 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uRGB32F_Tex_07, 0 );

	//Use the same depth/Stencil texture as for the G-Buffer FBO
	glBindTexture( GL_TEXTURE_2D, m_uDeferredDepthStencilTex );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_uDeferredDepthStencilTex, 0 );

	FBOservice::checkFBOErrors();

	
	
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	

	//Register the textures
	m_pTextureManager->DeclareTexture( TextureSemantics::GBUFFER_COLOR_GLOSS, m_uGBuffer[ GBuffer::ColorGloss ] );
	m_pTextureManager->DeclareTexture( TextureSemantics::GBUFFER_SPECULAR, m_uGBuffer[ GBuffer::Spec ] );
	m_pTextureManager->DeclareTexture( TextureSemantics::GBUFFER_NORMAL, m_uGBuffer[ GBuffer::Normal ] );
	m_pTextureManager->DeclareTexture( TextureSemantics::GBUFFER_DEPTH, m_uGBuffer[ GBuffer::Depth ] );
	m_pTextureManager->DeclareTexture( TextureSemantics::DEFERRED_DEPTHSTENCIL, m_uDeferredDepthStencilTex );
	m_pTextureManager->DeclareTexture( TextureSemantics::LIGHTING, m_uLightingTex_06 );
	m_pTextureManager->DeclareTexture( TextureSemantics::FINAL_COMPOSITE, m_uFinalCompositeTex_07 );


	//Notify the materials
	m_pMAT_Dirlight->SetColorGlossTex( m_uGBuffer[ GBuffer::ColorGloss ] );
	m_pMAT_Dirlight->SetDepthTex( m_uGBuffer[ GBuffer::Depth ] );
	m_pMAT_Dirlight->SetSpecTex( m_uGBuffer[ GBuffer::Spec ] );
	m_pMAT_Dirlight->SetNormalTex( m_uGBuffer[ GBuffer::Normal ] );

	m_pMAT_Pointlight->SetColorGlossTex( m_uGBuffer[ GBuffer::ColorGloss ] );
	m_pMAT_Pointlight->SetDepthTex( m_uGBuffer[ GBuffer::Depth ] );
	m_pMAT_Pointlight->SetSpecTex( m_uGBuffer[ GBuffer::Spec ] );
	m_pMAT_Pointlight->SetNormalTex( m_uGBuffer[ GBuffer::Normal ] );

	m_pMAT_FinalComposite->SetColorGlossTex( m_uGBuffer[ GBuffer::ColorGloss] );
	m_pMAT_FinalComposite->SetLocalIllumTex( m_uLightingTex_06 );	
}

GLDeferredRenderer& GLDeferredRenderer::GetInstance()
{
	static GLDeferredRenderer instance;
	return instance;
}

float haltonNumber( int base, int index )
{
	int currIndex = 1;
	int digit = 0;
	float fHalton = 0.0f;
	
	do 
	{
		digit = index % base;
		index /= base;

		fHalton += glm::pow( (float) base, (float) -currIndex ) * (float)digit;
		currIndex++;
	} 
	while ( index );

	return fHalton;
}


void GLDeferredRenderer::renderEntity( const Entity* pEntity, const SceneManager* pScene, const Camera *pCamera )
{
	const std::unique_ptr<Mesh>& pMesh = pEntity->GetMesh();

	if( !pMesh )
		return;

	Shader* pShader =  pMesh->GetMaterial()->GetDeferredShader();
	
	if( !pShader )
		return;
		
	m_pGLrenderer->RenderMesh( pMesh._Myptr, pShader, pCamera, pScene, pEntity->getNode()->getGlobalTransformMAT() );
}



void GLDeferredRenderer::renderDirLight( DirectionalLight* pLight, const SceneManager* pScene, const Camera* pCamera )
{
	m_pFSquad->RenderWithMaterial( m_pMAT_Dirlight );
}

void GLDeferredRenderer::renderPointLight( PointLight* pLight, const SceneManager* pScene, const Camera* pCamera )
{
	const glm::mat4& matLightModelWorld = pLight->getNode()->getGlobalTransformMAT();
	glm::vec3 v3LightPos = glm::vec3( matLightModelWorld[ 3 ][ 0 ], matLightModelWorld[ 3 ][ 1 ], matLightModelWorld[ 3 ][ 2 ] );
	const glm::vec3& v3CamPos = pCamera->getPosition();

	float fCamLightDistance = glm::length( v3LightPos - v3CamPos ); 
	
	//Mark all pixels occupied by the backside AND Occluded by geometry
	// Stencil 1 -> 2
	m_pGLrenderer->setCulling(true);
	m_pGLrenderer->setCullFace( GL_FRONT );
	m_pGLrenderer->setDepthTest( true );
	m_pGLrenderer->setDepthMask( false );
	m_pGLrenderer->setStencilTest( true );
	m_pGLrenderer->setColorMask( false, false, false, false );
	m_pGLrenderer->setStencilFunc( GL_EQUAL, 1, m_pGLrenderer->m_uStencilMask );
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_INCR, GL_KEEP );

	m_pGLrenderer->RenderMesh( m_pPointlightMesh._Myptr, m_pPointlightMesh->GetMaterial()->GetForwardShader(), pCamera, pScene, matLightModelWorld );

	//De-Mark pixels in front of the front side of the bounding region
	// Stencil 2 -> 1
	m_pGLrenderer->setCullFace( GL_BACK );
	m_pGLrenderer->setStencilFunc( GL_EQUAL, 2, m_pGLrenderer->m_uStencilMask );
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_DECR, GL_KEEP );

	m_pGLrenderer->RenderMesh(  m_pPointlightMesh._Myptr, m_pPointlightMesh->GetMaterial()->GetForwardShader(), pCamera, pScene, matLightModelWorld );
	
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	m_pGLrenderer->setDepthTest( false );
	m_pGLrenderer->setColorMask( true, true, true, true );

	m_pFSquad->RenderWithMaterial( m_pMAT_Pointlight );
}


void GLDeferredRenderer::RenderScene( SceneManager* pSceneManager )
{
	Camera* pCamera = m_pEngine->GetCurrentCamera();
	const glm::vec4& clearColor = m_pEngine->GetClearColor();

	//////////////////////////////////////////////////////////////////////////
	//G-Buffer Pass
	//////////////////////////////////////////////////////////////////////////
	m_pGLrenderer->setDepthTest( true );
	m_pGLrenderer->setDepthMask( GL_TRUE );
	m_pGLrenderer->setBlending( false );
	m_pGLrenderer->setCulling( true );
	m_pGLrenderer->setCullFace( GL_BACK );
	m_pGLrenderer->setStencilTest( true );
	m_pGLrenderer->setStencilFunc( GL_ALWAYS, 1, 0xFFFFFFFF );
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
	

	static GLenum eDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	STATIC_ASSERT( ARRAY_LENGTH( eDrawBuffers ) == GBuffer::num );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uDeferredFBO );

	glDrawBuffers( GBuffer::num, eDrawBuffers );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
		
	const std::vector<Entity*>& vRenderObjects = pSceneManager->GetRenderObjects();

	for( uint i = 0; i < vRenderObjects.size(); ++i )
	{
		renderEntity( vRenderObjects[ i ], pSceneManager, pCamera );
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Lights Passes
	//////////////////////////////////////////////////////////////////////////
	const std::vector<DirectionalLight*>& vDirLights = pSceneManager->getCachedDirectionalLights();
	const std::vector<PointLight*>& vPointLights = pSceneManager->getCachedPointLights();
	const std::vector<SpotLight*>& vSpotLights = pSceneManager->getCachedSpotLights();

	glBindFramebuffer( GL_FRAMEBUFFER, m_uLightingFBO_06 ); 
	static GLenum eDrawBuffer[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, eDrawBuffers );
	
	const glm::vec4& amb = m_pEngine->GetAmbientLightColor();
	glClearColor( amb.r, amb.g, amb.b, amb.a );
	glClear( GL_COLOR_BUFFER_BIT );
	glClearColor( clearColor.r, clearColor.g, clearColor.b, clearColor.a );

	m_pGLrenderer->setBlending( true );
	m_pGLrenderer->setBlendFunc( GL_ONE, GL_ONE );
	
	m_pGLrenderer->setDepthTest( false );
	m_pGLrenderer->setDepthMask( GL_FALSE );
		
	m_pGLrenderer->setStencilTest( true );
	m_pGLrenderer->setStencilFunc( GL_EQUAL, m_pGLrenderer->m_iStencilRef, m_pGLrenderer->m_uStencilMask );
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	
	m_pGLrenderer->SetLightModeActive( true );
	for( uint i = 0; i < vDirLights.size(); ++i )
	{
		m_pGLrenderer->m_uCurrLightIdx = i;
		m_pGLrenderer->prepareLightRendering( pCamera, pSceneManager );
		renderDirLight( vDirLights[ i ], pSceneManager, pCamera );	
	}
	
	for( uint i = 0; i < vPointLights.size(); ++i )
	{
		m_pGLrenderer->m_uCurrLightIdx = vDirLights.size() + i;
		m_pGLrenderer->prepareLightRendering( pCamera, pSceneManager );
		renderPointLight( vPointLights[ i ], pSceneManager, pCamera );
	}

	m_pGLrenderer->m_uCurrLightIdx = 0;
	m_pGLrenderer->SetLightModeActive( false );

	
	//////////////////////////////////////////////////////////////////////////
	//Screen-space passes
	//////////////////////////////////////////////////////////////////////////
	glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalCompositeFBO_07 );
	glClear( GL_COLOR_BUFFER_BIT );
	
	m_pGLrenderer->setDepthTest( false );
	m_pGLrenderer->setDepthMask( GL_FALSE );
	m_pGLrenderer->setBlending( false );
	m_pGLrenderer->setStencilTest( true );
	m_pGLrenderer->setStencilFunc( GL_LEQUAL, 1, m_pGLrenderer->m_uStencilMask );
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	
	m_pFSquad->RenderWithMaterial( m_pMAT_FinalComposite );
}
