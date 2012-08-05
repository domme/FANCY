#ifndef MAT_FSQUAD_TEXTURED3D
#define MAT_FSQUAD_TEXTURED3D

#include "VolumeMaterial.h"

class MAT_FSquad_Textured3D : public VolumeMaterial
{
public:
	MAT_FSquad_Textured3D();
	MAT_FSquad_Textured3D( MAT_FSquad_Textured3D& other );
	virtual ~MAT_FSquad_Textured3D();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual void PrepareMaterialRendering( Mesh* pMesh, const glm::mat4& rObjectTransformMAT );
	virtual VolumeMaterial* CloneVolumeMat() { return new MAT_FSquad_Textured3D( *this ); }
		
protected:
	virtual bool validate() const { return m_uVolumeTexture != GLUINT_HANDLE_INVALID; }

};


#endif