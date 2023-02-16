#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/MathIncludes.h"
#include "Rendering/TextureData.h"
#include "Rendering/TextureProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ImageData
  {
    TextureProperties myProperties;
    TextureData myData;
  };
//---------------------------------------------------------------------------//
  struct ImageLoader
  {
    static bool Load(const char* aPathAbs, uint someLoadFlags, ImageData& anImageOut);
  };
//---------------------------------------------------------------------------//
}
