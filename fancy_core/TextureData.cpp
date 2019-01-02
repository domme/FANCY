#include "fancy_core_precompile.h"
#include "TextureData.h"
#include "TextureProperties.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  TextureRegion TextureRegion::ourMaxRegion;
//---------------------------------------------------------------------------//
  TextureSubData::TextureSubData(const TextureProperties& someProperties)
    : myData(nullptr)
  {
    const DataFormatInfo info(someProperties.eFormat);
    myPixelSizeBytes = info.mySizeBytes;

    const uint width = someProperties.myWidth;
    const uint height = glm::max(1u, someProperties.myHeight);
    const uint depth = glm::max(1u, someProperties.myDepthOrArraySize);

    myTotalSizeBytes = myPixelSizeBytes * width * height * depth;
    mySliceSizeBytes = myPixelSizeBytes * width * height;
    myRowSizeBytes = myPixelSizeBytes * width;
  }
//---------------------------------------------------------------------------//
}
