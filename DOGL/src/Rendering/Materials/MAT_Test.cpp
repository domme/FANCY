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
	m_pDeferredShader = new Shader();
	m_pDeferredShader->LoadShader( "Shader\\Test.vert", "Shader\\Test.frag" );
	assignAttributeSemantic( m_pDeferredShader, "position" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pDeferredShader, "normal" , ShaderSemantics::NORMAL );
	assignAttributeSemantic( m_pDeferredShader, "uv" , ShaderSemantics::UV0 );
	assignAttributeSemantic( m_pDeferredShader, "tangent" , ShaderSemantics::TANGENT );

	assignUniformSemantic( m_pDeferredShader,  "WV" , ShaderSemantics::WORLDVIEW );
	assignUniformSemantic( m_pDeferredShader,  "MWVP" , ShaderSemantics::MODELWORLDVIEWPROJECTION );
	assignUniformSemantic( m_pDeferredShader,  "MWV" , ShaderSemantics::MODELWORLDVIEW );
	assignUniformSemantic( m_pDeferredShader,  "MWVIT" , ShaderSemantics::MODELWORLDVIEWIT );
	return true;
}


GLuint MAT_Test::GetTextureAtIndex( uint uIdx ) const
{
	return GLUINT_HANDLE_INVALID;
}
	