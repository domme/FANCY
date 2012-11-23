#include "MAT_Colored.h"

MAT_Colored::MAT_Colored() : Material()
{

}

MAT_Colored::MAT_Colored( MAT_Colored& other ) : Material( other )
{

}

MAT_Colored::~MAT_Colored()
{
	Material::~Material();
}

bool MAT_Colored::Init()
{
	using namespace ShaderSemantics;

	m_pShader = new Shader();
	m_pShader->LoadShader( "Shader\\MAT_Colored.vert", "Shader\\MAT_Colored.frag" );

	assignAttributeSemantic( m_pShader, "position", POSITION );
	

	assignAttributeSemantic( m_pShader, "normal", NORMAL );
	assignAttributeSemantic( m_pShader, "uv", UV0 );

	assignUniformSemantic(m_pShader, "MWVP" , MODELWORLDVIEWPROJECTION );
	assignUniformSemantic(m_pShader, "MWV" , ShaderSemantics::MODELWORLDVIEW );
	assignUniformSemantic(m_pShader, "MWVIT" , ShaderSemantics::MODELWORLDVIEWIT );			   
	assignUniformSemantic(m_pShader, "materialColor" , ShaderSemantics::MATERIALCOLOR );
	assignUniformSemantic(m_pShader, "v3SpecularColor" , ShaderSemantics::SPECULARCOLOR );
	assignUniformSemantic(m_pShader, "fSpecExp" , ShaderSemantics::SPECULAREXPONENT );
	assignUniformSemantic(m_pShader, "fGloss" , ShaderSemantics::GLOSSINESS ); 
	assignUniformSemantic(m_pShader, "fNear" , ShaderSemantics::FRUSTUM_NEAR );
	assignUniformSemantic(m_pShader,  "fFar" , ShaderSemantics::FRUSTUM_FAR );
	return true;
}


GLuint MAT_Colored::GetTextureAtIndex( uint uIdx ) const
{
	return 0;
}
	