#include "MAT_FSquad_Textured3D.h"

MAT_FSquad_Textured3D::MAT_FSquad_Textured3D() :VolumeMaterial()
{

}

MAT_FSquad_Textured3D::MAT_FSquad_Textured3D( MAT_FSquad_Textured3D& other ) : VolumeMaterial( other )
{
	
}

MAT_FSquad_Textured3D::~MAT_FSquad_Textured3D()
{

}

bool MAT_FSquad_Textured3D::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader/FSQuad_DebugImage.vert", "Shader/FSQuad_DebugImage3D.frag" );
	assignAttributeSemantic( m_pForwardShader,  "pos" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pForwardShader,  "uv" , ShaderSemantics::UV0 );
	assignUniformSemantic( m_pForwardShader, 	"img" , ShaderSemantics::TEXTURE_3D , 0 );
	assignUniformSemantic( m_pForwardShader,  "time" , ShaderSemantics::TIME );
	return true;
}

void MAT_FSquad_Textured3D::PrepareMaterialRendering( Mesh* pMesh, const glm::mat4& rObjectTransformMAT )
{

}

GLuint MAT_FSquad_Textured3D::GetTextureAtIndex( uint uIdx ) const
{
	if( uIdx == 0 )
		return m_uVolumeTexture;

	return GLUINT_HANDLE_INVALID;
}