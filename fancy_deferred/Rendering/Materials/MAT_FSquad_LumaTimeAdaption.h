#ifndef MAT_FSQUAD_LUMATIMEADAPTION
#define MAT_FSQUAD_LUMATIMEADAPTION

#include <Rendering/Materials/Material.h>

class DLLEXPORT  MAT_FSquad_LumaTimeAdaption : public Material
{
public:
	MAT_FSquad_LumaTimeAdaption();
	MAT_FSquad_LumaTimeAdaption( MAT_FSquad_LumaTimeAdaption& other );
	virtual ~MAT_FSquad_LumaTimeAdaption();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_FSquad_LumaTimeAdaption( *this ); }
	
	void SetLumaTextureLocs( GLuint uTexCurrFrame, GLuint uTexLastFrame ) { m_uCurrFrameLumaTex = uTexCurrFrame; m_uLastFrameLumaTex = uTexLastFrame; }

protected:
	virtual bool validate() const { return	m_uCurrFrameLumaTex != GLUINT_HANDLE_INVALID &&
										m_uLastFrameLumaTex != GLUINT_HANDLE_INVALID; }
	
private:
	GLuint m_uCurrFrameLumaTex;
	GLuint m_uLastFrameLumaTex;
};

#endif