#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include "../Includes.h"
#include "../Rendering/Managers/TextureManager.h"


class DLLEXPORT TextureLoader
{
public:
	~TextureLoader();

	static TextureLoader& GetInstance() { static TextureLoader instance; return instance; }

	uint32 LoadTexture1D( const String& szPath, bool* pbSuccess = NULL, STextureInfo* pTextureInfo = NULL );
	uint32 LoadTexture2D( const String& szPath, bool* pbSuccess = NULL, STextureInfo* pTextureInfo = NULL );

protected:
	bool lookupInRegistry( const String& szPath, STextureInfo* pTextureInfo, uint32& ruTex );

	TextureLoader();
};

#endif