#ifndef INCLUDE_TEXTURE_H
#define INCLUDE_TEXTURE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "Serializable.h"

namespace Fancy { namespace Rendering {

class Texture
{
public:
  SERIALIZABLE_RESOURCE(Texture)
};

} } // end of namespace Fancy::Rendering


#endif  // INCLUDE_TEXTURE_H