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
	m_pShader = new GPUProgram();
	m_pShader->LoadShader( "Shader\\FSQuad_Postpro_simple.vert", "Shader\\ToneMapping\\FSQuad_ToneMap.frag" ); 

	assignAttributeSemantic( m_pShader,  "pos" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pShader,  "uv" ,  ShaderSemantics::UV0 );

	assignUniformSemantic( m_pShader,  "exposure" ,  ShaderSemantics::HDR_EXPOSURE );
	assignUniformSemantic( m_pShader,  "bToneMappingEnabled" ,  ShaderSemantics::B_TONEMAPPING_ENABLED );

	assignUniformSemantic( m_pShader,  "inputTex" ,  ShaderSemantics::TEXTURE,  0 );
	assignUniformSemantic( m_pShader,  "avgLuminanceTex" ,  ShaderSemantics::TEXTURE,  1 );

	return true;
}


GLuint MAT_FSquad_ToneMapping::GetTextureAtIndex( uint uIdx ) const
{
	return uIdx == 0 ? m_uInputTexture : m_uAvgLuminanceTexture;
}
