
#include "../includes.h"

#include "GLRenderer.h"
#include "../IO/FileReader.h"
#include <assert.h>
#include "Materials/Material.h"
#include "../Geometry/VertexDeclarations.h"
#include "Shader.h"
#include "../Scene/SceneManager.h"
#include "Materials/MAT_Test.h"
#include "../Engine.h"
#include "../Light/Light.h"
#include "../Light/DirectionalLight.h"
#include "../Light/PointLight.h"
#include "../Light/SpotLight.h"
#include "../IO/ModelLoader.h"
#include "../Scene/AABoundingBox.h"
#include "Managers/TextureManager.h"

#include "GLVolumeRenderer.h"

#include "../Scene/Camera.h"
#include "GLTexture.h"
#include "../Services/FBOservice.h"
#include "../Services/GLDebug.h"

#include "Pass_GaussianBlur.h"

#include "../IO/TextureLoader.h"

#include "Materials/MAT_FSquad_FXAA.h"
#include "Materials/MAT_FSquad_ToneMapping.h"
#include "Materials/MAT_FSquad_LumaTimeAdaption.h"
#include "Materials/MAT_FSquad_GaussianBlur.h"
#include "Materials/MAT_FSquad_BrightPass.h"
#include "Materials/MAT_FSquad_Bloom.h"
#include "Materials/MAT_FSquad_Textured3D.h"

#include "IUniform.h"
#include "Uniform.h"
#include "UniformUtil.h"

const float PI = 3.14159265358979323846f;

const uint uBLURTEX_WIDTH = 128;
const uint uBLURTEX_HEIGHT = 128;


GLRenderer::GLRenderer() : 
m_pEngine( NULL ),
m_bDepthTest( true ),
m_eDepthFunc( GL_LESS ),
m_bStencilTest( false ),
m_eStencilFunc( GL_ALWAYS ),
m_iStencilRef( 1 ),
m_uStencilMask( 0xFFFFFFFF ),
m_eStencilOp_sFail( GL_KEEP ),
m_eStencilOp_depthFail( GL_KEEP ),
m_eStencilOp_depthPass( GL_KEEP ),
m_bCulling( true ),
m_eCullFaceDir( GL_BACK ),
m_uCurrLightIdx( 0 ),
m_eBlendSrc( GL_ONE ),
m_eBlendDest( GL_ONE ),
m_bBlending( false ) ,
m_bDepthMask( GL_TRUE ),
m_bColorMask_r( true ),
m_bColorMask_g( true ),
m_bColorMask_b( true ),
m_bColorMask_a( true )
{
	m_pEngine = &Engine::GetInstance();

	//Important: synchronize cached states with the hardware-states
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( m_eDepthFunc );
	glDepthMask( GL_TRUE );
	glDisable( GL_STENCIL_TEST );
	glStencilFunc( m_eStencilFunc, m_iStencilRef, m_uStencilMask );
	glStencilOp( m_eStencilOp_sFail, m_eStencilOp_depthFail, m_eStencilOp_depthPass );
	glEnable( GL_CULL_FACE );
	glCullFace( m_eCullFaceDir );
	glDisable( GL_BLEND );
	glBlendFunc( m_eBlendSrc, m_eBlendDest );
	glColorMask( true, true, true, true );	
}

GLRenderer::~GLRenderer()
{
	deleteResolutionDependentResources();

	SAFE_DELETE( m_pMAT_FXAA );
	SAFE_DELETE( m_pMAT_ToneMap );
	SAFE_DELETE( m_pMAT_LumaTimeAdaption );
	SAFE_DELETE( m_pMAT_GaussianBlur );
	SAFE_DELETE( m_pMAT_Bloom );
	SAFE_DELETE( m_pMAT_BrightPass );
}

void GLRenderer::init( uint uWidth, uint uHeight )
{
	m_pFSquad = &FullscreenQuad::getInstance();

	m_pEngine = &Engine::GetInstance();

	m_pTextureManager = &TextureManager::getInstance();

	m_pUniformRegistry = &UniformRegistry::GetInstance();

	m_pPassGaussianBlur = &Pass_GaussianBlur::GetInstance();
	m_pPassGaussianBlur->Init( 5, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT );

	setDepthFunc( GL_LESS );
	setDepthTest( true );

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

	
	if(  m_pEngine->getRenderMode() == Engine::RENDER_DEFERRED )
	{
		m_pSceneRenderer = &GLDeferredRenderer::GetInstance();
		m_pSceneRenderer->Init( uWidth, uHeight, this );		
	}

	else
	{
		/*
		m_pSceneRenderer = FORWARD renderer
		*/
	}

	m_pVolumeRenderer = &GLVolumeRenderer::GetInstance();
	m_pVolumeRenderer->Init( uWidth, uHeight, this );
	
	SetResolution( uWidth, uHeight );
}

void GLRenderer::SetResolution( uint uWidth, uint uHeight )
{
	m_uScreenWidth = uWidth;
	m_uScreenHeight = uHeight;
	setViewport( 0, 0, uWidth, uHeight );

	updateTextures();
		
	m_pSceneRenderer->SetResolution( uWidth, uHeight );	
	m_pVolumeRenderer->SetResolution( uWidth, uHeight );

	updatePostproMaterials();
}

void GLRenderer::deleteResolutionDependentResources()
{
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



void GLRenderer::updatePostproMaterials()
{
	//Notify Postpro-materials of the new textures:
	m_pMAT_BrightPass->SetInputTexture( m_uBloomTex_01 );
		
	m_pMAT_Bloom->SetBloomTexture( m_uBloomTex_01 );
			
	m_pMAT_LumaTimeAdaption->SetLumaTextureLocs( m_uCurrLuminanceTex_04, m_uLastLuminanceTex_05 );

	m_pMAT_ToneMap->SetInputTextureLoc( m_uFinalBeforeTonemapTex_08 );
	m_pMAT_ToneMap->SetAvgLuminanceTextureLoc( m_uSmall_LUM32F_Tex_03 );
	
	m_pMAT_FXAA->SetInputTexture( m_uFinalTonemappedTex_09 );
}



void GLRenderer::updateTextures()
{
	static bool bTexturesInit = false;

	if( bTexturesInit )
	{
		//Delete textures and FBOs to account for new resolution
		deleteResolutionDependentResources();
	}

	if( m_uScreenWidth == 0 )
		m_uScreenWidth = 1;

	if( m_uScreenHeight == 0 )
		m_uScreenHeight = 1;

	bTexturesInit = true;

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

	//////////////////////////////////////////////////////////////////////////
	//Bloom-FBOs
	//////////////////////////////////////////////////////////////////////////
	glBindFramebuffer( GL_FRAMEBUFFER, m_uSmall_RGB32F_FBO_01 );
	glBindTexture( GL_TEXTURE_2D, m_uSmall_RGB32F_Tex_01 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uSmall_RGB32F_Tex_01, 0 );

	FBOservice::checkFBOErrors();


	glBindFramebuffer( GL_FRAMEBUFFER, m_uSmall_RGB32F_FBO_02 );
	glBindTexture( GL_TEXTURE_2D, m_uSmall_RGB32F_Tex_02 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL );
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
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, m_uScreenWidth, m_uScreenHeight, 0, GL_RGB, GL_FLOAT, NULL );
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

	
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	

	m_pTextureManager->DeclareTexture( TextureSemantics::FINAL_AFTER_TONEMAP, m_uFinalTonemappedTex_09 );
	m_pTextureManager->DeclareTexture( TextureSemantics::LUMINANCE, m_uFinalLuminanceTex_03 );
	m_pTextureManager->DeclareTexture( TextureSemantics::BLURRED, m_uFinalBeforeTonemapTex_08 );
}


