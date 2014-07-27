#ifndef INCLUDE_TEXTURE_H
#define INCLUDE_TEXTURE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_TEXTURE

namespace FANCY { namespace Core { namespace Rendering {

class Texture : public PLATFORM_DEPENDENT_NAME(Texture) 
{

};

} } } // end of namespace FANCY::Core::Rendering


#endif  // INCLUDE_TEXTURE_H