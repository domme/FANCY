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
	m_pShader = new Shader();
	m_pShader->LoadShader( "Shader\\Lights\\Pointlight_Illumination.vert", "Shader\\Lights\\Pointlight_Illumination.frag" );
	
	assignAttributeSemantic( m_pShader,  "position" , ShaderSemantics::POSITION );

	assignUniformSemantic( m_pShader,  "fRend" , ShaderSemantics::LIGHTREND );
	assignUniformSemantic( m_pShader,  "v3PointlightPosVS" , ShaderSemantics::LIGHTPOSVIEW );
	assignUniformSemantic( m_pShader,  "matProj" , ShaderSemantics::PROJECTION );
	return true;
}


GLuint MAT_Pointlight_Illumination::GetTextureAtIndex( uint uIdx ) const
{
	return GLUINT_HANDLE_INVALID;
}
	