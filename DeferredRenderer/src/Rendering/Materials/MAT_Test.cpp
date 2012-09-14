#include "MAT_Test.h"

MAT_Test::MAT_Test() : Material()
{

}

MAT_Test::MAT_Test( MAT_Test& other ) : Material( other )
{

}

MAT_Test::~MAT_Test()
{
	Material::~Material();
}

bool MAT_Test::Init()
{
	m_pShader = new Shader();
	m_pShader->LoadShader( "Shader\\Test.vert", "Shader\\Test.frag" );
	assignAttributeSemantic( m_pShader, "position" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pShader, "normal" , ShaderSemantics::NORMAL );
	assignAttributeSemantic( m_pShader, "uv" , ShaderSemantics::UV0 );
	assignAttributeSemantic( m_pShader, "tangent" , ShaderSemantics::TANGENT );

	assignUniformSemantic( m_pShader,  "WV" , ShaderSemantics::WORLDVIEW );
	assignUniformSemantic( m_pShader,  "MWVP" , ShaderSemantics::MODELWORLDVIEWPROJECTION );
	assignUniformSemantic( m_pShader,  "MWV" , ShaderSemantics::MODELWORLDVIEW );
	assignUniformSemantic( m_pShader,  "MWVIT" , ShaderSemantics::MODELWORLDVIEWIT );
	return true;
}


GLuint MAT_Test::GetTextureAtIndex( uint uIdx ) const
{
	return GLUINT_HANDLE_INVALID;
}
	