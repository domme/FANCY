#include "MAT_FSquad_FXAA.h"

MAT_FSquad_FXAA::MAT_FSquad_FXAA() : Material(), m_uInputTexture( GLUINT_HANDLE_INVALID )
{

}

MAT_FSquad_FXAA::MAT_FSquad_FXAA( MAT_FSquad_FXAA& other ) : Material( other )
{
	m_uInputTexture = other.m_uInputTexture;
}

MAT_FSquad_FXAA::~MAT_FSquad_FXAA()
{
	Material::~Material();
}

bool MAT_FSquad_FXAA::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader\\FSQuad_Postpro_simple.vert", "Shader\\FXAA\\FSQuad_FXAA.frag" ); 

	assignAttributeSemantic( m_pForwardShader,  "pos" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pForwardShader,  "uv" , ShaderSemantics::UV0 );

	assignUniformSemantic( m_pForwardShader,  "pixelStep" , ShaderSemantics::SCREEN_TEXTURESTEP );
	assignUniformSemantic( m_pForwardShader,  "inputTexture" , ShaderSemantics::TEXTURE , 0 );

	
	return true;
}


GLuint MAT_FSquad_FXAA::GetTextureAtIndex( uint uIdx ) const
{
	return m_uInputTexture;
}