void GLRenderer::RenderScene( SceneManager* pSceneManager )
{
	pSceneManager->prepareRender(); //Update Scene graph
	prepareFrameRendering( m_pEngine->GetCurrentCamera(), pSceneManager ); //Recalculate and Update per-frame ("global") Uniforms
	
	Engine::EVolumeMode eVolumeMode = m_pEngine->GetVolumeMode();

	//Render regular Meshes
	if( eVolumeMode != Engine::VOLUMES_SHOW_ONLY )
		m_pSceneRenderer->RenderScene( pSceneManager );

	//Render Volumetric Meshes
	if( eVolumeMode != Engine::VOLUMES_DONT_SHOW )
		m_pVolumeRenderer->RenderScene( pSceneManager );

	GLuint uSceneTexture = 0;
	GLuint uSceneFBO = 0;

	//TODO: Add the implementation for the combination!
	if( eVolumeMode == Engine::VOLUMES_SHOW_BOTH ||
		eVolumeMode == Engine::VOLUMES_SHOW_ONLY	)
	{
		uSceneTexture = m_pVolumeRenderer->GetOutputTexture();
		uSceneFBO = m_pVolumeRenderer->GetOutputFBO();
	}

	else
	{
		uSceneTexture = m_pSceneRenderer->GetOutputTexture();
		uSceneFBO = m_pSceneRenderer->GetOutputFBO();
	}



	//setStencilTest( false );
	setDepthTest( false );

	///*
	//Render Postprocessing-effects

	//PASS: Bloom
	//////////////////////////////////////////////////////////////////////////
	//Input: FinalCompositeTex
	//Output: FinalBeforeTonemapTex
	if( m_pEngine->GetUseBloom() )
	{
		m_pMAT_Bloom->SetInputTexture( uSceneTexture );

		//Downsample for bloom
		glBindFramebuffer( GL_READ_FRAMEBUFFER, uSceneFBO );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_uBloomFBO_01 );
		glBlitFramebuffer( 0, 0, m_uScreenWidth, m_uScreenHeight, 0, 0, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR );

		//Apply brightpass-filter
		saveViewport();
		setViewport( 0, 0, uBLURTEX_WIDTH, uBLURTEX_HEIGHT );
		glBindFramebuffer( GL_FRAMEBUFFER, m_uBrightPassFBO_02 );
		m_pFSquad->RenderWithMaterial( m_pMAT_BrightPass );

		//Blur in several passes
		m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBrightPassTex_02, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT, this, 5 );
		m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBloomTex_01, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT, this, 5 );
		m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBloomTex_01, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT, this, 5 );
		m_pPassGaussianBlur->BlurTextureIntoFBO( m_uBloomTex_01, m_uBloomFBO_01, uBLURTEX_WIDTH, uBLURTEX_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT, this, 5 );

		restoreViewport();

		//Apply the actual bloom
		glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalBeforeTonemapFBO_08 );
		m_pFSquad->RenderWithMaterial( m_pMAT_Bloom );
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
	if( m_pEngine->GetToneMappingEnabled() )
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
		saveViewport();
		setViewport( 0, 0, 512, 512 );
		glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalLuminanceFBO_03 );
		glClear( GL_COLOR_BUFFER_BIT );
		m_pFSquad->RenderWithMaterial( m_pMAT_LumaTimeAdaption );
		restoreViewport();

		//Generate Mipmaps for the Final luminance Texture
		glBindTexture( GL_TEXTURE_2D, m_uFinalLuminanceTex_03 );
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	//////////////////////////////////////////////////////////////////////////
	//Tonemapping and gamma correction
	glBindFramebuffer( GL_FRAMEBUFFER, m_uFinalTonemapFBO_09 );
	glClear( GL_COLOR_BUFFER_BIT );
	m_pFSquad->RenderWithMaterial( m_pMAT_ToneMap );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//Render to Backbuffer with FXAA or not
	//////////////////////////////////////////////////////////////////////////
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	setStencilTest( false );

	if( m_pEngine->GetFXAAenabled() )
		m_pFSquad->RenderWithMaterial( m_pMAT_FXAA );

	else
		m_pFSquad->RenderTexture( m_uFinalTonemappedTex_09 );

	//*/
	//Render every debug-Operation
	if( m_pEngine->GetDebugTexturesVisible() && m_pEngine->GetDebugTexturePasses().size() )
	{
		saveViewport();
		const std::vector<SDebugTexturePass>& vTexturePasses = m_pEngine->GetDebugTexturePasses();

		float fTargetRatio = ( (float) m_uScreenWidth ) / (float) m_uScreenHeight;
		uint uScaling = vTexturePasses.size() > 1 ? vTexturePasses.size() : 2;
		uint uElementWidth = m_uScreenWidth / uScaling;
		uint uElementHeight = ( (float) uElementWidth ) / fTargetRatio;

		for( uint8 uIdx = 0; uIdx < vTexturePasses.size(); ++uIdx )
		{
			TextureSemantics::eTexSemantic eSemantic = vTexturePasses[ uIdx ].m_eTexSemantic;

			setViewport( uElementWidth * uIdx, 0, uElementWidth, uElementHeight ); 

			if( eSemantic < TextureSemantics::TEX_3D_DSM)
				m_pFSquad->RenderTexture( m_pTextureManager->LookupTexture( eSemantic ) );

			else 
				m_pFSquad->RenderTimeTexture3D( m_pTextureManager->LookupTexture( eSemantic ) );
		}

		restoreViewport();
	}
}



void GLRenderer::setDepthTest( bool bEnable )
{
	if( m_bDepthTest == bEnable )
		return;

	if( bEnable )
		glEnable( GL_DEPTH_TEST );

	else
		glDisable( GL_DEPTH_TEST );

	m_bDepthTest = bEnable;
}

void GLRenderer::setDepthFunc( GLenum func )
{
	if( m_eDepthFunc == func )
		return;

	glDepthFunc( func );

	m_eDepthFunc = func;
}


void GLRenderer::setStencilTest( bool bEnable )
{
	if( m_bStencilTest == bEnable )
		return;

	if( bEnable )
		glEnable( GL_STENCIL_TEST );

	else
		glDisable( GL_STENCIL_TEST );

	m_bStencilTest = bEnable;
}


