#ifndef PASS_GAUSSIANBLUR
#define PASS_GAUSSIANBLUR

#include <Includes.h>

#include "../Geometry/FullscreenQuad.h"
#include "Materials/MAT_FSquad_GaussianBlur.h"

class MAT_FSquad_GaussianBlur;
class GLRenderer;

class Pass_GaussianBlur
{
	public: 
		~Pass_GaussianBlur();
		static Pass_GaussianBlur& GetInstance() { static Pass_GaussianBlur instance; return instance; }

		void Init( uint uBlurStrength, uint uDestWidth, uint uDestHeight, GLenum eInternalFormat, GLenum eFormat, GLenum eDataType );

		void BlurTextureIntoFBO( GLuint uSrcTexture, GLuint uDestFBO, uint uDestWidth, uint uDestHeight, GLenum eInternalFormat, GLenum eFormat, GLenum eDatatype, GLRenderer* pRenderer, uint uBlurStrength = 3 );

	private:
		Pass_GaussianBlur();

		void	 createTempFBOandTex();
		void	 deleteTempFBOandTex();
		
		void	 createGaussAndOffsetTex();
		void	 deleteGaussAndOffsetTex();
		

		FullscreenQuad*			m_pFSquad;

		MAT_FSquad_GaussianBlur	m_clBlurMat;

		GLuint					m_uTempFBO;
		GLuint					m_uTempTex;

		GLenum					m_eInternalFormat;
		GLenum					m_eFormat;
		GLenum					m_eDataType;

		uint					m_uWidth;
		uint					m_uHeight;

		uint					m_uBlurStrength;

		uint					m_uGaussTex;
		uint					m_uOffsetTex;
};

#endif