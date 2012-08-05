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

	m_pDeferredShader = new Shader();
	m_pDeferredShader->LoadShader( "Shader\\MAT_Colored.vert", "Shader\\MAT_Colored.frag" );

	assignAttributeSemantic( m_pDeferredShader, "position", POSITION );
	

	assignAttributeSemantic( m_pDeferredShader, "normal", NORMAL );
	assignAttributeSemantic( m_pDeferredShader, "uv", UV0 );

	assignUniformSemantic(m_pDeferredShader, "MWVP" , MODELWORLDVIEWPROJECTION );
	assignUniformSemantic(m_pDeferredShader, "MWV" , ShaderSemantics::MODELWORLDVIEW );
	assignUniformSemantic(m_pDeferredShader, "MWVIT" , ShaderSemantics::MODELWORLDVIEWIT );			   
	assignUniformSemantic(m_pDeferredShader, "materialColor" , ShaderSemantics::MATERIALCOLOR );
	assignUniformSemantic(m_pDeferredShader, "v3SpecularColor" , ShaderSemantics::SPECULARCOLOR );
	assignUniformSemantic(m_pDeferredShader, "fSpecExp" , ShaderSemantics::SPECULAREXPONENT );
	assignUniformSemantic(m_pDeferredShader, "fGloss" , ShaderSemantics::GLOSSINESS ); 
	assignUniformSemantic(m_pDeferredShader, "fNear" , ShaderSemantics::FRUSTUM_NEAR );
	assignUniformSemantic(m_pDeferredShader,  "fFar" , ShaderSemantics::FRUSTUM_FAR );
	return true;
}


GLuint MAT_Colored::GetTextureAtIndex( uint uIdx ) const
{
	return 0;
}
	