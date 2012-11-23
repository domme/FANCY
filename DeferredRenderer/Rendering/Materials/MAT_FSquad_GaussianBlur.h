#ifndef MAT_FSQUAD_GAUSSIANBLUR
#define MAT_FSQUAD_GAUSSIANBLUR

#include <Rendering/Materials/Material.h>

class DLLEXPORT  MAT_FSquad_GaussianBlur : public Material
{
public:
	MAT_FSquad_GaussianBlur();
	MAT_FSquad_GaussianBlur( MAT_FSquad_GaussianBlur& other );
	virtual ~MAT_FSquad_GaussianBlur();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_FSquad_GaussianBlur( *this ); }
	virtual void UpdateUniform( IUniform* pUniform ) const;
	
	void SetInputTexture( GLuint uTex ) { m_uInputTexture = uTex; }
	void SetSamplingDirection( ESamplingDirection::eSamplingDirection eSamplingDir ) { m_eSamplingDirection = eSamplingDir; }

	void SetGaussTexture( GLuint uTex ) { m_uGaussTexture = uTex; }
	void SetOffsetTexture( GLuint uTex ) { m_uOffsetTexture = uTex; }

	void SetKernelSize( uint uSize ) { m_uKernelSize = uSize; }

protected:
	virtual bool validate() const { return	m_uGaussTexture		!= GLUINT_HANDLE_INVALID &&
										m_uOffsetTexture	!= GLUINT_HANDLE_INVALID &&
										m_uInputTexture		!= GLUINT_HANDLE_INVALID; }
	
private:
	GLuint										m_uGaussTexture;
	GLuint										m_uOffsetTexture;
	GLuint										m_uInputTexture;
	uint										m_uKernelSize;
	ESamplingDirection::eSamplingDirection		m_eSamplingDirection;

};

#endif