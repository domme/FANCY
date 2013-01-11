#include <assert.h>

//LibGL-Includes.......
#include <Rendering/GLRenderer.h>
#include <Rendering/Materials/Material.h>
#include <Geometry/VertexDeclarations.h>
#include <Rendering/Shader.h>
#include <Scene/Camera.h>
#include <Rendering/GLTexture.h>
#include <Services/FBOservice.h>
#include <Services/GLDebug.h>
#include <Light/Light.h>
#include <Light/PointLight.h>
#include <Light/DirectionalLight.h>
#include <Light/SpotLight.h>
#include <Geometry/Mesh.h>
#include <IO/ModelLoader.h>
#include <Scene/AABoundingBox.h>
#include <Rendering/Managers/TextureManager.h>
#include <IO/TextureLoader.h>
#include <Debug/PerformanceCheck.h>


#include "Materials/MAT_FSquad_FinalComposite.h"
#include "Materials/MAT_FSquad_DirLighting.h"
#include "Materials/MAT_FSquad_PointLighting.h"
#include "Materials/MAT_Pointlight_Illumination.h"
#include "Materials/MAT_FSquad_FXAA.h"
#include "Materials/MAT_FSquad_ToneMapping.h"
#include "Materials/MAT_FSquad_LumaTimeAdaption.h"
#include "Materials/MAT_FSquad_GaussianBlur.h"
#include "Materials/MAT_FSquad_BrightPass.h"
#include "Materials/MAT_FSquad_Bloom.h"

#include "GLDeferredRenderer.h"
#include "../Engine.h"
#include "Pass_GaussianBlur.h"

#ifdef __WINDOWS
#include <random>
#elif defined __OSX
#include <time.h>
#endif

#include <stdio.h>

const float PI = 3.14159265358979323846f;

const uint uBLURTEX_WIDTH = 128;
const uint uBLURTEX_HEIGHT = 128;


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
	SAFE_DELETE( m_pMAT_FXAA );
	SAFE_DELETE( m_pMAT_ToneMap );
	SAFE_DELETE( m_pMAT_LumaTimeAdaption );
	SAFE_DELETE( m_pMAT_GaussianBlur );
	SAFE_DELETE( m_pMAT_Bloom );
	SAFE_DELETE( m_pMAT_BrightPass );
	
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

	glDeleteFramebuffers( 1, &m_uSmall_RGB32F_FBO_01 );
	glDeleteFramebuffers( 1, &m_uSmall_RGB32F_FBO_02 );
	glDeleteFramebuffers( 1, &m_uSmall_LUM32F_FBO_03 );
	glDeleteFramebuffers( 1, &m_uSmall_LUM32F_FBO_04 );
	glDeleteFramebuffers( 1, &m_uSmall_LUM32F_FBO_05 );
	glDeleteFramebuffers( 1, &m_uRGB32F_FBO_08 );
	glDeleteFramebuffers( 1, &m_uRGB32F_FBO_09 );

	glDeleteTextures( 1, &m_uSmall_RGB32F_Tex_01 );
	glDeleteTextures( 1, &m_uSmall_RGB32F_Tex_02 );
	glDeleteTextures( 1, &m_uSmall_LUM32F_Tex_03 );
	glDeleteTextures( 1, &m_uSmall_LUM32F_Tex_04 );
	glDeleteTextures( 1, &m_uSmall_LUM32F_Tex_05 );

	glDeleteTextures( 1, &m_uRGB32F_Tex_08 );
	glDeleteTextures( 1, &m_uRGB32F_Tex_09 );
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
	m_pPointlightMesh = pPointlightMesh;

	MAT_Pointlight_Illumination* pMat = new MAT_Pointlight_Illumination();
	pMat->Init();
	m_pPointlightMesh->SetMaterial( pMat );

	m_pTextureManager = &TextureManager::getInstance();


	m_pPassGaussianBlur = &Pass_GaussianBlur::GetInstance();
#ifdef __WINDOWS
	m_pPassGaussianBlur->Init( 5, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT );
#elif defined __OSX
    m_pPassGaussianBlur->Init( 5, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F_ARB, GL_RGB, GL_FLOAT );
