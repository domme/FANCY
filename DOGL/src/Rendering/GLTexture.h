#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include "../includes.h"
//#include "OpenGL_Includes.h"
#include "../IO/PathService.h"

class DLLEXPORT GLTexture
{
	public:
		GLTexture();
		GLTexture( GLTexture& other );
		virtual ~GLTexture();

		GLuint getGlLocation() const { return m_uGlTextureLocation; }

		void SetTexture( GLuint uGLtexLoc );
		bool SetTexture( const String& szRelativeTexturePath );
		bool SetTexture1D( const String& szRelativeTexturePath );
		bool SetTexture3D( const String& szRelativeTextureBasePath, uint uStartIndex, uint uEndIndex, const String& szExtension );
		bool HasTexture() const { return m_bInitialized; }

		const glm::vec3& GetTextureSize() const { return m_v3TextureSize; }

	protected:
		GLuint			m_uGlTextureLocation;
		bool			m_bInitialized;
		glm::vec3	  	m_v3TextureSize;
};


#endif