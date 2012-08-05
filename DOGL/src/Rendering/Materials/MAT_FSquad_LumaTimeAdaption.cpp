#include "MAT_FSquad_LumaTimeAdaption.h"

MAT_FSquad_LumaTimeAdaption::MAT_FSquad_LumaTimeAdaption() : Material(), m_uCurrFrameLumaTex( GLUINT_HANDLE_INVALID ), m_uLastFrameLumaTex( GLUINT_HANDLE_INVALID )
{

}

MAT_FSquad_LumaTimeAdaption::MAT_FSquad_LumaTimeAdaption( MAT_FSquad_LumaTimeAdaption& other ) : Material( other )
{
	m_uCurrFrameLumaTex = other.m_uCurrFrameLumaTex;
	m_uLastFrameLumaTex = other.m_uLastFrameLumaTex;
}

MAT_FSquad_LumaTimeAdaption::~MAT_FSquad_LumaTimeAdaption()
{
	Material::~Material();
}

bool MAT_FSquad_LumaTimeAdaption::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader\\FSQuad_Postpro_simple.vert", "Shader\\ToneMapping\\FSQuad_LumaTimeAdaption.frag" ); 

	assignAttributeSemantic( m_pForwardShader,  "pos" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pForwardShader,  "uv" , ShaderSemantics::UV0 );
		
	assignUniformSemantic( m_pForwardShader,  "currentFrameLuma" , ShaderSemantics::TEXTURE , 0 );
	assignUniformSemantic( m_pForwardShader,  "lastFrameLuma" , ShaderSemantics::TEXTURE , 1 );

	assignUniformSemantic( m_pForwardShader,  "fAdaptionRate" , ShaderSemantics::LIGHT_ADAPTION_PERCENTAGE );
	return true;
}


GLuint MAT_FSquad_LumaTimeAdaption::GetTextureAtIndex( uint uIdx ) const
{
	return uIdx == 0 ? m_uCurrFrameLumaTex : m_uLastFrameLumaTex;
}