void GLRenderer::setStencilFunc( GLenum func, GLint ref, GLuint mask )
{
	if( m_eStencilFunc == func && m_iStencilRef == ref && m_uStencilMask == mask )
		return;

	glStencilFunc( func, ref, mask );

	m_eStencilFunc = func;
	m_iStencilRef = ref;
	m_uStencilMask = mask;
}


void GLRenderer::setStencilOp( GLenum stencilFail, GLenum depthFail, GLenum depthPass )
{
	if( m_eStencilOp_sFail == stencilFail && m_eStencilOp_depthFail == depthFail && m_eStencilOp_depthPass == depthPass )
		return;

	glStencilOp( stencilFail, depthFail, depthPass );

	m_eStencilOp_sFail = stencilFail;
	m_eStencilOp_depthFail = depthFail;
	m_eStencilOp_depthPass = depthPass;
}


void GLRenderer::setCulling( bool bEnable )
{
	if( m_bCulling == bEnable )
		return;

	if( bEnable )
		glEnable( GL_CULL_FACE );

	else
		glDisable( GL_CULL_FACE );

	m_bCulling = bEnable;
}


void GLRenderer::setCullFace( GLenum faceDir )
{
	if( m_eCullFaceDir == faceDir )
		return;

	glCullFace( faceDir );

	m_eCullFaceDir = faceDir;
}

void GLRenderer::setBlending( bool bEnable )
{
	if( m_bBlending == bEnable )
		return;

	if( bEnable )
		glEnable( GL_BLEND );

	else
		glDisable( GL_BLEND );

	m_bBlending = bEnable;
}

void GLRenderer::setBlendFunc( GLenum src, GLenum dest )
{
	if( src == m_eBlendSrc && dest == m_eBlendDest )
		return;

	glBlendFunc( src, dest );

	m_eBlendSrc = src;
	m_eBlendDest = dest;
}

void GLRenderer::setDepthMask( GLboolean bMask )
{
	if( bMask == m_bDepthMask )
		return;

	glDepthMask( bMask );

	m_bDepthMask = bMask;
}

void GLRenderer::setColorMask( bool bR, bool bG, bool bB, bool bA )
{
	if( bR != m_bColorMask_r || 
		bG != m_bColorMask_g ||
		bB != m_bColorMask_b ||
		bA != m_bColorMask_a )
	{
		glColorMask( bR, bG, bB, bA );
		
		m_bColorMask_r = bR;
		m_bColorMask_g = bG;
		m_bColorMask_b = bB;
		m_bColorMask_a = bA;
	}
}

void GLRenderer::saveViewport()
{
	m_v4TempViewport = m_v4Viewport;
}

void GLRenderer::restoreViewport()
{
	m_v4Viewport = m_v4TempViewport;
	glViewport( m_v4Viewport.x, m_v4Viewport.y, m_v4Viewport.z, m_v4Viewport.w );
}

void GLRenderer::setViewport( int x, int y, int width, int height )
{
	m_v4Viewport.x = x;
	m_v4Viewport.y = y;
	m_v4Viewport.z = width;
	m_v4Viewport.w = height;

	glViewport( m_v4Viewport.x, m_v4Viewport.y, m_v4Viewport.z, m_v4Viewport.w );
}


void GLRenderer::InitMesh( Mesh* pMesh )
{
	if( !pMesh )
	{
		return;
	}

	if( !pMesh->m_pMaterial ) 
	{
		pMesh->m_pMaterial = new MAT_Test();
		pMesh->m_pMaterial->Init();
	}
}

void GLRenderer::InitVolumeMesh( VolumeMesh* pVolumeMesh )
{
	if( !pVolumeMesh )
		return;

	if( !pVolumeMesh->m_pVolumeMaterial )
	{

		pVolumeMesh->SetMaterial( new MAT_FSquad_Textured3D() );

	}
}

