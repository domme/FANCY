#ifndef MAT_FSQUAD_POINTLIGHTING
#define MAT_FSQUAD_POINTLIGHTING

#include <Rendering/Materials/Material.h>

class MAT_FSquad_PointLighting : public Material
{
public:
	MAT_FSquad_PointLighting();
	MAT_FSquad_PointLighting( MAT_FSquad_PointLighting& other );
	virtual ~MAT_FSquad_PointLighting();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_FSquad_PointLighting( *this ); }

	void SetColorGlossTex( GLuint uTex ) { m_uColorGlossTex = uTex; }
	void SetNormalTex( GLuint uTex ) { m_uNormalTex = uTex; }
	void SetDepthTex( GLuint uTex ) { m_uDepthTex = uTex; }
	void SetSpecTex( GLuint uTex ) { m_uSpecTex = uTex; } 
	void SetShadowCubeTex( GLuint uTex ) { m_uCubeShadowTex = uTex; }

protected:
	virtual bool validate() const { return	m_uColorGlossTex	!= GLUINT_HANDLE_INVALID &&
										m_uDepthTex			!= GLUINT_HANDLE_INVALID &&
										m_uNormalTex		!= GLUINT_HANDLE_INVALID &&
										m_uSpecTex			!= GLUINT_HANDLE_INVALID; }

	GLuint m_uColorGlossTex;
	GLuint m_uNormalTex;
	GLuint m_uDepthTex;
	GLuint m_uSpecTex;
	GLuint m_uCubeShadowTex;
};


#endif