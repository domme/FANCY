#include <Rendering/UniformUtil.h>

#include "MAT_TexturedNormal.h"


MAT_TexturedNormal::MAT_TexturedNormal() : Material()
{

}

MAT_TexturedNormal::MAT_TexturedNormal( MAT_TexturedNormal& other ) : Material( other )
{
	m_clDiffuseTexture = Texture( other.m_clDiffuseTexture );
	m_clNormalTexture = Texture( other.m_clNormalTexture );
	m_fBumpIntensity = other.m_fBumpIntensity;
}

MAT_TexturedNormal::~MAT_TexturedNormal()
{
	Material::~Material();
}

bool MAT_TexturedNormal::Init()
{
	m_pShader = new GPUProgram();
	m_pShader->LoadShader( "Shader/MAT_TexturedNormal.vert", "Shader/MAT_TexturedNormal.frag" );
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
	assignUniformSemantic( m_pShader,  "fBumpIntensity" ,  ShaderSemantics::BUMPINTENSITY );
	assignUniformSemantic( m_pShader,  "fNear" ,  ShaderSemantics::FRUSTUM_NEAR );
	assignUniformSemantic( m_pShader,  "fFar" ,  ShaderSemantics::FRUSTUM_FAR );

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

glm::vec3 MAT_TexturedNormal::GetTextureSizeAtIndex( uint uIdx ) const
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