void GLRenderer::prepareLightRendering( const Camera* pCamera, const SceneManager* pScene )
{
	using namespace ShaderSemantics;
	typedef std::vector<IUniform*>								UniformVecT;
	typedef std::map<ShaderSemantics::Semantic, UniformVecT>	UniformMapT;

	const std::vector<Light*>& vLights = pScene->getCachedLights();

	const Light* pLight						= NULL;
	const DirectionalLight* pDirLight		= NULL;
	const PointLight*		pPointLight		= NULL;
	const SpotLight*		pSpotLight		= NULL;

	if( m_bActiveLightMode )
	{
		if( vLights.size() > m_uCurrLightIdx )
			pLight = vLights[ m_uCurrLightIdx ];
	}

	else
		LOG( "WARNING: Uniform-based light selection has been deprecated and is not implemented anymore!" );

	if( pLight )
	{
		if( pLight->getLightType() == Light::LIGHTTYPE_DIRECTIONAL )
			pDirLight = dynamic_cast<const DirectionalLight*>( pLight );
		else if( pLight->getLightType() == Light::LIGHTTYPE_POINT )
			pPointLight = dynamic_cast<const PointLight*>( pLight );
		else if( pLight->getLightType() == Light::LIGHTTYPE_SPOT )
			pSpotLight = dynamic_cast<const SpotLight*>( pLight );
	}

	//Loop through all registered uniforms. Note:: Assume that there are only global uniforms registered
	UniformMapT& rUniformMap = m_pUniformRegistry->GetUniformMap();
	UniformMapT::iterator iter;
	const glm::mat4& matView = pCamera->GetView();

	iter = rUniformMap.find( LIGHTDIRWORLD );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pDirLight )
		{
			const glm::vec3& dirWS = pDirLight->getCachedDirection();
			UniformUtil::UpdateUniforms( rvUniforms, dirWS );
		}

		else if( pSpotLight )
		{
			const glm::vec3& dirWS = pSpotLight->getCachedDirection();
			UniformUtil::UpdateUniforms( rvUniforms, dirWS );
		}
	}


	iter = rUniformMap.find( LIGHTDIRVIEW );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pDirLight )
		{
			const glm::vec3& dirWS = pDirLight->getCachedDirection();
			glm::vec4 dirVS4 = matView * glm::vec4( dirWS.x, dirWS.y, dirWS.z, 0.0f );
			glm::vec3 dirVS = glm::normalize( glm::vec3( dirVS4.x, dirVS4.y, dirVS4.z ) );

			UniformUtil::UpdateUniforms( rvUniforms, dirVS );

			//std::stringstream ss;
			//ss << dirVS.x << "\t" << dirVS.y << "\t" << dirVS.z;
			//LOG( ss.str() );
		}

		else if( pSpotLight )
		{
			const glm::vec3& dirWS = pSpotLight->getCachedDirection();
			glm::vec4 dirVS4 = matView * glm::vec4( dirWS.x, dirWS.y, dirWS.z, 0.0f );
			glm::vec3 dirVS = glm::normalize( glm::vec3( dirVS4.x, dirVS4.y, dirVS4.z ) );

			UniformUtil::UpdateUniforms( rvUniforms, dirVS );
		}
	}


	iter = rUniformMap.find( LIGHTPOSWORLD );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			const glm::vec3& posWS = pLight->getCachedPosition();
			UniformUtil::UpdateUniforms( rvUniforms, posWS );
		}
	}


	iter = rUniformMap.find( LIGHTPOSVIEW );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;


		if( pLight )
		{
			const glm::vec3& posWS = pLight->getCachedPosition();

			glm::vec4 posVS4 = matView * glm::vec4( posWS.x, posWS.y, posWS.z, 1.0 );
			glm::vec3 posVS = glm::vec3( posVS4.x, posVS4.y, posVS4.z );

			UniformUtil::UpdateUniforms( rvUniforms, posVS );
		}
	}


	iter = rUniformMap.find( LIGHTCOLOR );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;


		if( pLight )
		{
			const glm::vec3& color = pLight->getColor();
			UniformUtil::UpdateUniforms( rvUniforms, color );
		}

	}

	iter = rUniformMap.find( LIGHTINTENSITY );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			float fI = pLight->getIntensity();
			UniformUtil::UpdateUniforms( rvUniforms, fI );
		}

	}


	iter = rUniformMap.find( LIGHTCOLORINTENSITY );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			const glm::vec3& color = pLight->getColor() * pLight->getIntensity();
			UniformUtil::UpdateUniforms( rvUniforms, color );
		}

	}

	iter = rUniformMap.find( LIGHTRSTART );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pPointLight )
		{
			float fRstart = pPointLight->getFalloffStart();
			UniformUtil::UpdateUniforms( rvUniforms, fRstart );
		}

	}


	iter = rUniformMap.find( LIGHTREND );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pPointLight )
		{
			float fRend = pPointLight->getFalloffEnd();
			UniformUtil::UpdateUniforms( rvUniforms, fRend );
		}
	}

	iter = rUniformMap.find( LIGHTVIEW );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			UniformUtil::UpdateUniforms( rvUniforms, pLight->GetCamera()->GetView() );
		}
	}


	iter = rUniformMap.find( LIGHTVIEWI );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			UniformUtil::UpdateUniforms( rvUniforms, pLight->GetCamera()->GetViewInv() );
		}
	}


	iter = rUniformMap.find( LIGHTPROJ );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			UniformUtil::UpdateUniforms( rvUniforms, pLight->GetCamera()->GetProjection() );
		}	
	}


	iter = rUniformMap.find( LIGHTPROJI );
	if(  iter != rUniformMap.end() )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			const glm::mat4& proj =  pLight->GetCamera()->GetProjection();
			UniformUtil::UpdateUniforms( rvUniforms, glm::inverse( proj ) );
		}
	}

	/*

	using namespace ShaderSemantics;
	typedef std::vector<IUniform*>								UniformVecT;
	typedef std::map<ShaderSemantics::Semantic, UniformVecT>	UniformMapT;

	const std::vector<Light*>& vLights = pScene->getCachedLights();

	const Light* pLight						= NULL;
	const DirectionalLight* pDirLight		= NULL;
	const PointLight*		pPointLight		= NULL;
	const SpotLight*		pSpotLight		= NULL;

	//Loop through all registered uniforms. Note:: Assume that there are only global uniforms registered
	UniformMapT& rUniformMap = m_pUniformRegistry->GetUniformMap();
	for( UniformMapT::iterator iter = rUniformMap.begin(); iter != rUniformMap.end(); ++iter )
	{
		const glm::mat4& matView = pCamera->GetView();

		UniformVecT& rvUniforms = iter->second;

		Semantic eSemantic = iter->first;

		if( ( eSemantic > LIGHTTYPES_START && eSemantic < LIGHTTYPES_END ) == false )
			continue;
		
			if( m_bActiveLightMode )
			{
				if( vLights.size() > m_uCurrLightIdx )
					pLight = vLights[ m_uCurrLightIdx ];
			}

			else
				LOG( "WARNING: Uniform-based light selection has been deprecated and is not implemented anymore!" );

			if( pLight )
			{
				if( pLight->getLightType() == Light::LIGHTTYPE_DIRECTIONAL )
					pDirLight = dynamic_cast<const DirectionalLight*>( pLight );
				else if( pLight->getLightType() == Light::LIGHTTYPE_POINT )
					pPointLight = dynamic_cast<const PointLight*>( pLight );
				else if( pLight->getLightType() == Light::LIGHTTYPE_SPOT )
					pSpotLight = dynamic_cast<const SpotLight*>( pLight );
			}


			switch( eSemantic )
			{
				case LIGHTDIRWORLD:
					{
						if( pDirLight )
						{
							const glm::vec3& dirWS = pDirLight->getCachedDirection();
							UniformUtil::UpdateUniforms( rvUniforms, dirWS );
						}

						else if( pSpotLight )
						{
							const glm::vec3& dirWS = pSpotLight->getCachedDirection();
							UniformUtil::UpdateUniforms( rvUniforms, dirWS );
						}

					} break;

				case LIGHTDIRVIEW:
					{
						if( pDirLight )
						{
							const glm::vec3& dirWS = pDirLight->getCachedDirection();
							glm::vec4 dirVS4 = matView * glm::vec4( dirWS.x, dirWS.y, dirWS.z, 0.0f );
							glm::vec3 dirVS = glm::normalize( glm::vec3( dirVS4.x, dirVS4.y, dirVS4.z ) );

							UniformUtil::UpdateUniforms( rvUniforms, dirVS );

							//std::stringstream ss;
							//ss << dirVS.x << "\t" << dirVS.y << "\t" << dirVS.z;
							//LOG( ss.str() );
						}

						else if( pSpotLight )
						{
							const glm::vec3& dirWS = pSpotLight->getCachedDirection();
							glm::vec4 dirVS4 = matView * glm::vec4( dirWS.x, dirWS.y, dirWS.z, 0.0f );
							glm::vec3 dirVS = glm::normalize( glm::vec3( dirVS4.x, dirVS4.y, dirVS4.z ) );

							UniformUtil::UpdateUniforms( rvUniforms, dirVS );
						}
					} break;

				case LIGHTPOSWORLD:
					{
						if( pLight )
						{
							const glm::vec3& posWS = pLight->getCachedPosition();

							UniformUtil::UpdateUniforms( rvUniforms, posWS );
						}
					} break;

				case LIGHTPOSVIEW:
					{
						if( pLight )
						{
							const glm::vec3& posWS = pLight->getCachedPosition();

							glm::vec4 posVS4 = matView * glm::vec4( posWS.x, posWS.y, posWS.z, 1.0 );
							glm::vec3 posVS = glm::vec3( posVS4.x, posVS4.y, posVS4.z );

							UniformUtil::UpdateUniforms( rvUniforms, posVS );
						}
					} break;

				case LIGHTCOLOR:
					{
						if( pLight )
						{
							const glm::vec3& color = pLight->getColor();
							UniformUtil::UpdateUniforms( rvUniforms, color );
						}

					} break;

				case LIGHTINTENSITY:
					{
						if( pLight )
						{
							float fI = pLight->getIntensity();
							UniformUtil::UpdateUniforms( rvUniforms, fI );
						}

					} break;

				case LIGHTCOLORINTENSITY:
					{
						if( pLight )
						{
							const glm::vec3& color = pLight->getColor() * pLight->getIntensity();
							UniformUtil::UpdateUniforms( rvUniforms, color );
						}
					} break;

				case LIGHTRSTART:
					{
						if( pPointLight )
						{
							float fRstart = pPointLight->getFalloffStart();
							UniformUtil::UpdateUniforms( rvUniforms, fRstart );
						}
					} break;

				case LIGHTREND:
					{
						if( pPointLight )
						{
							float fRend = pPointLight->getFalloffEnd();
							UniformUtil::UpdateUniforms( rvUniforms, fRend );
						}
					} break;

				case LIGHTVIEW:
					{
						if( pLight )
						{
							UniformUtil::UpdateUniforms( rvUniforms, pLight->GetCamera()->GetView() );
						}
					} break;

				case LIGHTVIEWI:
					{
						if( pLight )
						{
							UniformUtil::UpdateUniforms( rvUniforms, pLight->GetCamera()->GetViewInv() );
						}
					} break;

				case LIGHTPROJ:
					{
						if( pLight )
						{
							UniformUtil::UpdateUniforms( rvUniforms, pLight->GetCamera()->GetProjection() );
						}
					} break;

				case LIGHTPROJI:
					{
						if( pLight )
						{
							const glm::mat4& proj =  pLight->GetCamera()->GetProjection();
							UniformUtil::UpdateUniforms( rvUniforms, glm::inverse( proj ) );
						}
					} break;
			}
	}
	*/
}



