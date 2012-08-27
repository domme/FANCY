#ifndef MAT_TEXTURED_H
#define MAT_TEXTURED_H

#include <Includes.h>
#include <Rendering/Materials/Material.h>
#include <Rendering/GLTexture.h>

class  MAT_Textured : public Material
{
public:
	MAT_Textured();
	MAT_Textured( MAT_Textured& other );
	virtual ~MAT_Textured();

	GLTexture& GetDiffuseTexture() { return m_clDiffuseTexture; }

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual glm::vec3 GetTextureSizeAtIndex( uint uIdx ) const { return m_clDiffuseTexture.GetTextureSize(); }
	virtual Material* Clone() { return new MAT_Textured( *this ); }
	
protected:
	virtual bool validate() const { return m_clDiffuseTexture.getGlLocation() != GLUINT_HANDLE_INVALID; }

	GLTexture m_clDiffuseTexture;
};


#endif