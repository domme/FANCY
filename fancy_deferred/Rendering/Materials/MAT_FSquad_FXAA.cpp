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
	m_pShader = new Shader();
	m_pShader->LoadShader( "Shader\\FSQuad_Postpro_simple.vert", "Shader\\FXAA\\FSQuad_FXAA.frag" ); 

	assignAttributeSemantic( m_pShader,  "pos" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pShader,  "uv" , ShaderSemantics::UV0 );

	assignUniformSemantic( m_pShader,  "pixelStep" , ShaderSemantics::SCREEN_TEXTURESTEP );
	assignUniformSemantic( m_pShader,  "inputTexture" , ShaderSemantics::TEXTURE , 0 );

	
	return true;
}


GLuint MAT_FSquad_FXAA::GetTextureAtIndex( uint uIdx ) const
{
	return m_uInputTexture;
}
