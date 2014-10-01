#ifndef INCLUDE_MATERIAL_H
#define INCLUDE_MATERIAL_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "FixedArray.h"
#include "ObjectName.h"

namespace Fancy { namespace Core { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class EMaterialTechnique
  {
    SOLID_GBUFFER = 0,
    SOLID_FORWARD,
    TRANSPARENT_FORWARD,
    // More to come...
    NUM
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class Material {




  };

//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Core::Rendering

#endif  // INCLUDE_MATERIAL_H