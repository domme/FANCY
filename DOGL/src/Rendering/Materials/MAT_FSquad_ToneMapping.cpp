#include "MAT_FSquad_ToneMapping.h"

MAT_FSquad_ToneMapping::MAT_FSquad_ToneMapping() : Material(), m_uAvgLuminanceTexture( GLUINT_HANDLE_INVALID ), m_uInputTexture( GLUINT_HANDLE_INVALID )
{

}

MAT_FSquad_ToneMapping::MAT_FSquad_ToneMapping( MAT_FSquad_ToneMapping& other ) : Material( other )
{
	m_uAvgLuminanceTexture = other.m_uAvgLuminanceTexture;
	m_uInputTexture = other.m_uInputTexture;
}

MAT_FSquad_ToneMapping::~MAT_FSquad_ToneMapping()
{
	Material::~Material();
}

bool MAT_FSquad_ToneMapping::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader\\FSQuad_Postpro_simple.vert", "Shader\\ToneMapping\\FSQuad_ToneMap.frag" ); 

	assignAttributeSemantic( m_pForwardShader,  "pos" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pForwardShader,  "uv" ,  ShaderSemantics::UV0 );

	assignUniformSemantic( m_pForwardShader,  "exposure" ,  ShaderSemantics::HDR_EXPOSURE );
	assignUniformSemantic( m_pForwardShader,  "bToneMappingEnabled" ,  ShaderSemantics::B_TONEMAPPING_ENABLED );

	assignUniformSemantic( m_pForwardShader,  "inputTex" ,  ShaderSemantics::TEXTURE,  0 );
	assignUniformSemantic( m_pForwardShader,  "avgLuminanceTex" ,  ShaderSemantics::TEXTURE,  1 );

	return true;
}


GLuint MAT_FSquad_ToneMapping::GetTextureAtIndex( uint uIdx ) const
{
	return uIdx == 0 ? m_uInputTexture : m_uAvgLuminanceTexture;
}
