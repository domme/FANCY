#include "fancy_core_precompile.h"
#include "TextureProperties.h"

namespace Fancy
{
  void TextureProperties::GetSize(uint aMipLevel, uint& aWidthOut, uint& aHeightOut, uint& aDepthOut) const
  {
    ASSERT(aMipLevel < myNumMipLevels);

    aWidthOut = myWidth >> aMipLevel;

    const bool hasHeight = myDimension != GpuResourceDimension::TEXTURE_1D && myDimension != GpuResourceDimension::TEXTURE_1D_ARRAY;
    aHeightOut = hasHeight ? myHeight >> aMipLevel : 1u;

    const bool hasDepth = GetDepthSize() > 1u;
    aDepthOut = hasDepth ? myDepthOrArraySize >> aMipLevel : 1u;
  }
}
