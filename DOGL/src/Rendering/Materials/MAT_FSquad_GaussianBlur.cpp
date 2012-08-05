#include "MAT_FSquad_GaussianBlur.h"
#include "../UniformUtil.h"

MAT_FSquad_GaussianBlur::MAT_FSquad_GaussianBlur() : Material(), m_uGaussTexture( GLUINT_HANDLE_INVALID ), m_uInputTexture( GLUINT_HANDLE_INVALID ), m_uOffsetTexture( GLUINT_HANDLE_INVALID )
{

}

MAT_FSquad_GaussianBlur::MAT_FSquad_GaussianBlur( MAT_FSquad_GaussianBlur& other ) : Material( other )
{
	m_uGaussTexture = other.m_uGaussTexture;
	m_uInputTexture = other.m_uInputTexture;
	m_uOffsetTexture = other.m_uOffsetTexture;
}

MAT_FSquad_GaussianBlur::~MAT_FSquad_GaussianBlur()
{
	Material::~Material();
}

bool MAT_FSquad_GaussianBlur::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader\\FSQuad_Postpro_simple.vert", "Shader\\FSQuad_GaussianBlur.frag" ); 

	assignAttributeSemantic( m_pForwardShader,  "pos" ,  ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pForwardShader,  "uv" ,  ShaderSemantics::UV0 );

	assignUniformSemantic( m_pForwardShader,  "tImg" ,  ShaderSemantics::TEXTURE,  0 );
	assignUniformSemantic( m_pForwardShader,  "tGauss" ,  ShaderSemantics::TEXTURE_1D,  1 );
	assignUniformSemantic( m_pForwardShader,  "tOffsets" ,  ShaderSemantics::TEXTURE_1D,  2 );

	assignUniformSemantic( m_pForwardShader,  "kernelSize" ,  ShaderSemantics::KERNEL_SIZE );

	assignUniformSemantic( m_pForwardShader,  "v2ImageSize" ,  ShaderSemantics::SCREEN_SIZE );
	assignUniformSemantic( m_pForwardShader,  "v2SamplingDir" ,  ShaderSemantics::SEPERATED_SAMPLING_DIR );
	return true;
}

void MAT_FSquad_GaussianBlur::UpdateUniform( IUniform* pUniform ) const
{
	Material::UpdateUniform( pUniform );

	switch( pUniform->GetSemantic() )
	{
	case ShaderSemantics::SEPERATED_SAMPLING_DIR:
		{
			glm::vec2 v2SamplingDir ( m_eSamplingDirection == ESamplingDirection::DIRECTION_HORIZONTAL ? 1.0 : 0.0, m_eSamplingDirection == ESamplingDirection::DIRECTION_VERTICAL ? 1.0 : 0.0 );
			UniformUtil::UpdateUniform( pUniform, v2SamplingDir );
		} break;

	case ShaderSemantics::KERNEL_SIZE:
		{
			int iKernelSize = m_uKernelSize;
			UniformUtil::UpdateUniform( pUniform, iKernelSize );
		} break;
	}
}


GLuint MAT_FSquad_GaussianBlur::GetTextureAtIndex( uint uIdx ) const
{
	switch( uIdx )
	{
		case 0:
			return m_uInputTexture;

		case 1:
			return m_uGaussTexture;

		case 2:
			return m_uOffsetTexture;
	}

	return GLUINT_HANDLE_INVALID;
}
