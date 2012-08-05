#ifndef MAT_VOLCUBE_RASTERIZE
#define MAT_VOLCUBE_RASTERIZE

#include "Material.h"

class MAT_VolCube_Rasterize : public Material
{
public:
	MAT_VolCube_Rasterize();
	MAT_VolCube_Rasterize( MAT_VolCube_Rasterize& other );
	virtual ~MAT_VolCube_Rasterize();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual void PrepareMaterialRendering( Mesh* pMesh, const glm::mat4& rObjectTransformMAT );
	virtual Material* Clone() { return new MAT_VolCube_Rasterize( *this ); }

protected:
	virtual bool validate() const { return true; }


};


#endif