#ifndef MAT_FSQUAD_TEXTURED
#define MAT_FSQUAD_TEXTURED

#include <Rendering/Materials/Material.h>

class DLLEXPORT MAT_FSquad_Textured : public Material
{
public:
	MAT_FSquad_Textured();
	MAT_FSquad_Textured( MAT_FSquad_Textured& other );
	virtual ~MAT_FSquad_Textured();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual void PrepareMaterialRendering( Mesh* pMesh, const glm::mat4& rObjectTransformMAT );
	virtual Material* Clone() { return new MAT_FSquad_Textured( *this ); }

	void SetTexture( GLuint uTex ) { m_uTexture = uTex; }
	
protected:
	virtual bool validate() const { return m_uTexture != GLUINT_HANDLE_INVALID; }

	GLuint m_uTexture;


};


#endif