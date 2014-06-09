#include "MAT_FSquad_PointLighting.h"
#include "../GLDeferredRenderer.h"

MAT_FSquad_PointLighting::MAT_FSquad_PointLighting() : Material(), m_uColorGlossTex( GLUINT_HANDLE_INVALID ), 
	m_uSpecTex( GLUINT_HANDLE_INVALID ),
	m_uDepthTex( GLUINT_HANDLE_INVALID ),
	m_uNormalTex( GLUINT_HANDLE_INVALID ),
	m_uCubeShadowTex( GLUINT_HANDLE_INVALID )
{

}

MAT_FSquad_PointLighting::MAT_FSquad_PointLighting( MAT_FSquad_PointLighting& other ) : Material( other )
{
	m_uColorGlossTex = other.m_uColorGlossTex;
	m_uSpecTex = other.m_uSpecTex;
	m_uDepthTex = other.m_uDepthTex;
	m_uNormalTex = other.m_uNormalTex;
}

MAT_FSquad_PointLighting::~MAT_FSquad_PointLighting()
{
	Material::~Material();
}

bool MAT_FSquad_PointLighting::Init()
{
	m_pShader = new GPUProgram();
	m_pShader->LoadShader( "Shader/FSQuad_Postpro.vert", "Shader/FSQuad_PointLighting.frag" );
	assignAttributeSemantic( m_pShader,  "pos" ,  ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pShader,  "uv" ,  ShaderSemantics::UV0 );

	assignUniformSemantic( m_pShader,  "fRatio" ,  ShaderSemantics::SCREEN_RATIO );
	assignUniformSemantic( m_pShader,  "fYfov" ,  ShaderSemantics::FRUSTUM_YFOV );
	assignUniformSemantic( m_pShader,  "fFar" ,  ShaderSemantics::FRUSTUM_FAR );
	assignUniformSemantic( m_pShader,  "fNear" ,  ShaderSemantics::FRUSTUM_NEAR );
	assignUniformSemantic( m_pShader,  "iScreenHeight" ,  ShaderSemantics::SCREEN_HEIGHT );
	assignUniformSemantic( m_pShader, "view", ShaderSemantics::VIEW );
	assignUniformSemantic( m_pShader, "viewI", ShaderSemantics::VIEWI );
	assignUniformSemantic( m_pShader, "lightView", ShaderSemantics::LIGHTVIEW );
	assignUniformSemantic( m_pShader, "lightProj", ShaderSemantics::LIGHTPROJ );
	
	
	assignUniformSemantic( m_pShader,  "colorGloss" ,  ShaderSemantics::TEXTURE,  GBuffer::ColorGloss );
	assignUniformSemantic( m_pShader,  "specN" ,  ShaderSemantics::TEXTURE,  GBuffer::Spec );
	assignUniformSemantic( m_pShader,  "normals" ,  ShaderSemantics::TEXTURE,  GBuffer::Normal );
	assignUniformSemantic( m_pShader,  "depth" ,  ShaderSemantics::TEXTURE,  GBuffer::Depth );
	
	assignUniformSemantic( m_pShader, "shadowCubeTex", ShaderSemantics::TEXTURE_CUBE, 5 );
	
	assignUniformSemantic( m_pShader,  "v3LightPosVS" ,  ShaderSemantics::LIGHTPOSVIEW );
	assignUniformSemantic( m_pShader,  "v3LightColor" ,  ShaderSemantics::LIGHTCOLORINTENSITY );
	assignUniformSemantic( m_pShader,  "fRstart" ,  ShaderSemantics::LIGHTRSTART );
	assignUniformSemantic( m_pShader,  "fRend" ,  ShaderSemantics::LIGHTREND );

	return true;
}

GLuint MAT_FSquad_PointLighting::GetTextureAtIndex( uint uIdx ) const
{
	switch( uIdx )
	{
	case GBuffer::ColorGloss:
		return m_uColorGlossTex;

	case GBuffer::Spec:
		return m_uSpecTex;

	case GBuffer::Normal:
		return m_uNormalTex;

	case GBuffer::Depth:
		return m_uDepthTex;

	case 5:
		return m_uCubeShadowTex;


	}

	return GLUINT_HANDLE_INVALID;
}