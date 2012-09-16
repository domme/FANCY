
#include "../includes.h"

#include "GLRenderer.h"
#include "../IO/FileReader.h"
#include <assert.h>
#include "Materials/Material.h"
#include "../Geometry/VertexDeclarations.h"
#include "Shader.h"
#include "../Light/DirectionalLight.h"
#include "../Light/PointLight.h"
#include "../Light/SpotLight.h"
#include "../IO/ModelLoader.h"
#include "../Scene/AABoundingBox.h"
#include "Managers/TextureManager.h"

#include "../Scene/Camera.h"
#include "GLTexture.h"
#include "../Services/FBOservice.h"
#include "../Services/GLDebug.h"

#include "../IO/TextureLoader.h"

#include "IUniform.h"
#include "Uniform.h"
#include "UniformUtil.h"

const float PI = 3.14159265358979323846f;


GLRenderer::GLRenderer() : 
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
m_bColorMask_a( true ),
m_bShowDebugTextures( false ),
m_bUseFXAA( true ),
m_bUseToneMapping( true ),
m_bUseBloom( true ), 
m_fHDRexposure( 0.5f ),
m_fHDRlightAdaption( 0.04f )
{
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
	
}

void GLRenderer::init( uint uWidth, uint uHeight )
{
	m_pTextureManager = &TextureManager::getInstance();

	m_pUniformRegistry = &UniformRegistry::GetInstance();

	setDepthFunc( GL_LESS );
	setDepthTest( true );

	
	SetResolution( uWidth, uHeight );
}

void GLRenderer::SetResolution( uint uWidth, uint uHeight )
{
	m_uScreenWidth = uWidth;
	m_uScreenHeight = uHeight;
	setViewport( 0, 0, uWidth, uHeight );
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


void GLRenderer::prepareLightRendering( const Camera* pCamera, const Light* pLight )
{
	using namespace ShaderSemantics;
	typedef std::vector<IUniform*>								UniformVecT;
	typedef std::map<ShaderSemantics::Semantic, UniformVecT>	UniformMapT;

	const DirectionalLight* pDirLight		= NULL;
	const PointLight*		pPointLight		= NULL;
	const SpotLight*		pSpotLight		= NULL;
	
	const glm::mat4& matView = pCamera->GetView();
	
	if( pLight->getLightType() == Light::LIGHTTYPE_DIRECTIONAL )
		pDirLight = dynamic_cast<const DirectionalLight*>( pLight );
	else if( pLight->getLightType() == Light::LIGHTTYPE_POINT )
		pPointLight = dynamic_cast<const PointLight*>( pLight );
	else if( pLight->getLightType() == Light::LIGHTTYPE_SPOT )
		pSpotLight = dynamic_cast<const SpotLight*>( pLight );
	

	//Loop through all registered uniforms. Note:: Assume that there are only global uniforms registered
	UniformMapT& rUniformMap = m_pUniformRegistry->GetUniformMap();
	UniformMapT::iterator iter;
	UniformMapT::iterator iterEnd = rUniformMap.end();
	

	iter = rUniformMap.find( LIGHTDIRWORLD );

	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pDirLight )
		{
			const glm::vec3& dirWS = pDirLight->getDirection();
			UniformUtil::UpdateUniforms( rvUniforms, dirWS );
		}

		else if( pSpotLight )
		{
			const glm::vec3& dirWS = pSpotLight->getDirection();
			UniformUtil::UpdateUniforms( rvUniforms, dirWS );
		}
	}


	iter = rUniformMap.find( LIGHTDIRVIEW );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pDirLight )
		{
			const glm::vec3& dirWS = pDirLight->getDirection();
			glm::vec4 dirVS4 = matView * glm::vec4( dirWS.x, dirWS.y, dirWS.z, 0.0f );
			glm::vec3 dirVS = glm::normalize( glm::vec3( dirVS4.x, dirVS4.y, dirVS4.z ) );

			UniformUtil::UpdateUniforms( rvUniforms, dirVS );

			//std::stringstream ss;
			//ss << dirVS.x << "\t" << dirVS.y << "\t" << dirVS.z;
			//LOG( ss.str() );
		}

		else if( pSpotLight )
		{
			const glm::vec3& dirWS = pSpotLight->getDirection();
			glm::vec4 dirVS4 = matView * glm::vec4( dirWS.x, dirWS.y, dirWS.z, 0.0f );
			glm::vec3 dirVS = glm::normalize( glm::vec3( dirVS4.x, dirVS4.y, dirVS4.z ) );

			UniformUtil::UpdateUniforms( rvUniforms, dirVS );
		}
	}


	iter = rUniformMap.find( LIGHTPOSWORLD );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			glm::vec3 posWS = pLight->getPositionWS();
			UniformUtil::UpdateUniforms( rvUniforms, posWS );
		}
	}


	iter = rUniformMap.find( LIGHTPOSVIEW );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;


		if( pLight )
		{
			glm::vec3 posWS = pLight->getPositionWS();

			glm::vec4 posVS4 = matView * glm::vec4( posWS.x, posWS.y, posWS.z, 1.0 );
			glm::vec3 posVS = glm::vec3( posVS4.x, posVS4.y, posVS4.z );

			UniformUtil::UpdateUniforms( rvUniforms, posVS );
		}
	}


	iter = rUniformMap.find( LIGHTCOLOR );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;


		if( pLight )
		{
			const glm::vec3& color = pLight->getColor();
			UniformUtil::UpdateUniforms( rvUniforms, color );
		}

	}

	iter = rUniformMap.find( LIGHTINTENSITY );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			float fI = pLight->getIntensity();
			UniformUtil::UpdateUniforms( rvUniforms, fI );
		}

	}


	iter = rUniformMap.find( LIGHTCOLORINTENSITY );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			const glm::vec3& color = pLight->getColor() * pLight->getIntensity();
			UniformUtil::UpdateUniforms( rvUniforms, color );
		}

	}

	iter = rUniformMap.find( LIGHTRSTART );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pPointLight )
		{
			float fRstart = pPointLight->getFalloffStart();
			UniformUtil::UpdateUniforms( rvUniforms, fRstart );
		}

	}


	iter = rUniformMap.find( LIGHTREND );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pPointLight )
		{
			float fRend = pPointLight->getFalloffEnd();
			UniformUtil::UpdateUniforms( rvUniforms, fRend );
		}
	}

	iter = rUniformMap.find( LIGHTVIEW );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			UniformUtil::UpdateUniforms( rvUniforms, pLight->GetCamera()->GetView() );
		}
	}


	iter = rUniformMap.find( LIGHTVIEWI );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			UniformUtil::UpdateUniforms( rvUniforms, pLight->GetCamera()->GetViewInv() );
		}
	}


	iter = rUniformMap.find( LIGHTPROJ );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			UniformUtil::UpdateUniforms( rvUniforms, pLight->GetCamera()->GetProjection() );
		}	
	}


	iter = rUniformMap.find( LIGHTPROJI );
	if(  iter != iterEnd )
	{
		UniformVecT& rvUniforms = iter->second;

		if( pLight )
		{
			const glm::mat4& proj =  pLight->GetCamera()->GetProjection();
			UniformUtil::UpdateUniforms( rvUniforms, glm::inverse( proj ) );
		}
	}
}



