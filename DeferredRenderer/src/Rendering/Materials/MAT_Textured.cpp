#include "MAT_Textured.h"

MAT_Textured::MAT_Textured() : Material()
{

}

MAT_Textured::MAT_Textured( MAT_Textured& other ) : Material( other )
{
	m_clDiffuseTexture = GLTexture( other.m_clDiffuseTexture );
}

MAT_Textured::~MAT_Textured()
{
	Material::~Material();
}

bool MAT_Textured::Init()
{
	m_pShader = new Shader();
	m_pShader->LoadShader( "Shader/MAT_Textured.vert", "Shader/MAT_Textured.frag" );
	assignAttributeSemantic( m_pShader, "position" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pShader, "normal" , ShaderSemantics::NORMAL );
	assignAttributeSemantic( m_pShader, "uv" , ShaderSemantics::UV0 );
	assignAttributeSemantic( m_pShader, "tangent" , ShaderSemantics::TANGENT );

	assignUniformSemantic( m_pShader,  "MWVP" , ShaderSemantics::MODELWORLDVIEWPROJECTION );
	assignUniformSemantic( m_pShader,  "MWV" , ShaderSemantics::MODELWORLDVIEW );
	assignUniformSemantic( m_pShader,  "MWVIT" , ShaderSemantics::MODELWORLDVIEWIT );

	assignUniformSemantic( m_pShader,  "v3SpecularColor" , ShaderSemantics::SPECULARCOLOR );
	assignUniformSemantic( m_pShader,  "fSpecExp" , ShaderSemantics::SPECULAREXPONENT );
	assignUniformSemantic( m_pShader,  "fGloss" , ShaderSemantics::GLOSSINESS ); 
	assignUniformSemantic( m_pShader,  "v3DiffuseReflectivity" , ShaderSemantics::DIFFUSEREFLECTIVITY );

	assignUniformSemantic( m_pShader,  "diffTex" , ShaderSemantics::TEXTURE,  0 );
	assignUniformSemantic( m_pShader,  "fNear" , ShaderSemantics::FRUSTUM_NEAR );
	assignUniformSemantic( m_pShader,  "fFar" , ShaderSemantics::FRUSTUM_FAR );

	m_clDiffuseTexture.SetTexture( "Textures/UV.tga" );

	return true;
}

GLuint MAT_Textured::GetTextureAtIndex( uint uIdx ) const
{
	if( uIdx == 0 )
		return m_clDiffuseTexture.getGlLocation();

	return GLUINT_HANDLE_INVALID;
}
