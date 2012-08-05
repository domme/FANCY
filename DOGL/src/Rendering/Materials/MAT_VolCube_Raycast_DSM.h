#ifndef MAT_VOLCUBE_RAYCAST_DSM
#define MAT_VOLCUBE_RAYCAST_DSM

#include "VolumeMaterial.h"

class MAT_VolCube_Raycast_DSM : public VolumeMaterial
{
public:
	MAT_VolCube_Raycast_DSM();
	MAT_VolCube_Raycast_DSM( MAT_VolCube_Raycast_DSM& other );
	virtual ~MAT_VolCube_Raycast_DSM();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual const glm::vec3& GetTextureSizeAtIndex( uint uIdx ) const;
	virtual void PrepareMaterialRendering( Mesh* pMesh, const glm::mat4& rObjectTransformMAT );
	virtual VolumeMaterial* CloneVolumeMat() { return new MAT_VolCube_Raycast_DSM( *this ); }

	

protected:
	virtual bool validate() const { return m_uVolumeTexture != GLUINT_HANDLE_INVALID; }

	

};


#endif