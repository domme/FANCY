#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy
{
  struct RenderPlatformCaps
  {
    RenderPlatformCaps()
      : myMaxNumVertexAttributes(32)
    {}

    uint myMaxNumVertexAttributes;
  };
}
