#include "MAT_VolCube_Raycast_Simple.h"

MAT_VolCube_Raycast_Simple::MAT_VolCube_Raycast_Simple() : VolumeMaterial()
{

}

MAT_VolCube_Raycast_Simple::MAT_VolCube_Raycast_Simple( MAT_VolCube_Raycast_Simple& other ) : VolumeMaterial( other )
{
	
}

MAT_VolCube_Raycast_Simple::~MAT_VolCube_Raycast_Simple()
{

}

bool MAT_VolCube_Raycast_Simple::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader/Volume/VolCube_Rasterize_Raycast.vert", "Shader/Volume/VolCube_Raycast_simple.frag" );
	assignAttributeSemantic( m_pForwardShader,  "pos" , ShaderSemantics::POSITION );

	assignUniformSemantic( m_pForwardShader,  "MWVP" , ShaderSemantics::MODELWORLDVIEWPROJECTION );
	//assignUniformSemantic( m_pForwardShader,  "MWV" , ShaderSemantics::MODELWORLDVIEW );

	assignUniformSemantic( m_pForwardShader,  "screenDimensions" , ShaderSemantics::SCREEN_SIZE );

	assignUniformSemantic( m_pForwardShader,  "texTransferFunction" , ShaderSemantics::TEXTURE_1D, 0 );
	assignUniformSemantic( m_pForwardShader,  "texRayEndPositions" , ShaderSemantics::TEXTURE, 1 );
	assignUniformSemantic( m_pForwardShader,  "texVolume" , ShaderSemantics::TEXTURE_3D, 2 );
	assignUniformSemantic( m_pForwardShader,  "texDSM" , ShaderSemantics::TEXTURE_3D, 3 );
	assignUniformSemantic( m_pForwardShader,  "bNoTransferFunktion" , ShaderSemantics::USE_DEBUG_TEXTURES );
	assignUniformSemantic( m_pForwardShader,  "bUseShadows" , ShaderSemantics::VOLMAT_USE_SHADOWS );
	assignUniformSemantic( m_pForwardShader,  "v3LightPosWS" , ShaderSemantics::LIGHTPOSWORLD, 0 );
	assignUniformSemantic( m_pForwardShader,  "v3LightColorIntensity" , ShaderSemantics::LIGHTCOLORINTENSITY, 0 );
	assignUniformSemantic( m_pForwardShader,  "fLightRstart" , ShaderSemantics::LIGHTRSTART, 0 );
	assignUniformSemantic( m_pForwardShader,  "fLightRend" , ShaderSemantics::LIGHTREND, 0 );
	assignUniformSemantic( m_pForwardShader,  "m4ModelWorldI" , ShaderSemantics::MODELWORLDI );
	assignUniformSemantic( m_pForwardShader,  "m4ModelWorld" , ShaderSemantics::MODELWORLD );
	assignUniformSemantic( m_pForwardShader,  "m4LightProj" , ShaderSemantics::LIGHTPROJ );

	assignUniformSemantic( m_pForwardShader,  "m4ModelWorldLightView" , ShaderSemantics::MODELWORLDLIGHTVIEW, 0 );

	assignUniformSemantic( m_pForwardShader,  "v3CameraPosWS" , ShaderSemantics::CAMERAPOSWORLD );
	assignUniformSemantic( m_pForwardShader,  "fSamplingRate" , ShaderSemantics::SAMPLING_RATE );
	assignUniformSemantic( m_pForwardShader,  "fWindowValue" , ShaderSemantics::VOLUME_WINDOW_VALUE );
	assignUniformSemantic( m_pForwardShader,  "v3VolumeSize" , ShaderSemantics::TEXTURE_SIZE, 2 );

	assignUniformSemantic( m_pForwardShader,  "clearColor" , ShaderSemantics::CLEAR_COLOR );
	return true;
}

void MAT_VolCube_Raycast_Simple::PrepareMaterialRendering( Mesh* pMesh, const glm::mat4& rObjectTransformMAT )
{

}

GLuint MAT_VolCube_Raycast_Simple::GetTextureAtIndex( uint uIdx ) const
{
	switch( uIdx )
	{
		case 0:
			return m_uTransferFunctionTexture;

		case 1:
			return m_uRaystartTexture;

		case 2:
			return m_uVolumeTexture;

		case 3:
			return m_uShadowTexture;

		default:
			return GLUINT_HANDLE_INVALID;
	}
}


const glm::vec3& MAT_VolCube_Raycast_Simple::GetTextureSizeAtIndex( uint uIdx ) const
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

