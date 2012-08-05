#include "MAT_FSquad_PointLighting.h"
#include "../GLDeferredRenderer.h"

MAT_FSquad_PointLighting::MAT_FSquad_PointLighting() : Material(), m_uColorGlossTex( GLUINT_HANDLE_INVALID ), 
	m_uSpecTex( GLUINT_HANDLE_INVALID ),
	m_uDepthTex( GLUINT_HANDLE_INVALID ),
	m_uNormalTex( GLUINT_HANDLE_INVALID )
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
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader/FSQuad_Postpro.vert", "Shader/FSQuad_PointLighting.frag" );
	assignAttributeSemantic( m_pForwardShader,  "pos" ,  ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pForwardShader,  "uv" ,  ShaderSemantics::UV0 );

	assignUniformSemantic( m_pForwardShader,  "fRatio" ,  ShaderSemantics::SCREEN_RATIO );
	assignUniformSemantic( m_pForwardShader,  "fYfov" ,  ShaderSemantics::FRUSTUM_YFOV );
	assignUniformSemantic( m_pForwardShader,  "fFar" ,  ShaderSemantics::FRUSTUM_FAR );
	assignUniformSemantic( m_pForwardShader,  "fNear" ,  ShaderSemantics::FRUSTUM_NEAR );
	assignUniformSemantic( m_pForwardShader,  "iScreenHeight" ,  ShaderSemantics::SCREEN_HEIGHT );
	assignUniformSemantic( m_pForwardShader, "view", ShaderSemantics::VIEW );

	assignUniformSemantic( m_pForwardShader,  "colorGloss" ,  ShaderSemantics::TEXTURE,  GBuffer::ColorGloss );
	assignUniformSemantic( m_pForwardShader,  "specN" ,  ShaderSemantics::TEXTURE,  GBuffer::Spec );
	assignUniformSemantic( m_pForwardShader,  "normals" ,  ShaderSemantics::TEXTURE,  GBuffer::Normal );
	assignUniformSemantic( m_pForwardShader,  "depth" ,  ShaderSemantics::TEXTURE,  GBuffer::Depth );
	
	assignUniformSemantic( m_pForwardShader,  "v3LightPosVS" ,  ShaderSemantics::LIGHTPOSVIEW );
	assignUniformSemantic( m_pForwardShader,  "v3LightColor" ,  ShaderSemantics::LIGHTCOLORINTENSITY );
	assignUniformSemantic( m_pForwardShader,  "fRstart" ,  ShaderSemantics::LIGHTRSTART );
	assignUniformSemantic( m_pForwardShader,  "fRend" ,  ShaderSemantics::LIGHTREND );

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
	}

	return GLUINT_HANDLE_INVALID;
}