#ifndef MAT_FSQUAD_BRIGHTPASS
#define MAT_FSQUAD_BRIGHTPASS

#include "Material.h"

class DLLEXPORT MAT_FSquad_BrightPass : public Material
{
public:
	MAT_FSquad_BrightPass();
	MAT_FSquad_BrightPass( MAT_FSquad_BrightPass& other );
	virtual ~MAT_FSquad_BrightPass();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_FSquad_BrightPass( *this ); }
	
	void SetInputTexture( GLuint uTex ) { m_uInputTexture = uTex; }

protected:
	virtual bool validate() const { return m_uInputTexture != GLUINT_HANDLE_INVALID; }
	
private:
	GLuint m_uInputTexture;

};

#endif