void GLRenderer::prepareFrameRendering( const Camera* pCamera, const SceneManager* pScene )
{
	using namespace ShaderSemantics;
	typedef std::vector<IUniform*>								UniformVecT;
	typedef std::map<ShaderSemantics::Semantic, UniformVecT>	UniformMapT;

	const glm::mat4& matWorld = pScene->getRootNode()->getGlobalTransformMAT();
	const glm::mat4& matView = pCamera->GetView();
	const glm::mat4& matProj = pCamera->GetProjection();
	const glm::mat4 matWorldView = matView * matWorld;
	const glm::mat4 matWorldViewProj = matProj * matWorldView;


	//Loop through all registered uniforms. Note:: Assume that there are only global uniforms registered
	UniformMapT& rUniformMap = m_pUniformRegistry->GetUniformMap();
	for( UniformMapT::iterator iter = rUniformMap.begin(); iter != rUniformMap.end(); ++iter )
	{
		UniformVecT& rvUniforms = iter->second;
		
		Semantic eSemantic = iter->first;

		if( UniformUtil::IsGlobalSemantic( eSemantic ) == false )
			continue;
		
		switch( eSemantic )
		{
			case WORLD:
				{
					UniformUtil::UpdateUniforms( rvUniforms, matWorld );
				} break;

			case WORLDI:
				{
					glm::mat4 mat = glm::inverse( matWorld );
					UniformUtil::UpdateUniforms( rvUniforms, mat );
				} break;

			case WORLDIT:
				{
					glm::mat4 mat = glm::inverseTranspose( matWorld );
					UniformUtil::UpdateUniforms( rvUniforms, mat );
				} break;

			case VIEW:
				{
					UniformUtil::UpdateUniforms( rvUniforms, matView );
				} break;

			case VIEWI:
				{
					glm::mat4 mat = glm::inverse( matView );
					UniformUtil::UpdateUniforms( rvUniforms, mat );
				} break;

			case VIEWIT:
				{
					glm::mat4 mat = glm::inverseTranspose( matView );
					UniformUtil::UpdateUniforms( rvUniforms, mat );
				} break;

			case WORLDVIEW:
				{
					UniformUtil::UpdateUniforms( rvUniforms, matWorldView );
				} break;

			case WORLDVIEWI:
				{
					glm::mat4 mat = glm::inverse( matWorldView );
					UniformUtil::UpdateUniforms( rvUniforms, mat );
				} break;

			case WORLDVIEWIT:
				{
					glm::mat4 mat = glm::inverseTranspose( matWorldView );
					UniformUtil::UpdateUniforms( rvUniforms, mat );
				} break;

			case PROJECTION:
				{
					UniformUtil::UpdateUniforms( rvUniforms, matProj );
				} break;

			case PROJECTIONI:
				{
					glm::mat4 mat = glm::inverse( matProj );
					UniformUtil::UpdateUniforms( rvUniforms, mat );
				} break;

			case PROJECTIONIT:
				{
					glm::mat4 mat = glm::inverseTranspose( matProj );
					UniformUtil::UpdateUniforms( rvUniforms, mat );
				} break;

			case WORLDVIEWPROJECTION:
				{
					UniformUtil::UpdateUniforms( rvUniforms, matWorldViewProj );
				} break;

			case WORLDVIEWPROJECTIONI:
				{
					glm::mat4 mat = glm::inverse( matWorldViewProj );
					UniformUtil::UpdateUniforms( rvUniforms, mat );
				} break;

			case WORLDVIEWPROJECTIONIT:
				{
					glm::mat4 mat = glm::inverseTranspose( matWorldViewProj );
					UniformUtil::UpdateUniforms( rvUniforms, mat );
				} break;

			case TIME:
				{
					static float fTime = 0.0001f;
					fTime += 0.0001f;
					UniformUtil::UpdateUniforms( rvUniforms, fTime );
				} break;

		

			case FRUSTUM_NEAR:
				{
					UniformUtil::UpdateUniforms( rvUniforms, pCamera->getNearPlane() );
				} break;

			case FRUSTUM_FAR:
				{
					UniformUtil::UpdateUniforms( rvUniforms, pCamera->getFarPlane() );
				} break;

			case FRUSTUM_YFOV:
				{
					UniformUtil::UpdateUniforms( rvUniforms, pCamera->getFovRad() );
				} break;

			case SCREEN_WIDTH:
				{
					UniformUtil::UpdateUniforms( rvUniforms, m_v4Viewport.z );
				} break;

			case SCREEN_HEIGHT:
				{
					UniformUtil::UpdateUniforms( rvUniforms, m_v4Viewport.w );
				} break;

			case SCREEN_SIZE:
				{
					UniformUtil::UpdateUniforms( rvUniforms, glm::vec2( m_v4Viewport.z, m_v4Viewport.w ) );
				} break;

			case SCREEN_RATIO:
				{
					UniformUtil::UpdateUniforms( rvUniforms, (float)  m_v4Viewport.z / (float) m_v4Viewport.w  );
				} break;

			case SCREEN_TEXTURESTEP:
				{
					 UniformUtil::UpdateUniforms( rvUniforms, glm::vec2( 1.0f / (float)  m_v4Viewport.z, 1.0f / (float) m_v4Viewport.w ) );
				} break;

			case AMBIENT_LIGHT:
				{
					const glm::vec4& ambient = m_pEngine->GetAmbientLightColor();
					glm::vec3 v3Amb( ambient.x, ambient.y, ambient.z );
					UniformUtil::UpdateUniforms( rvUniforms, v3Amb );
				} break;

			case CLEAR_COLOR:
				{
					const glm::vec4& clear = m_pEngine->GetClearColor();
					glm::vec3 v3Clear ( clear.x, clear.y, clear.z );
					UniformUtil::UpdateUniforms( rvUniforms, v3Clear );
				} break;

			case HDR_EXPOSURE:
				{
					UniformUtil::UpdateUniforms( rvUniforms, m_pEngine->GetHDRExposure() );
				} break;

			case B_TONEMAPPING_ENABLED:
				{
					UniformUtil::UpdateUniforms( rvUniforms, m_pEngine->GetToneMappingEnabled() );
				} break;

			case B_BLOOM_ENABLED:
				{
					UniformUtil::UpdateUniforms( rvUniforms, m_pEngine->GetUseBloom() );
				} break;

			case LIGHT_ADAPTION_PERCENTAGE:
				{
					UniformUtil::UpdateUniforms( rvUniforms, m_pEngine->GetHDRlightAdaption() );
				} break;

			case USE_DEBUG_TEXTURES:
				{
					UniformUtil::UpdateUniforms( rvUniforms, m_pEngine->GetDebugTexturesVisible() );
				} break;

			case CAMERAPOSWORLD:
				{
					UniformUtil::UpdateUniforms( rvUniforms, pCamera->getPosition() );
				} break;

		} //end switch
	} 
}

