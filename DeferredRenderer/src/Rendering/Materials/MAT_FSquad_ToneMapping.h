#ifndef MAT_FSQUAD_TONEMAPPING
#define MAT_FSQUAD_TONEMAPPING

#include <Rendering/Materials/Material.h>

class  MAT_FSquad_ToneMapping : public Material
{
public:
	MAT_FSquad_ToneMapping();
	MAT_FSquad_ToneMapping( MAT_FSquad_ToneMapping& other );
	virtual ~MAT_FSquad_ToneMapping();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_FSquad_ToneMapping( *this ); }
	
	void SetInputTextureLoc( GLuint uTex ) { m_uInputTexture = uTex; }
	void SetAvgLuminanceTextureLoc( GLuint uTex ) { m_uAvgLuminanceTexture = uTex; }

protected:
	virtual bool validate() const { return m_uInputTexture != GLUINT_HANDLE_INVALID && m_uAvgLuminanceTexture != GLUINT_HANDLE_INVALID; }

private:
	GLuint m_uInputTexture;
	GLuint m_uAvgLuminanceTexture;
};

#endif