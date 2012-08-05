#include "MAT_VolCube_Rasterize.h"

MAT_VolCube_Rasterize::MAT_VolCube_Rasterize() : Material()
{

}

MAT_VolCube_Rasterize::MAT_VolCube_Rasterize( MAT_VolCube_Rasterize& other ) : Material( other )
{
	
}

MAT_VolCube_Rasterize::~MAT_VolCube_Rasterize()
{

}

bool MAT_VolCube_Rasterize::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader/Volume/VolCube_Rasterize.vert", "Shader/Volume/VolCube_Rasterize.frag" );
	assignAttributeSemantic( m_pForwardShader,  "pos" , ShaderSemantics::POSITION );

	assignUniformSemantic( m_pForwardShader,  "MWVP" , ShaderSemantics::MODELWORLDVIEWPROJECTION );
	return true;
}

void MAT_VolCube_Rasterize::PrepareMaterialRendering( Mesh* pMesh, const glm::mat4& rObjectTransformMAT )
{

}

GLuint MAT_VolCube_Rasterize::GetTextureAtIndex( uint uIdx ) const
{
	return GLUINT_HANDLE_INVALID;
}