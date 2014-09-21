#ifndef INCLUDE_TEXTURESAMPLER_H
#define INCLUDE_TEXTURESAMPLER_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_TEXTURESAMPLER

namespace Fancy { namespace Core { namespace Rendering {

  class TextureSampler : public PLATFORM_DEPENDENT_NAME(TextureSampler)
  {

  };

} } } // end of namespace Fancy::Core::Rendering

#endif  // INCLUDE_TEXTURESAMPLER_H