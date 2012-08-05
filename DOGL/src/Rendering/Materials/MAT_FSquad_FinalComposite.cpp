#include "MAT_FSquad_FinalComposite.h"
#include "../GLDeferredRenderer.h"

MAT_FSquad_FinalComposite::MAT_FSquad_FinalComposite() : Material(), 
m_uColorGlossTex( GLUINT_HANDLE_INVALID ), 
m_uLocalIllumTexture( GLUINT_HANDLE_INVALID )
{

}

MAT_FSquad_FinalComposite::MAT_FSquad_FinalComposite( MAT_FSquad_FinalComposite& other ) : Material( other )
{
	m_uColorGlossTex = other.m_uColorGlossTex;
	m_uLocalIllumTexture = other.m_uLocalIllumTexture;
}

MAT_FSquad_FinalComposite::~MAT_FSquad_FinalComposite()
{

}

bool MAT_FSquad_FinalComposite::Init()
{
	m_pForwardShader = new Shader();
	m_pForwardShader->LoadShader( "Shader/FSQuad_Postpro.vert", "Shader/FSQuad_FinalComposite.frag" );
	assignAttributeSemantic( m_pForwardShader,  "pos", ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pForwardShader,  "uv", ShaderSemantics::UV0 );
	
	
	assignUniformSemantic( m_pForwardShader,  "fRatio", ShaderSemantics::SCREEN_RATIO );
	assignUniformSemantic( m_pForwardShader,  "fYfov", ShaderSemantics::FRUSTUM_YFOV );
	assignUniformSemantic( m_pForwardShader,  "fFar", ShaderSemantics::FRUSTUM_FAR );
	assignUniformSemantic( m_pForwardShader,  "iScreenHeight", ShaderSemantics::SCREEN_HEIGHT );

	assignUniformSemantic( m_pForwardShader,  "colorGloss", ShaderSemantics::TEXTURE , 1 );
	assignUniformSemantic( m_pForwardShader,  "localIllum", ShaderSemantics::TEXTURE , 2 );

	return true;
}

GLuint MAT_FSquad_FinalComposite::GetTextureAtIndex( uint uIdx ) const
{
	switch( uIdx )
	{
		case 1:
			return m_uColorGlossTex;

		case 2:
			return m_uLocalIllumTexture;
	}

	return GLUINT_HANDLE_INVALID;
}