#endif

	m_pGLrenderer->setDepthFunc( GL_LESS );
	m_pGLrenderer->setDepthTest( true );

	m_pMAT_FXAA = new MAT_FSquad_FXAA();
	m_pMAT_FXAA->Init();

	m_pMAT_ToneMap = new MAT_FSquad_ToneMapping();
	m_pMAT_ToneMap->Init();

	m_pMAT_LumaTimeAdaption = new MAT_FSquad_LumaTimeAdaption();
	m_pMAT_LumaTimeAdaption->Init();

	m_pMAT_GaussianBlur = new MAT_FSquad_GaussianBlur();
	m_pMAT_GaussianBlur->Init();

	m_pMAT_BrightPass = new MAT_FSquad_BrightPass();
	m_pMAT_BrightPass->Init();

	m_pMAT_Bloom = new MAT_FSquad_Bloom();
	m_pMAT_Bloom->Init();

	OnResolutionChanged( glm::ivec2( uWidth, uHeight ) );

	//TEST
	//m_uDicomTestImage = TextureLoader::GetInstance().LoadTexture3D( "Textures/Volumes/HeadTorso/HeadTorso", 100, 460, "pbm" );	
	//m_uDicomTestImage = TextureLoader::GetInstance().LoadTexture2D( "Textures/Volumes/HeadTorso/HeadTorso001.pbm" );
}


void GLDeferredRenderer::OnResolutionChanged( glm::ivec2 vRes )
{
	m_uScreenWidth = vRes.x;
	m_uScreenHeight = vRes.y;

	updateTextures();

	updatePostproMaterials();
}


void GLDeferredRenderer::updatePostproMaterials()
{
	//Notify Postpro-materials of the new textures:
	m_pMAT_BrightPass->SetInputTexture( m_uBloomTex_01 );

	m_pMAT_Bloom->SetBloomTexture( m_uBloomTex_01 );

	m_pMAT_LumaTimeAdaption->SetLumaTextureLocs( m_uCurrLuminanceTex_04, m_uLastLuminanceTex_05 );

	m_pMAT_ToneMap->SetInputTextureLoc( m_uFinalBeforeTonemapTex_08 );
	m_pMAT_ToneMap->SetAvgLuminanceTextureLoc( m_uSmall_LUM32F_Tex_03 );

	m_pMAT_FXAA->SetInputTexture( m_uFinalTonemappedTex_09 );
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


	glGenFramebuffers( 1, &m_uSmall_RGB32F_FBO_01 );
	glGenFramebuffers( 1, &m_uSmall_RGB32F_FBO_02 );
	glGenFramebuffers( 1, &m_uSmall_LUM32F_FBO_03 );
	glGenFramebuffers( 1, &m_uSmall_LUM32F_FBO_04 );
	glGenFramebuffers( 1, &m_uSmall_LUM32F_FBO_05 );
	glGenFramebuffers( 1, &m_uRGB32F_FBO_08 );
	glGenFramebuffers( 1, &m_uRGB32F_FBO_09 );

	glGenTextures( 1, &m_uSmall_RGB32F_Tex_01 );
	glGenTextures( 1, &m_uSmall_RGB32F_Tex_02 );
	glGenTextures( 1, &m_uSmall_LUM32F_Tex_03 );
	glGenTextures( 1, &m_uSmall_LUM32F_Tex_04 );
	glGenTextures( 1, &m_uSmall_LUM32F_Tex_05 );

	glGenTextures( 1, &m_uRGB32F_Tex_08 );
	glGenTextures( 1, &m_uRGB32F_Tex_09 );
	

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
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, m_uScreenWidth, m_uScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Normal ], 0 );

	glBindTexture( GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Depth ] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE32F_ARB, m_uScreenWidth, m_uScreenHeight, 0, GL_LUMINANCE, GL_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Depth ], 0 );

	//glBindTexture( GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Pos ] );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
	//glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_uGBuffer[ GBuffer::Pos ], 0 );

	glBindTexture( GL_TEXTURE_2D, m_uDeferredDepthStencilTex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, m_uScreenWidth, m_uScreenHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_uDeferredDepthStencilTex, 0 );

	FBOservice::checkFBOErrors();

	//////////////////////////////////////////////////////////////////////////
	//Bloom-FBOs
	//////////////////////////////////////////////////////////////////////////
	glBindFramebuffer( GL_FRAMEBUFFER, m_uSmall_RGB32F_FBO_01 );
	glBindTexture( GL_TEXTURE_2D, m_uSmall_RGB32F_Tex_01 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
#ifdef __WINDOWS
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL );
#elif defined __OSX
    	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL );
