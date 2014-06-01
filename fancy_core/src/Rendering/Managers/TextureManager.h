#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include "../../includes.h"
#include "../TextureSemantics.h"
//#include "OpenGL_Includes.h"

class DLLEXPORT TextureManager
{
	public:
		static TextureManager& getInstance();
		~TextureManager();

		uint32 LookupTexture( TextureSemantics::eTexSemantic eSemantic ) { return m_vInternalTextures[ eSemantic ]; }
		void DeclareTexture( TextureSemantics::eTexSemantic eSemantic, uint32 uTex ) { m_vInternalTextures[ eSemantic ] = uTex; }

	private:
		TextureManager();
		uint32 m_vInternalTextures[ TextureSemantics::NUM ]; 

};


/////////////////////////////////////////////////////////////////////////////

struct STextureInfo
{
	STextureInfo( uint uWidth, uint uHeight, uint uDepth ) { m_uWidth = uWidth; m_uHeight = uHeight; m_uDepth = uDepth; }
	STextureInfo() : m_uWidth( 0 ), m_uHeight( 0 ), m_uDepth( 0 ) {}

	uint m_uWidth;
	uint m_uHeight;
	uint m_uDepth;
};

class TextureInformationRegistry
{
public:
	static TextureInformationRegistry& GetInstance() { static TextureInformationRegistry instance; return instance; }
	~TextureInformationRegistry();

	const STextureInfo* GetInfoForTexture( uint32 uTex );
	void AddTextureInfo( uint32 uTex, const STextureInfo& sTextureInfo );
	void DeleteTexture( uint32 uTex );


private:
	TextureInformationRegistry() {}
	std::map<uint32, STextureInfo> m_mapTextureInfo;

	typedef std::map<uint32, STextureInfo>::iterator MapIter;
};

#endif