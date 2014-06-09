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
	m_pShader = new GPUProgram();
	m_pShader->LoadShader( "Shader\\FSQuad_Postpro_simple.vert", "Shader\\FSQuad_BrightPass.frag" ); 

	assignAttributeSemantic( m_pShader,  "pos" ,ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pShader,  "uv" ,ShaderSemantics::UV0 );

	assignUniformSemantic( m_pShader,  "tImg" ,ShaderSemantics::TEXTURE, 0 );
	return true;
}

GLuint MAT_FSquad_BrightPass::GetTextureAtIndex( uint uIdx ) const
{
	return m_uInputTexture;
}