#endif
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uSmall_RGB32F_Tex_01, 0 );

	FBOservice::checkFBOErrors();


	glBindFramebuffer( GL_FRAMEBUFFER, m_uSmall_RGB32F_FBO_02 );
	glBindTexture( GL_TEXTURE_2D, m_uSmall_RGB32F_Tex_02 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
#ifdef __WINDOWS
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL );
#elif defined __OSX
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL );
#endif
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uSmall_RGB32F_Tex_02, 0 );


	FBOservice::checkFBOErrors();


	//////////////////////////////////////////////////////////////////////////
	// Average Luminance FBOs
	//////////////////////////////////////////////////////////////////////////
	//Final Luminance
	glBindFramebuffer( GL_FRAMEBUFFER, m_uSmall_LUM32F_FBO_03 );
	glBindTexture( GL_TEXTURE_2D, m_uSmall_LUM32F_Tex_03 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE32F_ARB, 512, 512, 0, GL_LUMINANCE, GL_FLOAT, NULL );
	glGenerateMipmap( GL_TEXTURE_2D );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uSmall_LUM32F_Tex_03, 0 );

	FBOservice::checkFBOErrors();

	//Curr Luminance
	glBindFramebuffer( GL_FRAMEBUFFER, m_uSmall_LUM32F_FBO_04 );
	glBindTexture( GL_TEXTURE_2D, m_uSmall_LUM32F_Tex_04 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE32F_ARB, 512, 512, 0, GL_LUMINANCE, GL_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uSmall_LUM32F_Tex_04, 0 );

	FBOservice::checkFBOErrors();

	//Last Luminance
	glBindFramebuffer( GL_FRAMEBUFFER, m_uSmall_LUM32F_FBO_05 );
	glBindTexture( GL_TEXTURE_2D, m_uSmall_LUM32F_Tex_05 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE32F_ARB, 512, 512, 0, GL_LUMINANCE, GL_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uSmall_LUM32F_Tex_05, 0 );

	FBOservice::checkFBOErrors();

	//////////////////////////////////////////////////////////////////////////
	//Final before Tone mapping
	//////////////////////////////////////////////////////////////////////////
	glBindFramebuffer( GL_FRAMEBUFFER, m_uRGB32F_FBO_08 );
	glBindTexture( GL_TEXTURE_2D, m_uRGB32F_Tex_08 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
#ifdef __WINDOWS
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
#elif defined __OSX
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
#endif
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uRGB32F_Tex_08, 0 );

	FBOservice::checkFBOErrors();

	//////////////////////////////////////////////////////////////////////////
	// Tonemap FBOs
	//////////////////////////////////////////////////////////////////////////
	glBindFramebuffer( GL_FRAMEBUFFER, m_uRGB32F_FBO_09 );
	glBindTexture( GL_TEXTURE_2D, m_uRGB32F_Tex_09 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, m_uScreenWidth, m_uScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uRGB32F_Tex_09, 0 );

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
#ifdef __WINDOWS
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
#elif defined __OSX
    	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
#endif
    
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
#ifdef __WINDOWS
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
#elif defined __OSX
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
#endif
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
	m_pTextureManager->DeclareTexture( TextureSemantics::FINAL_AFTER_TONEMAP, m_uFinalTonemappedTex_09 );
	m_pTextureManager->DeclareTexture( TextureSemantics::LUMINANCE, m_uFinalLuminanceTex_03 );
	m_pTextureManager->DeclareTexture( TextureSemantics::BLURRED, m_uFinalBeforeTonemapTex_08 );


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



void GLDeferredRenderer::renderEntity( Entity* pEntity, const SceneManager* pScene, const Camera *pCamera )
{
	Mesh* pMesh = pEntity->GetMesh();

	if( !pMesh )
		return;

	Shader* pShader =  pMesh->GetMaterial()->GetShader();
	
	if( !pShader )
		return;

	//Culling
	if( !pCamera->IsVisible( pEntity->getBoundingSphere() * pEntity->getNode()->getGlobalTransformMAT() ) )
	{
		//LOG( "Mesh culled" );
		return;
	}

	const glm::mat4& matModel = pEntity->getNode()->getLocalTransform().getAsMat4();
	m_pGLrenderer->RenderMesh( pMesh, matModel, pScene->getRootNode()->getGlobalTransformMAT(), pCamera, pMesh->GetMaterial(), pShader );
}


