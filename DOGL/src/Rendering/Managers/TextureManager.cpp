#include "TextureManager.h"
#include "../../IO/TextureTGA.h"
#include "../../IO/PathService.h"

TextureManager& TextureManager::getInstance()
{
	static TextureManager instance;
	return instance;
}

TextureManager::TextureManager()
{
	memset( &m_vInternalTextures[ 0 ], 0, ARRAY_LENGTH( m_vInternalTextures ) );
}

TextureManager::~TextureManager()
{

}


//////////////////////////////////////////////////////////////////////////

TextureInformationRegistry::~TextureInformationRegistry() {}

const STextureInfo* TextureInformationRegistry::GetInfoForTexture( GLuint uTex )
{
	MapIter iter = m_mapTextureInfo.find( uTex );
	
	return iter != m_mapTextureInfo.end() ? &iter->second : NULL;
}

void TextureInformationRegistry::AddTextureInfo( GLuint uTex, const STextureInfo& sTextureInfo )
{
	m_mapTextureInfo[ uTex ] = sTextureInfo;
}


void TextureInformationRegistry::DeleteTexture( GLuint uTex )
{
	MapIter iter = m_mapTextureInfo.find( uTex );

	if( iter == m_mapTextureInfo.end() )
		return;

	m_mapTextureInfo.erase( iter );
}
