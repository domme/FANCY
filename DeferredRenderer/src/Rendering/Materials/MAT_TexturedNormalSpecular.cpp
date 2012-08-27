#include <Rendering/UniformUtil.h>

#include "MAT_TexturedNormalSpecular.h"


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
	m_pShader = new Shader();
	m_pShader->LoadShader( "Shader/MAT_TexturedNormalSpecular.vert", "Shader/MAT_TexturedNormalSpecular.frag" );
	assignAttributeSemantic( m_pShader, "position" ,  ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pShader, "normal" ,  ShaderSemantics::NORMAL );
	assignAttributeSemantic( m_pShader, "uv" ,  ShaderSemantics::UV0 );
	assignAttributeSemantic( m_pShader, "tangent" ,  ShaderSemantics::TANGENT );

	assignUniformSemantic( m_pShader,  "MWVP" ,  ShaderSemantics::MODELWORLDVIEWPROJECTION );
	assignUniformSemantic( m_pShader,  "MWV" ,  ShaderSemantics::MODELWORLDVIEW );
	assignUniformSemantic( m_pShader,  "MWVIT" ,  ShaderSemantics::MODELWORLDVIEWIT );

	assignUniformSemantic( m_pShader,  "v3SpecularColor" ,  ShaderSemantics::SPECULARCOLOR );
	assignUniformSemantic( m_pShader,  "fSpecExp" ,  ShaderSemantics::SPECULAREXPONENT );
	assignUniformSemantic( m_pShader,  "fGloss" ,  ShaderSemantics::GLOSSINESS ); 
	assignUniformSemantic( m_pShader,  "v3DiffuseReflectivity" ,  ShaderSemantics::DIFFUSEREFLECTIVITY );

	assignUniformSemantic( m_pShader,  "diffTex" ,  ShaderSemantics::TEXTURE, 0 );
	assignUniformSemantic( m_pShader,  "normTex" ,  ShaderSemantics::TEXTURE, 1 );
	assignUniformSemantic( m_pShader,  "specTex" ,  ShaderSemantics::TEXTURE, 2 );
	assignUniformSemantic( m_pShader,  "glossTex" ,  ShaderSemantics::TEXTURE, 3 );

	assignUniformSemantic( m_pShader,  "fBumpIntensity" ,  ShaderSemantics::BUMPINTENSITY );
	assignUniformSemantic( m_pShader,  "fNear" ,  ShaderSemantics::FRUSTUM_NEAR );
	assignUniformSemantic( m_pShader,  "fFar" ,  ShaderSemantics::FRUSTUM_FAR );

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

glm::vec3 MAT_TexturedNormalSpecular::GetTextureSizeAtIndex( uint uIdx ) const
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


