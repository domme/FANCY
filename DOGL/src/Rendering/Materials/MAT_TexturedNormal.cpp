#include "MAT_TexturedNormal.h"
#include "../UniformUtil.h"

MAT_TexturedNormal::MAT_TexturedNormal() : Material()
{

}

MAT_TexturedNormal::MAT_TexturedNormal( MAT_TexturedNormal& other ) : Material( other )
{
	m_clDiffuseTexture = GLTexture( other.m_clDiffuseTexture );
	m_clNormalTexture = GLTexture( other.m_clNormalTexture );
	m_fBumpIntensity = other.m_fBumpIntensity;
}

MAT_TexturedNormal::~MAT_TexturedNormal()
{
	Material::~Material();
}

bool MAT_TexturedNormal::Init()
{
	m_pDeferredShader = new Shader();
	m_pDeferredShader->LoadShader( "Shader/MAT_TexturedNormal.vert", "Shader/MAT_TexturedNormal.frag" );
	assignAttributeSemantic( m_pDeferredShader, "position" ,  ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pDeferredShader, "normal" ,  ShaderSemantics::NORMAL );
	assignAttributeSemantic( m_pDeferredShader, "uv" ,  ShaderSemantics::UV0 );
	assignAttributeSemantic( m_pDeferredShader, "tangent" ,  ShaderSemantics::TANGENT );

	assignUniformSemantic( m_pDeferredShader,  "MWVP" ,  ShaderSemantics::MODELWORLDVIEWPROJECTION );
	assignUniformSemantic( m_pDeferredShader,  "MWV" ,  ShaderSemantics::MODELWORLDVIEW );
	assignUniformSemantic( m_pDeferredShader,  "MWVIT" ,  ShaderSemantics::MODELWORLDVIEWIT );

	assignUniformSemantic( m_pDeferredShader,  "v3SpecularColor" ,  ShaderSemantics::SPECULARCOLOR );
	assignUniformSemantic( m_pDeferredShader,  "fSpecExp" ,  ShaderSemantics::SPECULAREXPONENT );
	assignUniformSemantic( m_pDeferredShader,  "fGloss" ,  ShaderSemantics::GLOSSINESS ); 
	assignUniformSemantic( m_pDeferredShader,  "v3DiffuseReflectivity" ,  ShaderSemantics::DIFFUSEREFLECTIVITY );

	assignUniformSemantic( m_pDeferredShader,  "diffTex" ,  ShaderSemantics::TEXTURE, 0 );
	assignUniformSemantic( m_pDeferredShader,  "normTex" ,  ShaderSemantics::TEXTURE, 1 );
	assignUniformSemantic( m_pDeferredShader,  "fBumpIntensity" ,  ShaderSemantics::BUMPINTENSITY );
	assignUniformSemantic( m_pDeferredShader,  "fNear" ,  ShaderSemantics::FRUSTUM_NEAR );
	assignUniformSemantic( m_pDeferredShader,  "fFar" ,  ShaderSemantics::FRUSTUM_FAR );

	//m_clDiffuseTexture.SetTexture( "Textures/UV.tga" );
	//m_clNormalTexture.SetTexture( "Textures/UV.tga" );

	return true;
}

GLuint MAT_TexturedNormal::GetTextureAtIndex( uint uIdx ) const
{
	switch( uIdx )
	{
		case 0:
			return m_clDiffuseTexture.getGlLocation();
		break;

		case 1:
			return m_clNormalTexture.getGlLocation();
		break;
	}

	return 0;
}

const glm::vec3& MAT_TexturedNormal::GetTextureSizeAtIndex( uint uIdx ) const
{
	switch( uIdx )
	{
	case 0:
		return m_clDiffuseTexture.GetTextureSize();

	case 1:
		return m_clNormalTexture.GetTextureSize();
	}

	return glm::vec3( 0.0f, 0.0f, 0.0f );
}



void MAT_TexturedNormal::UpdateUniform( IUniform* pUniform ) const
{
	Material::UpdateUniform( pUniform );

	using namespace ShaderSemantics;

	switch( pUniform->GetSemantic() )
	{
		case BUMPINTENSITY:
			UniformUtil::UpdateUniform( pUniform, m_fBumpIntensity );
		break;
	}

}
