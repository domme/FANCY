#ifndef MAT_FSQUAD_DIRLIGHTING
#define MAT_FSQUAD_DIRLIGHTING

#include <Rendering/Materials/Material.h>

class MAT_FSquad_DirLighting : public Material
{
public:
	MAT_FSquad_DirLighting();
	MAT_FSquad_DirLighting( MAT_FSquad_DirLighting& other );
	virtual ~MAT_FSquad_DirLighting();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_FSquad_DirLighting( *this ); }

	void SetColorGlossTex( GLuint uTex ) { m_uColorGlossTex = uTex; }
	void SetNormalTex( GLuint uTex ) { m_uNormalTex = uTex; }
	void SetDepthTex( GLuint uTex ) { m_uDepthTex = uTex; }
	void SetSpecTex( GLuint uTex ) { m_uSpecTex = uTex; } 

protected:
	virtual bool validate() const { return	m_uColorGlossTex	!= GLUINT_HANDLE_INVALID &&
										m_uDepthTex			!= GLUINT_HANDLE_INVALID &&
										m_uNormalTex		!= GLUINT_HANDLE_INVALID &&
										m_uSpecTex			!= GLUINT_HANDLE_INVALID; }

	GLuint m_uColorGlossTex;
	GLuint m_uNormalTex;
	GLuint m_uDepthTex;
	GLuint m_uSpecTex;
};


#endif