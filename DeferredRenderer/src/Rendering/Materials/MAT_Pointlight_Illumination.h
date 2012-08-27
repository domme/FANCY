#ifndef MAT_POINTLIGHT_ILLUMINATION_H
#define MAT_POINTLIGHT_ILLUMINATION_H

#include <Rendering/Materials/Material.h>

class  MAT_Pointlight_Illumination : public Material
{
public:
	MAT_Pointlight_Illumination();
	MAT_Pointlight_Illumination( MAT_Pointlight_Illumination& other );
	virtual ~MAT_Pointlight_Illumination();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex(  uint uIdx ) const;
	virtual Material* Clone() { return new MAT_Pointlight_Illumination( *this ); }

protected:
	virtual bool validate() const { return true; }
};

#endif