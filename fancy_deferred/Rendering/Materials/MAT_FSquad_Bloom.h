#ifndef MAT_FSQUAD_BLOOM
#define MAT_FSQUAD_BLOOM

#include <Rendering/Materials/Material.h>

class DLLEXPORT  MAT_FSquad_Bloom : public Material
{
public:
	MAT_FSquad_Bloom();
	MAT_FSquad_Bloom( MAT_FSquad_Bloom& other );
	virtual ~MAT_FSquad_Bloom();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_FSquad_Bloom( *this ); }
	
	void SetInputTexture( GLuint uTex ) { m_uInputTexture = uTex; }
	void SetBloomTexture( GLuint uTex ) { m_uBloomTexture = uTex; }

protected:
	virtual bool validate()  const { return m_uInputTexture != GLUINT_HANDLE_INVALID && m_uBloomTexture != GLUINT_HANDLE_INVALID; }
	
private:
	GLuint m_uInputTexture;
	GLuint m_uBloomTexture;

};

#endif