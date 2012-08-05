#include "MAT_TexturedNormalSpecular.h"
#include "../UniformUtil.h"

MAT_TexturedNormalSpecular::MAT_TexturedNormalSpecular() : Material()
{

}

MAT_TexturedNormalSpecular::MAT_TexturedNormalSpecular( MAT_TexturedNormalSpecular& other ) : Material( other )
{
	m_clDiffuseTexture = GLTexture( other.m_clDiffuseTexture );
	m_clNormalTexture = GLTexture( other.m_clNormalTexture );
	m_clSpecTexture = GLTexture( other.m_clSpecTexture );
	m_clGlossTexture = GLTexture( other.m_clGlossTexture );
	m_fBumpIntensity = other.m_fBumpIntensity;
}

MAT_TexturedNormalSpecular::~MAT_TexturedNormalSpecular()
{
	Material::~Material();
}

bool MAT_TexturedNormalSpecular::Init()
{
	m_pDeferredShader = new Shader();
	m_pDeferredShader->LoadShader( "Shader/MAT_TexturedNormalSpecular.vert", "Shader/MAT_TexturedNormalSpecular.frag" );
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
	assignUniformSemantic( m_pDeferredShader,  "specTex" ,  ShaderSemantics::TEXTURE, 2 );
	assignUniformSemantic( m_pDeferredShader,  "glossTex" ,  ShaderSemantics::TEXTURE, 3 );

	assignUniformSemantic( m_pDeferredShader,  "fBumpIntensity" ,  ShaderSemantics::BUMPINTENSITY );
	assignUniformSemantic( m_pDeferredShader,  "fNear" ,  ShaderSemantics::FRUSTUM_NEAR );
	assignUniformSemantic( m_pDeferredShader,  "fFar" ,  ShaderSemantics::FRUSTUM_FAR );

	//m_clDiffuseTexture.SetTexture( "Textures/UV.tga" );
	//m_clNormalTexture.SetTexture( "Textures/UV.tga" );

	m_clGlossTexture.SetTexture( "Textures/white.tga" );
	m_clSpecTexture.SetTexture( "Textures/white.tga" );

	return true;
}

GLuint MAT_TexturedNormalSpecular::GetTextureAtIndex( uint uIdx ) const
{
	switch( uIdx )
	{
		case 0:
			return m_clDiffuseTexture.getGlLocation();
		break;

		case 1:
			return m_clNormalTexture.getGlLocation();
		break;

		case 2:
			return m_clSpecTexture.getGlLocation();
		break;

		case 3:
			return m_clGlossTexture.getGlLocation();
		break;
	}

	return GLUINT_HANDLE_INVALID;
}

const glm::vec3& MAT_TexturedNormalSpecular::GetTextureSizeAtIndex( uint uIdx ) const
{

	switch( uIdx )
	{
	case 0:
		return m_clDiffuseTexture.GetTextureSize();
		break;

	case 1:
		return m_clNormalTexture.GetTextureSize();
		break;

	case 2:
		return m_clSpecTexture.GetTextureSize();
		break;

	case 3:
		return m_clGlossTexture.GetTextureSize();
		break;
	}

	return glm::vec3( 0.0f, 0.0f, 0.0f );
}

void MAT_TexturedNormalSpecular::UpdateUniform( IUniform* pUniform ) const
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


