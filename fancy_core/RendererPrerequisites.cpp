#include "RendererPrerequisites.h"
#include "Fancy.h"
#include "RenderCore.h"

namespace Fancy {
//---------------------------------------------------------------------------//
TextureSubData::TextureSubData(const TextureProperties& someProperties)
  : myData(nullptr)
{
  DataFormat actualFormat = RenderCore::ResolveFormat(someProperties.eFormat);
  DataFormatInfo info(actualFormat);
  myPixelSizeBytes = info.mySizeBytes;

  uint width = someProperties.myWidth;
  uint height = glm::max(1u, someProperties.myHeight);
  uint depth = glm::max(1u, someProperties.myDepthOrArraySize);

  myTotalSizeBytes = myPixelSizeBytes * width * height * depth;
  mySliceSizeBytes = myPixelSizeBytes * width * height;
  myRowSizeBytes = myPixelSizeBytes * width;
}
//---------------------------------------------------------------------------//
}
