#include "MAT_Pointlight_Illumination.h"

MAT_Pointlight_Illumination::MAT_Pointlight_Illumination() : Material()
{

}

MAT_Pointlight_Illumination::MAT_Pointlight_Illumination( MAT_Pointlight_Illumination& other ) : Material( other )
{

}

MAT_Pointlight_Illumination::~MAT_Pointlight_Illumination()
{
	Material::~Material();
}

bool MAT_Pointlight_Illumination::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader\\Lights\\Pointlight_Illumination.vert", "Shader\\Lights\\Pointlight_Illumination.frag" );
	
	assignAttributeSemantic( m_pForwardShader,  "position" , ShaderSemantics::POSITION );

	assignUniformSemantic( m_pForwardShader,  "fRend" , ShaderSemantics::LIGHTREND );
	assignUniformSemantic( m_pForwardShader,  "v3PointlightPosVS" , ShaderSemantics::LIGHTPOSVIEW );
	assignUniformSemantic( m_pForwardShader,  "matProj" , ShaderSemantics::PROJECTION );
	return true;
}


GLuint MAT_Pointlight_Illumination::GetTextureAtIndex( uint uIdx ) const
{
	return GLUINT_HANDLE_INVALID;
}
	