void GLRenderer::prepareMeshRendering( const Mesh* pMesh, Shader* pShader, const Camera* pCamera, const SceneManager* pScene, const glm::mat4& matModelWorld, const Material* pMaterial )
{
	using namespace ShaderSemantics;

	const glm::mat4& matWorld = pScene->getRootNode()->getGlobalTransformMAT();
	const glm::mat4& matView = pCamera->GetView();
	const glm::mat4& matProj = pCamera->GetProjection();
	const glm::mat4 matModelWorldViewProj = matProj * matView * matModelWorld;
	const glm::mat4 matWorldView = matView * matWorld;
	const glm::mat4 matModelWorldView = matView * matModelWorld;

	const std::vector<Light*>& vLights = pScene->getCachedLights();

	const Light* pLight						= NULL;
	const DirectionalLight* pDirLight		= NULL;
	const PointLight*		pPointLight		= NULL;
	const SpotLight*		pSpotLight		= NULL;
	

	const std::vector<IUniform*>& rvUniforms = pShader->GetUniforms();

	for( uint i = 0; i < rvUniforms.size(); ++i )
	{
		IUniform* pUniform = rvUniforms[ i ];
		ShaderSemantics::Semantic eSemantic = pUniform->GetSemantic();

		if( UniformUtil::IsGlobalSemantic( eSemantic ) )
			continue;

		if( m_bActiveLightMode )
		{
			if( vLights.size() > m_uCurrLightIdx )
				pLight = vLights[ m_uCurrLightIdx ];
		}


		if( pLight )
		{
				if( pLight->getLightType() == Light::LIGHTTYPE_DIRECTIONAL )
					pDirLight = dynamic_cast<const DirectionalLight*>( pLight );
				else if( pLight->getLightType() == Light::LIGHTTYPE_POINT )
					pPointLight = dynamic_cast<const PointLight*>( pLight );
				else if( pLight->getLightType() == Light::LIGHTTYPE_SPOT )
					pSpotLight = dynamic_cast<const SpotLight*>( pLight );
		}
		
		switch( eSemantic )
		{
		case MODEL:
			{

			} break;

		case MODELI:
			{

			} break;

		case MODELIT:
			{

			} break;

		case MODELWORLD:
			{
				UniformUtil::UpdateUniform( pUniform, matModelWorld ); 
			} break;

		case MODELWORLDI:
			{
				glm::mat4 mat = glm::inverse( matModelWorld );
				UniformUtil::UpdateUniform( pUniform, mat );
			} break;

		case MODELWORLDIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matModelWorld );
				UniformUtil::UpdateUniform( pUniform, mat );
			} break;

		case MODELWORLDVIEW:
			{
				UniformUtil::UpdateUniform( pUniform, matModelWorldView );
			} break;

		case MODELWORLDVIEWI:
			{
				glm::mat4 mat = glm::inverse( matModelWorldView );
				UniformUtil::UpdateUniform( pUniform, mat );
			} break;

		case MODELWORLDVIEWIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matModelWorldView );
				UniformUtil::UpdateUniform( pUniform, mat );
			} break;

		case MODELWORLDVIEWPROJECTION:
			{
				UniformUtil::UpdateUniform( pUniform, matModelWorldViewProj );
			} break;

		case MODELWORLDVIEWPROJECTIONI:
			{
				glm::mat4 mat = glm::inverse( matModelWorldViewProj );
				UniformUtil::UpdateUniform( pUniform, mat );
			} break;

		case MODELWORLDVIEWPROJECTIONIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matModelWorldViewProj );
				UniformUtil::UpdateUniform( pUniform, mat );
			} break;

		case MODELWORLDLIGHTVIEW:
			{
				if( pLight )
				{
					glm::mat4 mwlv = pLight->GetCamera()->GetView() * matModelWorld;
					UniformUtil::UpdateUniform( pUniform, mwlv );
				}

			} break;

			default:
				//If not found here, this Uniform will be handled by the material
				pMaterial->UpdateUniform( pUniform );
			break;

		}
	}

}

