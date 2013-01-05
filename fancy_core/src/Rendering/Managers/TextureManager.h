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

		GLuint LookupTexture( TextureSemantics::eTexSemantic eSemantic ) { return m_vInternalTextures[ eSemantic ]; }
		void DeclareTexture( TextureSemantics::eTexSemantic eSemantic, GLuint uTex ) { m_vInternalTextures[ eSemantic ] = uTex; }

	private:
		TextureManager();
		GLuint m_vInternalTextures[ TextureSemantics::NUM ]; 

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

	const STextureInfo* GetInfoForTexture( GLuint uTex );
	void AddTextureInfo( GLuint uTex, const STextureInfo& sTextureInfo );
	void DeleteTexture( GLuint uTex );


private:
	TextureInformationRegistry() {}
	std::map<GLuint, STextureInfo> m_mapTextureInfo;

	typedef std::map<GLuint, STextureInfo>::iterator MapIter;
};

#endif