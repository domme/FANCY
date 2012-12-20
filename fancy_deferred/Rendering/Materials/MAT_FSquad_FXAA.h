#ifndef MAT_FSQUAD_FXAA
#define MAT_FSQUAD_FXAA

#include <Rendering/Materials/Material.h>

class DLLEXPORT  MAT_FSquad_FXAA : public Material
{
public:
	MAT_FSquad_FXAA();
	MAT_FSquad_FXAA( MAT_FSquad_FXAA& other );
	virtual ~MAT_FSquad_FXAA();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_FSquad_FXAA( *this ); }

	void SetInputTexture( GLuint uTex ) { m_uInputTexture = uTex; }

protected:
	virtual bool validate() const { return m_uInputTexture != GLUINT_HANDLE_INVALID; }

private:
	GLuint m_uInputTexture;
};

#endif