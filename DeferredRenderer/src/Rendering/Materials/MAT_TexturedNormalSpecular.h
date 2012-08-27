#ifndef MAT_TEXTUREDNORMALSPECULAR_H
#define MAT_TEXTUREDNORMALSPECULAR_H

#include <Includes.h>
#include <Rendering/Materials/Material.h>
#include <Rendering/GLTexture.h>

class  MAT_TexturedNormalSpecular : public Material
{
public:
	MAT_TexturedNormalSpecular();
	MAT_TexturedNormalSpecular( MAT_TexturedNormalSpecular& other );
	virtual ~MAT_TexturedNormalSpecular();

	GLTexture&	GetDiffuseTexture() { return m_clDiffuseTexture; }
	GLTexture&	GetNormalTexture() { return m_clNormalTexture; }
	GLTexture&	GetSpecularTexture() { return m_clSpecTexture; }
	GLTexture&	GetGlossTexture() { return m_clGlossTexture; }

	float		GetBumpIntensity() const { return m_fBumpIntensity; }

	void		SetBumpIntensity( float fBumpIntensity ) { m_fBumpIntensity = fBumpIntensity; }

	virtual void UpdateUniform( IUniform* pUniform ) const;

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual glm::vec3 GetTextureSizeAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_TexturedNormalSpecular( *this ); }
	

protected:
	virtual bool validate() const { return m_clDiffuseTexture.getGlLocation() != GLUINT_HANDLE_INVALID &&
									 m_clNormalTexture.getGlLocation() != GLUINT_HANDLE_INVALID &&
									 m_clSpecTexture.getGlLocation() != GLUINT_HANDLE_INVALID && 
									 m_clGlossTexture.getGlLocation() != GLUINT_HANDLE_INVALID; }

	GLTexture	m_clDiffuseTexture;
	GLTexture	m_clNormalTexture;
	GLTexture	m_clSpecTexture;
	GLTexture	m_clGlossTexture;

	float		m_fBumpIntensity;
};


#endif