void GLRenderer::RenderMesh(  const Mesh* pMesh, Shader* pShader, const Camera* pCamera, const SceneManager* pScene, const glm::mat4& matModelWorld, const Material* pMaterial /* = NULL */ )
{
	if( !pMaterial )
		pMaterial = pMesh->GetMaterial();

//#ifdef _DEBUG
		pMaterial->ValidateMaterial();
//#endif

	prepareMeshRendering( pMesh, pShader, pCamera, pScene, matModelWorld, pMaterial );
	
	const VertexDeclaration* pVertexInfo = pMesh->GetVertexInfo();
	const std::vector<VertexElement>& vVertexElements = pVertexInfo->GetVertexElements();
	const std::vector<Light*>& vLights = pScene->getCachedLights();

	glBindBuffer( GL_ARRAY_BUFFER, pVertexInfo->GetVertexBufferLoc() );

	if( pVertexInfo->GetUseIndices() )
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pVertexInfo->GetIndexBufferLoc() );

	GLuint uEnabledVertexAttribs[ 20 ];
	uint8 u8AttribIdx = 0;

	pShader->ApplyShader();

	const std::vector<IUniform*>& rAttributes = pShader->GetAttributes();

	//Loop through all attributes and set Attrib pointers accordingly
	for( uint i = 0; i < rAttributes.size(); ++i )
	{
		const IUniform* pAttribute = rAttributes[ i ];
		ShaderSemantics::Semantic eSemantic = pAttribute->GetSemantic();
		GLint iAttributeHandle = pAttribute->GetGLhandle();

		
		const VertexElement* pElement;

		if( !pVertexInfo->GetVertexElement( eSemantic, &pElement ) )
			break;

			if( iAttributeHandle > GL_MAX_VERTEX_ATTRIBS )
				break;

			glEnableVertexAttribArray( iAttributeHandle );
			uEnabledVertexAttribs[ u8AttribIdx++ ] = iAttributeHandle;
			glVertexAttribPointer( iAttributeHandle, pElement->GetDataCount(), GL_FLOAT, GL_FALSE, pVertexInfo->GetStride(), BUFFER_OFFSET( pElement->GetOffset() ) );
	}
	
	//Now submit all uniforms to GL that have changed
	pShader->CleanUniforms();

	if( pVertexInfo->GetUseIndices() )
		glDrawElements( pVertexInfo->GetPrimitiveType(), pVertexInfo->GetIndexCount(), GL_UNSIGNED_INT, 0 );
	else
		glDrawArrays( pVertexInfo->GetPrimitiveType(), 0, pVertexInfo->GetVertexCount() );


	for( uint8 uIdx = 0; uIdx < u8AttribIdx; ++uIdx )
		glDisableVertexAttribArray( uEnabledVertexAttribs[ uIdx ] );

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//TODO: don't reset textures
	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindTexture( GL_TEXTURE_1D, 0 );
	glUseProgram(0);


	//Below: old semantics-code
	/*
	const glm::mat4& matWorld = pScene->getRootNode()->getGlobalTransformMAT();
	const glm::mat4& matView = pCamera->GetView();
	const glm::mat4& matProj = pCamera->GetProjection();
	const glm::mat4 matWorldViewProj = matProj * matView * matWorld;
	const glm::mat4 matModelWorldViewProj = matProj * matView * matModelWorld;
	const glm::mat4 matWorldView = matView * matWorld;
	const glm::mat4 matModelWorldView = matView * matModelWorld;
	const std::vector<SShaderSemantic>& vSemantics = pShader->GetSemantics();

	
	
	for( uint i = 0; i < vSemantics.size(); ++i )
	{
		using namespace ShaderSemantics;

		const SShaderSemantic& semantic = vSemantics[ i ];
		
		const Light* pLight						= NULL;
		const DirectionalLight* pDirLight		= NULL;
		const PointLight*		pPointLight		= NULL;
		const SpotLight*		pSpotLight		= NULL;

		if(semantic.eSemantic > LIGHTTYPES_START && semantic.eSemantic < LIGHTTYPES_END )
		{
			if( m_bActiveLightMode )
			{
				if( vLights.size() > m_uCurrLightIdx )
					pLight = vLights[ m_uCurrLightIdx ];
			}

			else if( vLights.size() > semantic.iN )
			{
				pLight = vLights[ semantic.iN ];
			}

			if( pLight )
			{
				if( pLight->getLightType() == Light::LIGHTTYPE_DIRECTIONAL )
					pDirLight = dynamic_cast<const DirectionalLight*>( pLight );
				else if( pLight->getLightType() == Light::LIGHTTYPE_POINT )
					pPointLight = dynamic_cast<const PointLight*>( pLight );
				else if( pLight->getLightType() == Light::LIGHTTYPE_SPOT )
					pSpotLight = dynamic_cast<const SpotLight*>( pLight );
			}
		}
		

		switch( semantic.eSemantic )
		{
			//Attributes
		case POSITION:
		case NORMAL:
		case UV0:
		case UV1:
		case UV2:
		case UV3:
		case UV4:
		case UV5:
		case UV6:
		case UV7:
		case TANGENT:
		case BITANGENT:
		case VERTEXCOLOR:
			{
				const VertexElement* pElement;

				if( !pVertexInfo->GetVertexElement( semantic.eSemantic, &pElement ) )
					break;

				if( semantic.uGLlocation > GL_MAX_VERTEX_ATTRIBS )
					break;

				glEnableVertexAttribArray( semantic.uGLlocation );
				uEnabledVertexAttribs[ u8AttribIdx++ ] = semantic.uGLlocation;
				glVertexAttribPointer( semantic.uGLlocation, pElement->GetDataCount(), GL_FLOAT, GL_FALSE, pVertexInfo->GetStride(), BUFFER_OFFSET( pElement->GetOffset() ) );
				
			} break;


			//Uniforms
		case MODEL:
			{

			} break;

		case MODELI:
			{

			} break;

		case MODELIT:
			{

			} break;

		case WORLD:
			{
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, &(matWorld[0].x) );
			} break;

		case WORLDI:
			{
				glm::mat4 mat = glm::inverse( matWorld );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case WORLDIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matWorld );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case MODELWORLD:
			{
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, &(matModelWorld[0].x) );
			} break;

		case MODELWORLDI:
			{
				glm::mat4 mat = glm::inverse( matModelWorld );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case MODELWORLDIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matModelWorld );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case VIEW:
			{
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, &(matView[0].x) );
			} break;

		case VIEWI:
			{
				glm::mat4 mat = glm::inverse( matView );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case VIEWIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matView );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case MODELWORLDVIEW:
			{
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( matModelWorldView ) );
			} break;

		case MODELWORLDVIEWI:
			{
				glm::mat4 mat = glm::inverse( matModelWorldView );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case MODELWORLDVIEWIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matModelWorldView );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case WORLDVIEW:
			{
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( matWorldView ) );
			} break;

		case WORLDVIEWI:
			{
				glm::mat4 mat = glm::inverse( matModelWorldView );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case WORLDVIEWIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matWorldView );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;


		case PROJECTION:
			{
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, (float*) &(matProj[0].x) );
			} break;

		case PROJECTIONI:
			{
				glm::mat4 mat = glm::inverse( matProj );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case PROJECTIONIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matProj );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case WORLDVIEWPROJECTION:
			{
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, &(matWorldViewProj[0].x) );
			} break;

		case WORLDVIEWPROJECTIONI:
			{
				glm::mat4 mat = glm::inverse( matWorldViewProj );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case WORLDVIEWPROJECTIONIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matWorldViewProj );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case MODELWORLDVIEWPROJECTION:
			{
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, &(matModelWorldViewProj[0].x) );
			} break;

		case MODELWORLDVIEWPROJECTIONI:
			{
				glm::mat4 mat = glm::inverse( matModelWorldViewProj );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case MODELWORLDVIEWPROJECTIONIT:
			{
				glm::mat4 mat = glm::inverseTranspose( matModelWorldViewProj );
				glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mat ) );
			} break;

		case TIME:
			{
				static float fTime = 0.0001f;
				fTime += 0.0001f;
				glUniform1f( semantic.uGLlocation, fTime );

			} break;

		case LIGHTDIRWORLD:
			{
				if( pDirLight )
				{
					const glm::vec3& dirWS = pDirLight->getCachedDirection();
					glUniform3f( semantic.uGLlocation, dirWS.x, dirWS.y, dirWS.z );
				}

				else if( pSpotLight )
				{
					const glm::vec3& dirWS = pSpotLight->getCachedDirection();
					glUniform3f( semantic.uGLlocation, dirWS.x, dirWS.y, dirWS.z );
				}

			} break;

		case LIGHTDIRVIEW:
			{
				if( pDirLight )
				{
					const glm::vec3& dirWS = pDirLight->getCachedDirection();
					glm::vec4 dirVS4 = matView * glm::vec4( dirWS.x, dirWS.y, dirWS.z, 0.0f );
					glm::vec3 dirVS = glm::normalize( glm::vec3( dirVS4.x, dirVS4.y, dirVS4.z ) );

					glUniform3f( semantic.uGLlocation, dirVS.x, dirVS.y, dirVS.z );

					//std::stringstream ss;
					//ss << dirVS.x << "\t" << dirVS.y << "\t" << dirVS.z;
					//LOG( ss.str() );
				}

				else if( pSpotLight )
				{
					const glm::vec3& dirWS = pSpotLight->getCachedDirection();
					glm::vec4 dirVS4 = matView * glm::vec4( dirWS.x, dirWS.y, dirWS.z, 0.0f );
					glm::vec3 dirVS = glm::normalize( glm::vec3( dirVS4.x, dirVS4.y, dirVS4.z ) );

					glUniform3f( semantic.uGLlocation, dirVS.x, dirVS.y, dirVS.z );
				}
			} break;

		case LIGHTPOSWORLD:
			{
				if( pLight )
				{
					const glm::vec3& posWS = pLight->getCachedPosition();

					glUniform3f( semantic.uGLlocation, posWS.x, posWS.y, posWS.z );
				}
			} break;

		case LIGHTPOSVIEW:
			{
				if( pLight )
				{
					const glm::vec3& posWS = pLight->getCachedPosition();

					glm::vec4 posVS4 = matView * glm::vec4( posWS.x, posWS.y, posWS.z, 1.0 );
					glm::vec3 posVS = glm::vec3( posVS4.x, posVS4.y, posVS4.z );

					glUniform3fv( semantic.uGLlocation, 1, glm::value_ptr( posVS ) );
				}
			} break;

		case LIGHTCOLOR:
			{
				if( pLight )
				{
					const glm::vec3& color = pLight->getColor();
					glUniform3fv( semantic.uGLlocation, 1, glm::value_ptr( color ) );
				}

			} break;

		case LIGHTINTENSITY:
			{
				if( pLight )
				{
					float fI = pLight->getIntensity();
					glUniform1f( semantic.uGLlocation, fI );
				}

			} break;

		case LIGHTCOLORINTENSITY:
			{
				if( pLight )
				{
					const glm::vec3& color = pLight->getColor() * pLight->getIntensity();
					glUniform3fv( semantic.uGLlocation, 1, glm::value_ptr( color ) );
				}
			} break;

		case LIGHTRSTART:
			{
				if( pPointLight )
				{
					float fRstart = pPointLight->getFalloffStart();
					glUniform1f( semantic.uGLlocation, fRstart );
				}
			} break;

		case LIGHTREND:
			{
				if( pPointLight )
				{
					float fRend = pPointLight->getFalloffEnd();
					glUniform1f( semantic.uGLlocation, fRend );
				}
			} break;

		case FRUSTUM_NEAR:
			{
				glUniform1f( semantic.uGLlocation, pCamera->getNearPlane() );
			} break;

		case FRUSTUM_FAR:
			{
				glUniform1f( semantic.uGLlocation, pCamera->getFarPlane() );
			} break;

		case FRUSTUM_YFOV:
			{
				glUniform1f( semantic.uGLlocation, pCamera->getFovRad() );
			} break;

		case SCREEN_WIDTH:
			{
				glUniform1i( semantic.uGLlocation, m_v4Viewport.z );
			} break;

		case SCREEN_HEIGHT:
			{
				glUniform1i( semantic.uGLlocation, m_v4Viewport.w );
			} break;

		case SCREEN_SIZE:
			{
				glUniform2f( semantic.uGLlocation, m_v4Viewport.z, m_v4Viewport.w );
			} break;

		case SCREEN_RATIO:
			{
				glUniform1f( semantic.uGLlocation, (float)  m_v4Viewport.z / (float) m_v4Viewport.w );
			} break;

		case SCREEN_TEXTURESTEP:
			{
				glUniform2f( semantic.uGLlocation, 1.0f / (float)  m_v4Viewport.z, 1.0f / (float) m_v4Viewport.w  );
			} break;

		case AMBIENT_LIGHT:
			{
				const glm::vec4& ambient = m_pEngine->GetAmbientLightColor();
				glUniform3f( semantic.uGLlocation, ambient.x, ambient.y, ambient.z );
			} break;

		case CLEAR_COLOR:
			{
				const glm::vec4& clear = m_pEngine->GetClearColor();
				glUniform3f( semantic.uGLlocation, clear.x, clear.y, clear.z );
			} break;

		case HDR_EXPOSURE:
			{
				glUniform1f( semantic.uGLlocation, m_pEngine->GetHDRExposure() );
			} break;

		case B_TONEMAPPING_ENABLED:
			{
				glUniform1i( semantic.uGLlocation, m_pEngine->GetToneMappingEnabled() );
			} break;

		case B_BLOOM_ENABLED:
			{
				glUniform1i( semantic.uGLlocation, m_pEngine->GetUseBloom() ); 
			} break;

		case LIGHT_ADAPTION_PERCENTAGE:
			{
				glUniform1f( semantic.uGLlocation, m_pEngine->GetHDRlightAdaption() );
			} break;

		case USE_DEBUG_TEXTURES:
			{
				glUniform1i( semantic.uGLlocation, m_pEngine->GetDebugTexturesVisible() );
			} break;

		case CAMERAPOSWORLD:
			{
				glUniform3fv( semantic.uGLlocation, 1, glm::value_ptr( pCamera->getPosition() ) );
			} break;

		case LIGHTVIEW:
			{
				if( pLight )
				{
					glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( pLight->GetCamera()->GetView() ) );
				}
			} break;

		case LIGHTVIEWI:
			{
				if( pLight )
				{
					glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( pLight->GetCamera()->GetViewInv()  ) );
				}
			} break;

		case LIGHTPROJ:
			{
				if( pLight )
				{
					glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( pLight->GetCamera()->GetProjection() ) );
				}
			} break;

		case LIGHTPROJI:
			{
				if( pLight )
				{
					const glm::mat4& proj =  pLight->GetCamera()->GetProjection();
					glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( glm::inverse( proj ) ) );
				}
			} break;

		case MODELWORLDLIGHTVIEW:
			{
				if( pLight )
				{
					glm::mat4 mwlv = pLight->GetCamera()->GetView() * matModelWorld;
					glUniformMatrix4fv( semantic.uGLlocation, 1, GL_FALSE, glm::value_ptr( mwlv ) );
				}

			} break;


		default:
			//If not found here, this semantic will be handled by the material
			pMaterial->SendShaderSemantic( semantic );
		break;

		} //end switch
	} //end loop semantics

	*/

	
}









