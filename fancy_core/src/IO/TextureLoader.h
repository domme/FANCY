#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include "../Includes.h"
#include "../Rendering/Managers/TextureManager.h"


class DLLEXPORT TextureLoader
{
public:
	
	

	~TextureLoader();

	static TextureLoader& GetInstance() { static TextureLoader instance; return instance; }

	GLuint LoadTexture1D( const String& szPath, bool* pbSuccess = NULL, STextureInfo* pTextureInfo = NULL );
	GLuint LoadTexture2D( const String& szPath, bool* pbSuccess = NULL, STextureInfo* pTextureInfo = NULL );
	GLuint LoadTexture3D( const String& szBasePath, uint uStartIndex, uint uEndIndex, const String& szFileType, bool* pbSuccess = NULL, STextureInfo* pTextureInfo = NULL );
	
	GLuint CreateVolumeTransferFunction1D( const std::vector<float>& vIntensities, float fMaxIntensity, const std::vector<glm::vec4>& vColors );


protected:
	bool lookupInRegistry( const String& szPath, STextureInfo* pTextureInfo, GLuint& ruTex );

	TextureLoader();
	bool loadPFMfile( const String& szAbsPath, uint* pWidth, uint* pHeight, float** ppImageData );
	bool loadPFMfile_Grayscale( const String& szAbsPath, uint* pWidth, uint* pHeight, float** ppImageData );
	bool loadPFMfile_Grayscale16( const String& szAbsPath, uint* pWidth, uint* pHeight, glm::float16** ppImageData );

};

#endif