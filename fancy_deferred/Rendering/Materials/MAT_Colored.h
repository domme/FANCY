#ifndef MAT_COLORED_H
#define MAT_COLORED_H

#include <Rendering/Materials/Material.h>

class DLLEXPORT  MAT_Colored : public Material
{
public:
	MAT_Colored();
	MAT_Colored( MAT_Colored& other );
	virtual ~MAT_Colored();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex( uint uIdx ) const;
	virtual Material* Clone() { return new MAT_Colored( *this ); }
protected:
	virtual bool validate()  const { return true; }

};

#endif