void GLDeferredRenderer::renderEntities( SceneManager* pScene, Camera* pCamera )
{
	const std::vector<Entity*>& vRenderObjects = pScene->GetRenderObjects();

	for( uint i = 0; i < vRenderObjects.size(); ++i )
	{
		renderEntity( vRenderObjects[ i ], pScene, pCamera );
	}
}




void GLDeferredRenderer::renderShadowMap( PointLight* pLight, SceneManager* pScene, Camera* pCamera )
{
	//Render all shadow-map passes to construct shadow-cubemap
	//////////////////////////////////////////////////////////////////////////
	if( pLight->GetDirty() )
	{
		pLight->SetDirty( false );

		PerformanceCheck clPerfCheck;
		clPerfCheck.StartPerformanceCheck( "Render Pointlight Shadowmap" );

		uint uNumShadowPasses = pLight->GetNumShadowmapPasses();

		//m_pGLrenderer->setColorMask( false, false, false, false ); //Deactivate color-channel writes
		
		m_pGLrenderer->saveViewport();
				
		//pLight->SetPosition( m_pEngine->GetCurrentCamera()->getPosition() );

		glBindFramebuffer( GL_FRAMEBUFFER, pLight->GetShadowmapFBO() );
		GLenum drawBufs[] = { GL_NONE, GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT0 };
		glDrawBuffers( 4, drawBufs );

		m_pGLrenderer->setViewport( 0, 0, pLight->GetShadowmapResolution().x, pLight->GetShadowmapResolution().y );
		
		for( int i = 0; i < uNumShadowPasses; ++i )
		{
			pLight->PrepareShadowmapPass( i ); //Apply the correct camera-transformations, bind the FBO and clear the cube-side contents
			m_pGLrenderer->prepareFrameRendering( pLight->GetCamera(), pScene->getRootNode()->getGlobalTransformMAT() ); //Has to be called again because the camera changes :(
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			renderEntities( pScene, pLight->GetCamera() );	
		}


		m_pGLrenderer->restoreViewport();
		clPerfCheck.FinishAndLogPerformanceCheck();

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}
}

void GLDeferredRenderer::renderShadowMap( DirectionalLight* pLight, SceneManager* pScene, Camera* pCamera )
{

}


void GLDeferredRenderer::renderDirLight( DirectionalLight* pLight, SceneManager* pScene, Camera* pCamera )
{
	m_pFSquad->RenderWithMaterial( m_pMAT_Dirlight, pCamera );
}

void GLDeferredRenderer::renderPointLight( PointLight* pLight, SceneManager* pScene, Camera* pCamera )
{
	//Mark all pixels occupied by the backside AND Occluded by geometry
	// Stencil 1 -> 2
	m_pGLrenderer->setCulling(true);
	m_pGLrenderer->setCullFace( GL_FRONT );
	m_pGLrenderer->setDepthTest( true );
	m_pGLrenderer->setDepthMask( false );
	m_pGLrenderer->setStencilTest( true );
	m_pGLrenderer->setColorMask( false, false, false, false );
	m_pGLrenderer->setStencilFunc( GL_EQUAL, 1, m_pGLrenderer->getStencilMask() );
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_INCR, GL_KEEP );

	m_pGLrenderer->RenderMesh( m_pPointlightMesh, glm::mat4( 1.0f ) , *m_pEngine->GetWorldMat(), pCamera, m_pPointlightMesh->GetMaterial() );

	//De-Mark pixels in front of the front side of the bounding region
	// Stencil 2 -> 1
	m_pGLrenderer->setCullFace( GL_BACK );
	m_pGLrenderer->setStencilFunc( GL_EQUAL, 2, m_pGLrenderer->getStencilMask() );
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_DECR, GL_KEEP );

	
	m_pGLrenderer->RenderMesh( m_pPointlightMesh, glm::mat4( 1.0f ), *m_pEngine->GetWorldMat(), pCamera, m_pPointlightMesh->GetMaterial() );

	m_pGLrenderer->setStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	m_pGLrenderer->setColorMask( true, true, true, true );
	m_pGLrenderer->setDepthTest( false );

	m_pMAT_Pointlight->SetShadowCubeTex( pLight->GetShdowCubeMap() );
	m_pFSquad->RenderWithMaterial( m_pMAT_Pointlight, pCamera );
}


