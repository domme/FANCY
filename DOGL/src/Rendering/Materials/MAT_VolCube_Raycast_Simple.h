#ifndef MAT_VOLCUBE_RAYCAST_SIMPLE
#define MAT_VOLCUBE_RAYCAST_SIMPLE

#include "VolumeMaterial.h"

class MAT_VolCube_Raycast_Simple : public VolumeMaterial
{
public:
	MAT_VolCube_Raycast_Simple();
	MAT_VolCube_Raycast_Simple( MAT_VolCube_Raycast_Simple& other );
	virtual ~MAT_VolCube_Raycast_Simple();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual const glm::vec3& GetTextureSizeAtIndex( uint uIdx ) const;
	virtual void PrepareMaterialRendering( Mesh* pMesh, const glm::mat4& rObjectTransformMAT );
	virtual VolumeMaterial* CloneVolumeMat() { return new MAT_VolCube_Raycast_Simple( *this ); }
	

protected:
	



};


#endif