#include "MAT_Textured.h"

MAT_Textured::MAT_Textured() : Material()
{

}

MAT_Textured::MAT_Textured( MAT_Textured& other ) : Material( other )
{
	m_clDiffuseTexture = GLTexture( other.m_clDiffuseTexture );
}

MAT_Textured::~MAT_Textured()
{
	Material::~Material();
}

bool MAT_Textured::Init()
{
	m_pDeferredShader = new Shader();
	m_pDeferredShader->LoadShader( "Shader/MAT_Textured.vert", "Shader/MAT_Textured.frag" );
	assignAttributeSemantic( m_pDeferredShader, "position" , ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pDeferredShader, "normal" , ShaderSemantics::NORMAL );
	assignAttributeSemantic( m_pDeferredShader, "uv" , ShaderSemantics::UV0 );
	assignAttributeSemantic( m_pDeferredShader, "tangent" , ShaderSemantics::TANGENT );

	assignUniformSemantic( m_pDeferredShader,  "MWVP" , ShaderSemantics::MODELWORLDVIEWPROJECTION );
	assignUniformSemantic( m_pDeferredShader,  "MWV" , ShaderSemantics::MODELWORLDVIEW );
	assignUniformSemantic( m_pDeferredShader,  "MWVIT" , ShaderSemantics::MODELWORLDVIEWIT );

	assignUniformSemantic( m_pDeferredShader,  "v3SpecularColor" , ShaderSemantics::SPECULARCOLOR );
	assignUniformSemantic( m_pDeferredShader,  "fSpecExp" , ShaderSemantics::SPECULAREXPONENT );
	assignUniformSemantic( m_pDeferredShader,  "fGloss" , ShaderSemantics::GLOSSINESS ); 
	assignUniformSemantic( m_pDeferredShader,  "v3DiffuseReflectivity" , ShaderSemantics::DIFFUSEREFLECTIVITY );

	assignUniformSemantic( m_pDeferredShader,  "diffTex" , ShaderSemantics::TEXTURE,  0 );
	assignUniformSemantic( m_pDeferredShader,  "fNear" , ShaderSemantics::FRUSTUM_NEAR );
	assignUniformSemantic( m_pDeferredShader,  "fFar" , ShaderSemantics::FRUSTUM_FAR );

	m_clDiffuseTexture.SetTexture( "Textures/UV.tga" );

	return true;
}

GLuint MAT_Textured::GetTextureAtIndex( uint uIdx ) const
{
	if( uIdx == 0 )
		return m_clDiffuseTexture.getGlLocation();

	return GLUINT_HANDLE_INVALID;
}
