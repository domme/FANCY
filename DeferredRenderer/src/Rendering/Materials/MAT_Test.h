#ifndef MAT_TEST_H
#define MAT_TEST_H

#include <Rendering/Materials/Material.h>

class  MAT_Test : public Material
{
public:
	MAT_Test();
	MAT_Test( MAT_Test& other );
	virtual ~MAT_Test();

	virtual bool Init();
	virtual GLuint GetTextureAtIndex(  uint uIdx ) const;
	virtual Material* Clone() { return new MAT_Test( *this ); }

protected:
	virtual bool validate() const { return true; }
};

#endif