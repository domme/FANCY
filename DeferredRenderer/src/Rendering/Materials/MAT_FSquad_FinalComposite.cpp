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
	m_pShader = new Shader();
	m_pShader->LoadShader( "Shader/FSQuad_Postpro.vert", "Shader/FSQuad_FinalComposite.frag" );
	assignAttributeSemantic( m_pShader,  "pos", ShaderSemantics::POSITION );
	assignAttributeSemantic( m_pShader,  "uv", ShaderSemantics::UV0 );
	
	
	assignUniformSemantic( m_pShader,  "fRatio", ShaderSemantics::SCREEN_RATIO );
	assignUniformSemantic( m_pShader,  "fYfov", ShaderSemantics::FRUSTUM_YFOV );
	assignUniformSemantic( m_pShader,  "fFar", ShaderSemantics::FRUSTUM_FAR );
	assignUniformSemantic( m_pShader,  "iScreenHeight", ShaderSemantics::SCREEN_HEIGHT );

	assignUniformSemantic( m_pShader,  "colorGloss", ShaderSemantics::TEXTURE , 1 );
	assignUniformSemantic( m_pShader,  "localIllum", ShaderSemantics::TEXTURE , 2 );

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