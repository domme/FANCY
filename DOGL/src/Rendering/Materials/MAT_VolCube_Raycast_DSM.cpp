#include "MAT_VolCube_Raycast_DSM.h"

MAT_VolCube_Raycast_DSM::MAT_VolCube_Raycast_DSM() : VolumeMaterial()
{

}

MAT_VolCube_Raycast_DSM::MAT_VolCube_Raycast_DSM( MAT_VolCube_Raycast_DSM& other ) : VolumeMaterial( other )
{
	
}

MAT_VolCube_Raycast_DSM::~MAT_VolCube_Raycast_DSM()
{

}

bool MAT_VolCube_Raycast_DSM::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader/Volume/VolCube_Rasterize_Raycast.vert", "Shader/Volume/VolCube_Raycast_DSM.frag" );
	assignAttributeSemantic( m_pForwardShader,  "pos" ,  ShaderSemantics::POSITION );

	assignUniformSemantic( m_pForwardShader,  "MWVP" ,  ShaderSemantics::MODELWORLDVIEWPROJECTION );
	//assignUniformSemantic( m_pForwardShader,  "MWV" ,  ShaderSemantics::MODELWORLDVIEW );

	assignUniformSemantic( m_pForwardShader,  "screenDimensions" ,  ShaderSemantics::SCREEN_SIZE );

	assignUniformSemantic( m_pForwardShader,  "texTransferFunction" ,  ShaderSemantics::TEXTURE_1D,  0 );
	assignUniformSemantic( m_pForwardShader,  "texRayEndPositions" ,  ShaderSemantics::TEXTURE,  1 );
	assignUniformSemantic( m_pForwardShader,  "texVolume" ,  ShaderSemantics::TEXTURE_3D,  2 );
		
	assignUniformSemantic( m_pForwardShader,  "m4ModelWorldView" ,  ShaderSemantics::MODELWORLDVIEW );
	assignUniformSemantic( m_pForwardShader,  "fSamplingRate" ,  ShaderSemantics::SAMPLING_RATE );
	assignUniformSemantic( m_pForwardShader,  "fWindowValue" ,  ShaderSemantics::VOLUME_WINDOW_VALUE );
	assignUniformSemantic( m_pForwardShader,  "v3VolumeSize" ,  ShaderSemantics::TEXTURE_SIZE,  2 );
	return true;
}

void MAT_VolCube_Raycast_DSM::PrepareMaterialRendering( Mesh* pMesh, const glm::mat4& rObjectTransformMAT )
{

}

GLuint MAT_VolCube_Raycast_DSM::GetTextureAtIndex( uint uIdx ) const
{
	switch( uIdx )
	{
		case 0:
			return m_uTransferFunctionTexture;

		case 1:
			return m_uRaystartTexture;

		case 2:
			return m_uVolumeTexture;

		default:
			return GLUINT_HANDLE_INVALID;
	}
}


const glm::vec3& MAT_VolCube_Raycast_DSM::GetTextureSizeAtIndex( uint uIdx ) const
{
	switch( uIdx )
	{
	case 0:
		return m_v3TransferFunktionSize;

	//case 1:
		//return m_uRaystartTexture;

	case 2:
		return m_v3VolumeTextureSize;

	default:
		return glm::vec3( 0.0f, 0.0f, 0.0f );
	}

}

