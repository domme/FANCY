#ifndef MAT_TEXTUREDNORMAL_H
#define MAT_TEXTUREDNORMAL_H

#include <Includes.h>
#include <Rendering/Materials/Material.h>
#include <Rendering/GLTexture.h>

class  DLLEXPORT MAT_TexturedNormal : public Material
{
public:
	MAT_TexturedNormal();
	MAT_TexturedNormal( MAT_TexturedNormal& other );
	virtual ~MAT_TexturedNormal();

	Texture& GetDiffuseTexture() { return m_clDiffuseTexture; }
	Texture& GetNormalTexture() { return m_clNormalTexture; }
	float		GetBumpIntensity() const { return m_fBumpIntensity; }

	void		SetBumpIntensity( float fBumpIntensity ) { m_fBumpIntensity = fBumpIntensity; }

	virtual void UpdateUniform( IUniform* pUniform ) const;

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual glm::vec3 GetTextureSizeAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_TexturedNormal( *this ); }
	

protected:
	virtual bool validate() const { return m_clDiffuseTexture.getGlLocation() != GLUINT_HANDLE_INVALID &&
									 m_clNormalTexture.getGlLocation() != GLUINT_HANDLE_INVALID; }

	Texture	m_clDiffuseTexture;
	Texture	m_clNormalTexture;

	float		m_fBumpIntensity;
};


#endif