void GLRenderer::prepareFrameRendering( const Camera* pCamera, const glm::mat4& matWorld )
{
	using namespace ShaderSemantics;
	typedef std::vector<IUniform*>								UniformVecT;
	typedef std::map<ShaderSemantics::Semantic, UniformVecT>	UniformMapT;

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
					const glm::vec4& ambient = m_v4AmbientLightColor;
					glm::vec3 v3Amb( ambient.x, ambient.y, ambient.z );
					UniformUtil::UpdateUniforms( rvUniforms, v3Amb );
				} break;

			case CLEAR_COLOR:
				{
					const glm::vec4& clear = m_v4ClearColor;
					glm::vec3 v3Clear ( clear.x, clear.y, clear.z );
					UniformUtil::UpdateUniforms( rvUniforms, v3Clear );
				} break;

			case HDR_EXPOSURE:
				{
					UniformUtil::UpdateUniforms( rvUniforms, GetHDRExposure() );
				} break;

			case B_TONEMAPPING_ENABLED:
				{
					UniformUtil::UpdateUniforms( rvUniforms, GetToneMappingEnabled() );
				} break;

			case B_BLOOM_ENABLED:
				{
					UniformUtil::UpdateUniforms( rvUniforms, GetUseBloom() );
				} break;

			case LIGHT_ADAPTION_PERCENTAGE:
				{
					UniformUtil::UpdateUniforms( rvUniforms, GetHDRlightAdaption() );
				} break;

			case USE_DEBUG_TEXTURES:
				{
					UniformUtil::UpdateUniforms( rvUniforms, GetDebugTexturesVisible() );
				} break;

			case CAMERAPOSWORLD:
				{
					UniformUtil::UpdateUniforms( rvUniforms, pCamera->getPosition() );
				} break;

		} //end switch
	} 
}

void GLRenderer::prepareMeshRendering( Mesh* pMesh, const glm::mat4& matModel, const glm::mat4& matWorld, const Camera* pCamera, Material* pMaterial /*= NULL*/, Shader* pShader /* = NULL */,  const Light* pLight /* = NULL */ )
{
	using namespace ShaderSemantics;

	const glm::mat4& matView = pCamera->GetView();
	const glm::mat4& matProj = pCamera->GetProjection();
	const glm::mat4 matModelWorld = matWorld * matModel;
	const glm::mat4 matModelWorldViewProj = matProj * matView * matModelWorld;
	const glm::mat4 matWorldView = matView * matWorld;
	const glm::mat4 matModelWorldView = matView * matModelWorld;

	
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

				else
					LOG( "LIGHT-information requested in render-call but not provided any light" );

			} break;

			default:
				//If not found here, this Uniform will be handled by the material
				pMaterial->UpdateUniform( pUniform );
			break;

		}
	}

}

void GLRenderer::RenderMesh( Mesh* pMesh, const glm::mat4& matModel, const glm::mat4& matWorld, const Camera* pCamera, Material* pMaterial /* = NULL */, Shader* pShader /* = NULL */, const Light* pLight /* = NULL */ ) 
{
	if( !pMaterial )
		pMaterial = pMesh->GetMaterial();

//#ifdef _DEBUG
		pMaterial->ValidateMaterial();
//#endif

	if( !pShader )
		pShader = pMaterial->GetShader();

	prepareMeshRendering( pMesh, matModel, matWorld, pCamera, pMaterial, pShader, pLight );
	
	const VertexDeclaration* pVertexInfo = pMesh->GetVertexInfo();
	const std::vector<VertexElement>& vVertexElements = pVertexInfo->GetVertexElements();
	
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
}









