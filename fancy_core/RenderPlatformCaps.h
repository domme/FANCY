#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy
{
  struct RenderPlatformCaps
  {
    RenderPlatformCaps()
      : myMaxNumVertexAttributes(32)
      , myCbufferPlacementAlignment(0)
    {}

    uint myMaxNumVertexAttributes;
    uint myCbufferPlacementAlignment;
  };
}
