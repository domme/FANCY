#ifndef MAT_FSQUAD_FINALCOMPOSITE
#define MAT_FSQUAD_FINALCOMPOSITE

#include "Material.h"

class MAT_FSquad_FinalComposite : public Material
{
public:
	MAT_FSquad_FinalComposite();
	MAT_FSquad_FinalComposite( MAT_FSquad_FinalComposite& other );
	virtual ~MAT_FSquad_FinalComposite();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_FSquad_FinalComposite( *this ); }

	void SetColorGlossTex( GLuint uTex ) { m_uColorGlossTex = uTex; }
	void SetLocalIllumTex( GLuint uTex ) { m_uLocalIllumTexture = uTex; }

protected:
	virtual bool validate() const { return	m_uColorGlossTex	!= GLUINT_HANDLE_INVALID &&
										m_uLocalIllumTexture != GLUINT_HANDLE_INVALID; }

	GLuint m_uColorGlossTex;
	GLuint m_uLocalIllumTexture;

};


#endif