void GLDeferredRenderer::RenderScene( SceneManager* pSceneManager )
{
	Camera* pCamera = pSceneManager->GetCamera();
	const glm::vec4& clearColor = m_pEngine->GetClearColor();
	PerformanceCheck clPerfCheck;

	//Update Scene graph
	CHECK_PERFORMANCE( pSceneManager->prepareRender(), "UpdateSceneGraph" ); 

	//Set States for shadowmap-rendering and g-buffer
	m_pGLrenderer->setDepthTest( true );
	m_pGLrenderer->setDepthMask( GL_TRUE );
	m_pGLrenderer->setBlending( false );
	m_pGLrenderer->setCulling( true );
	m_pGLrenderer->setCullFace( GL_BACK );
	m_pGLrenderer->setStencilTest( true );
	m_pGLrenderer->setStencilFunc( GL_ALWAYS, 1, 0xFFFFFFFF );
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );


	//////////////////////////////////////////////////////////////////////////
	//Shadow-map updates
	//////////////////////////////////////////////////////////////////////////
	const std::vector<DirectionalLight*>& vDirLights = pSceneManager->getCachedDirectionalLights();
	const std::vector<PointLight*>& vPointLights = pSceneManager->getCachedPointLights();
	const std::vector<SpotLight*>& vSpotLights = pSceneManager->getCachedSpotLights();

	//Render shadowmaps of all dirty lights:

	clPerfCheck.StartPerformanceCheck( "Render Pointlight Shadows" );
	for( uint i = 0; i < vPointLights.size(); ++i )
	{
		m_pGLrenderer->setCurrLightIndex( vDirLights.size() + i );
		m_pGLrenderer->prepareLightRendering( pCamera, vPointLights[ i ] );
		renderShadowMap( vPointLights[ i ], pSceneManager, vPointLights[ i ]->GetCamera() );
	}

	clPerfCheck.FinishAndLogPerformanceCheck();

	//Re-enable color-channel writes after shadowmap-rendering
	m_pGLrenderer->setColorMask( true, true, true, true );
	//////////////////////////////////////////////////////////////////////////
		
	//Update all global uniforms to render from the viewer-camera
	CHECK_PERFORMANCE( m_pGLrenderer->prepareFrameRendering( pSceneManager->GetCamera(), pSceneManager->getRootNode()->getGlobalTransformMAT() ), "UpdateGlobalUniforms" ); //Recalculate and Update per-frame ("global") Uniforms

	//////////////////////////////////////////////////////////////////////////
	//G-Buffer Pass
	//////////////////////////////////////////////////////////////////////////
	
	static GLenum eDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	STATIC_ASSERT( ARRAY_LENGTH( eDrawBuffers ) == GBuffer::num );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uDeferredFBO );

	glDrawBuffers( GBuffer::num, eDrawBuffers );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	
	
	clPerfCheck.StartPerformanceCheck( "Render Entities" );
	renderEntities( pSceneManager, pCamera );
	clPerfCheck.FinishAndLogPerformanceCheck();
	//////////////////////////////////////////////////////////////////////////


		
	//////////////////////////////////////////////////////////////////////////
	// Lights Passes
	//////////////////////////////////////////////////////////////////////////
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
	m_pGLrenderer->setStencilFunc( GL_EQUAL, m_pGLrenderer->getStencilRef(), m_pGLrenderer->getStencilMask() );
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	
	m_pGLrenderer->SetLightModeActive( true );

	clPerfCheck.StartPerformanceCheck( "Render Directional lights" );
	for( uint i = 0; i < vDirLights.size(); ++i )
	{
		m_pGLrenderer->setCurrLightIndex( i );
		m_pGLrenderer->prepareLightRendering( pCamera, vDirLights[ i ] );
		renderDirLight( vDirLights[ i ], pSceneManager, pCamera );	
	}
	clPerfCheck.FinishAndLogPerformanceCheck();

	clPerfCheck.StartPerformanceCheck( "Render Point lights" );
	for( uint i = 0; i < vPointLights.size(); ++i )
	{
		m_pGLrenderer->setCurrLightIndex( vDirLights.size() + i );
		m_pGLrenderer->prepareLightRendering( pCamera, vPointLights[ i ] );
		renderPointLight( vPointLights[ i ], pSceneManager, pCamera );
	}
	clPerfCheck.FinishAndLogPerformanceCheck();

	m_pGLrenderer->setCurrLightIndex( 0 );
	m_pGLrenderer->SetLightModeActive( false );

	//DEBUG:
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	
	//////////////////////////////////////////////////////////////////////////
	//Screen-space passes
	//////////////////////////////////////////////////////////////////////////
	glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalCompositeFBO_07 );
	glClear( GL_COLOR_BUFFER_BIT );
	
	m_pGLrenderer->setDepthTest( false );
	m_pGLrenderer->setDepthMask( GL_FALSE );
	m_pGLrenderer->setBlending( false );
	m_pGLrenderer->setStencilTest( true );
	m_pGLrenderer->setStencilFunc( GL_LEQUAL, 1, m_pGLrenderer->getStencilMask() );
	m_pGLrenderer->setStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	

	CHECK_PERFORMANCE( m_pFSquad->RenderWithMaterial( m_pMAT_FinalComposite, pCamera ), "Final before Postpro" );

	GLuint uSceneTexture = 0;
	GLuint uSceneFBO = 0;

	uSceneTexture = GetOutputTexture();
	uSceneFBO = GetOutputFBO();

	m_pGLrenderer->setDepthTest( false );

	//Render Postprocessing-effects

	//PASS: Bloom
	//////////////////////////////////////////////////////////////////////////
	//Input: FinalCompositeTex
	//Output: FinalBeforeTonemapTex
	if( m_pGLrenderer->GetUseBloom() )
	{
		m_pMAT_Bloom->SetInputTexture( uSceneTexture );

		//Downsample for bloom
		glBindFramebuffer( GL_READ_FRAMEBUFFER, uSceneFBO );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_uBloomFBO_01 );
		glBlitFramebuffer( 0, 0, m_uScreenWidth, m_uScreenHeight, 0, 0, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR );

		//Apply brightpass-filter
		m_pGLrenderer->saveViewport();
		m_pGLrenderer->setViewport( 0, 0, uBLURTEX_WIDTH, uBLURTEX_HEIGHT );
		glBindFramebuffer( GL_FRAMEBUFFER, m_uBrightPassFBO_02 );
		m_pFSquad->RenderWithMaterial( m_pMAT_BrightPass, pCamera );

		//Blur in several passes
