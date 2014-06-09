#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include "../includes.h"
//#include "OpenGL_Includes.h"
#include "../IO/PathService.h"

class DLLEXPORT Texture
{
	public:
		Texture();
		Texture( Texture& other );
		virtual ~Texture();

		uint32 getGlLocation() const { return m_uGlTextureLocation; }

		void SetTexture( uint32 uGLtexLoc );
		bool SetTexture( const String& szRelativeTexturePath );
		bool SetTexture1D( const String& szRelativeTexturePath );
		bool HasTexture() const { return m_bInitialized; }

		const glm::vec3& GetTextureSize() const { return m_v3TextureSize; }

	protected:
		uint32			m_uGlTextureLocation;
		bool			  m_bInitialized;
		glm::vec3	  m_v3TextureSize;
};


#endif