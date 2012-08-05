#include "MAT_FSquad_Bloom.h"

MAT_FSquad_Bloom::MAT_FSquad_Bloom() : 
Material(),
m_uBloomTexture( GLUINT_HANDLE_INVALID ),
m_uInputTexture( GLUINT_HANDLE_INVALID )
{

}

MAT_FSquad_Bloom::MAT_FSquad_Bloom( MAT_FSquad_Bloom& other ) :
Material( other ),
m_uBloomTexture( GLUINT_HANDLE_INVALID ),
m_uInputTexture( GLUINT_HANDLE_INVALID )
{
	m_uBloomTexture = other.m_uBloomTexture;
	m_uInputTexture = other.m_uInputTexture;
}

MAT_FSquad_Bloom::~MAT_FSquad_Bloom()
{
	Material::~Material();
}

bool MAT_FSquad_Bloom::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader\\FSQuad_Postpro_simple.vert", "Shader\\FSQuad_Bloom.frag" ); 

	assignAttributeSemantic( m_pForwardShader,  "pos"  , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pForwardShader,  "uv"  , ShaderSemantics::UV0 );

	assignUniformSemantic( m_pForwardShader,  "tImg"  , ShaderSemantics::TEXTURE , 0 );
	assignUniformSemantic( m_pForwardShader,  "tBloomSource"  , ShaderSemantics::TEXTURE , 1 );
	assignUniformSemantic( m_pForwardShader,  "bUseBloom"  , ShaderSemantics::B_BLOOM_ENABLED );
	return true;
}

GLuint MAT_FSquad_Bloom::GetTextureAtIndex( uint uIdx ) const
{
	return uIdx == 0 ? m_uInputTexture : m_uBloomTexture;
}