#ifdef __WINDOWS
		CHECK_PERFORMANCE( m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBrightPassTex_02, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT, m_pGLrenderer, pCamera, 5 ), "Blur1" );
		CHECK_PERFORMANCE( m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBloomTex_01, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT, m_pGLrenderer, pCamera, 5 ), "Blur2" );
		CHECK_PERFORMANCE( m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBloomTex_01, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT, m_pGLrenderer, pCamera, 5 ), "Blur3" );
		CHECK_PERFORMANCE( m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBloomTex_01, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT, m_pGLrenderer, pCamera, 5 ), "Blur4" );
#elif defined __OSX
        CHECK_PERFORMANCE( m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBrightPassTex_02, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F_ARB, GL_RGB, GL_FLOAT, m_pGLrenderer, pCamera, 5 ), "Blur1" );
		CHECK_PERFORMANCE( m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBloomTex_01, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F_ARB, GL_RGB, GL_FLOAT, m_pGLrenderer, pCamera, 5 ), "Blur2" );
		CHECK_PERFORMANCE( m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBloomTex_01, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F_ARB, GL_RGB, GL_FLOAT, m_pGLrenderer, pCamera, 5 ), "Blur3" );
		CHECK_PERFORMANCE( m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBloomTex_01, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F_ARB, GL_RGB, GL_FLOAT, m_pGLrenderer, pCamera, 5 ), "Blur4" );
