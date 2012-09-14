#include "MAT_FSquad_Textured.h"

MAT_FSquad_Textured::MAT_FSquad_Textured() : Material(), m_uTexture( GLUINT_HANDLE_INVALID )
{

}

MAT_FSquad_Textured::MAT_FSquad_Textured( MAT_FSquad_Textured& other ) : Material( other )
{
	m_uTexture = other.m_uTexture;
}

MAT_FSquad_Textured::~MAT_FSquad_Textured()
{

}

bool MAT_FSquad_Textured::Init()
{
	m_pShader = new Shader();
	m_pShader->LoadShader( "Shader/FSQuad_DebugImage.vert", "Shader/FSQuad_DebugImage.frag" );
	assignAttributeSemantic( m_pShader,  "pos" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pShader,  "uv" , ShaderSemantics::UV0 );
	assignUniformSemantic( m_pShader,  "img" , ShaderSemantics::TEXTURE , 0 );
	return true;
}

void MAT_FSquad_Textured::PrepareMaterialRendering( Mesh* pMesh, const glm::mat4& rObjectTransformMAT )
{

}

GLuint MAT_FSquad_Textured::GetTextureAtIndex( uint uIdx ) const
{
	if( uIdx == 0 )
		return m_uTexture;

	return GLUINT_HANDLE_INVALID;
}