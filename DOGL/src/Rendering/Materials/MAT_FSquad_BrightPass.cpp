#include "MAT_FSquad_BrightPass.h"

MAT_FSquad_BrightPass::MAT_FSquad_BrightPass() : Material(), m_uInputTexture( GLUINT_HANDLE_INVALID )
{

}

MAT_FSquad_BrightPass::MAT_FSquad_BrightPass( MAT_FSquad_BrightPass& other ) : Material( other )
{
	m_uInputTexture = other.m_uInputTexture;
}

MAT_FSquad_BrightPass::~MAT_FSquad_BrightPass()
{
	Material::~Material();
}

bool MAT_FSquad_BrightPass::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader\\FSQuad_Postpro_simple.vert", "Shader\\FSQuad_BrightPass.frag" ); 

	assignAttributeSemantic( m_pForwardShader,  "pos" ,ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pForwardShader,  "uv" ,ShaderSemantics::UV0 );

	assignUniformSemantic( m_pForwardShader,  "tImg" ,ShaderSemantics::TEXTURE, 0 );
	return true;
}

GLuint MAT_FSquad_BrightPass::GetTextureAtIndex( uint uIdx ) const
{
	return m_uInputTexture;
}