#endif

		m_pGLrenderer->restoreViewport();

		//Apply the actual bloom
		glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalBeforeTonemapFBO_08 );
		
		CHECK_PERFORMANCE( m_pFSquad->RenderWithMaterial( m_pMAT_Bloom, pCamera ), "Bloom" );
	}

	else
	{
		glBindFramebuffer( GL_READ_FRAMEBUFFER, uSceneFBO );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_uFinalBeforeTonemapFBO_08 );
		glBlitFramebuffer( 0, 0, m_uScreenWidth, m_uScreenHeight, 0, 0, m_uScreenWidth, m_uScreenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	}
	//////////////////////////////////////////////////////////////////////////



	//Pass: Tone-Mapping
	//////////////////////////////////////////////////////////////////////////
	//Input: FinalBeforeTonemapTex
	//Output: FinalTonemappedTex
	if( m_pGLrenderer->GetToneMappingEnabled() )
	{
		//////////////////////////////////////////////////////////////////////////
		//Luminance Adaption
		//////////////////////////////////////////////////////////////////////////
		//Resolve to luminance Format: FinalBeforeTonemap ----> CurrLuminance
		glBindFramebuffer( GL_READ_FRAMEBUFFER, m_uFinalBeforeTonemapFBO_08 );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_uCurrLuminanceFBO_04 );
		glBlitFramebuffer( 0, 0, m_uScreenWidth, m_uScreenHeight, 0, 0, 512, 512, GL_COLOR_BUFFER_BIT, GL_LINEAR );

		//FinalLuminance --> LastLuminance
		glBindFramebuffer( GL_READ_FRAMEBUFFER, m_uFinalLuminanceFBO_03 );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_uLastLuminanceFBO_05 );
		glBlitFramebuffer( 0, 0, 512, 512, 0, 0, 512, 512, GL_COLOR_BUFFER_BIT, GL_NEAREST );

		//Adapt luminance depending on the adaption-value:  (LastLuminance, CurrLuminance) ----> FinalLuminance
		m_pGLrenderer->saveViewport();
		m_pGLrenderer->setViewport( 0, 0, 512, 512 );
		glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalLuminanceFBO_03 );
		glClear( GL_COLOR_BUFFER_BIT );
		CHECK_PERFORMANCE( m_pFSquad->RenderWithMaterial( m_pMAT_LumaTimeAdaption, pCamera ), "LumaAdaption" );
		m_pGLrenderer->restoreViewport();

		//Generate Mipmaps for the Final luminance Texture
		glBindTexture( GL_TEXTURE_2D, m_uFinalLuminanceTex_03 );
		CHECK_PERFORMANCE( glGenerateMipmap( GL_TEXTURE_2D ), "Luma MIPMAP generation" );
	}

	//////////////////////////////////////////////////////////////////////////
	//Tonemapping and gamma correction
	glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalTonemapFBO_09 );
	glClear( GL_COLOR_BUFFER_BIT );
	CHECK_PERFORMANCE( m_pFSquad->RenderWithMaterial( m_pMAT_ToneMap, pCamera ), "Tonemap and Gamma correction" );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//Render to Backbuffer with FXAA or not
	//////////////////////////////////////////////////////////////////////////
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	m_pGLrenderer->setStencilTest( false );

	if( m_pGLrenderer->GetFXAAenabled() )
	{
		CHECK_PERFORMANCE( m_pFSquad->RenderWithMaterial( m_pMAT_FXAA, pCamera ), "FXAA" );
	}

	else
		m_pFSquad->RenderTexture( m_uFinalTonemappedTex_09, pCamera );


	//m_pFSquad->RenderTexture( vPointLights[ 0 ]->GetShdowCubeMap() );

	//*/
	//Render every debug-Operation

	if( m_pGLrenderer->GetDebugTexturesVisible() && m_pEngine->GetDebugTexturePasses().size() )
	{
		m_pGLrenderer->saveViewport();
		const std::vector<SDebugTexturePass>& vTexturePasses = m_pEngine->GetDebugTexturePasses();

		float fTargetRatio = ( (float) m_uScreenWidth ) / (float) m_uScreenHeight;
		uint uScaling = vTexturePasses.size() > 1 ? vTexturePasses.size() : 2;
		uint uElementWidth = m_uScreenWidth / uScaling;
		uint uElementHeight = static_cast<uint>( ( (float) uElementWidth ) / fTargetRatio );

		for( uint8 uIdx = 0; uIdx < vTexturePasses.size(); ++uIdx )
		{
			TextureSemantics::eTexSemantic eSemantic = vTexturePasses[ uIdx ].m_eTexSemantic;

			m_pGLrenderer->setViewport( uElementWidth * uIdx, 0, uElementWidth, uElementHeight ); 

			if( eSemantic < TextureSemantics::TEX_3D_DSM)
				m_pFSquad->RenderTexture( m_pTextureManager->LookupTexture( eSemantic ), pCamera );
		}

		m_pGLrenderer->restoreViewport();
	}
}

