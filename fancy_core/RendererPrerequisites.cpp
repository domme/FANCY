#include "RendererPrerequisites.h"
#include "Fancy.h"
#include "RenderCore.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  uint MultiBuffering::getCurrentBufferIndex()
  {
    return Fancy::FancyRuntime::GetInstance()->GetCurrentFrameIndex() % MultiBuffering::kGpuMultiBufferingCount;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
TextureSubData::TextureSubData(const TextureParams& someParams)
  : myData(nullptr)
{
  DataFormat actualFormat = RenderCore::ResolveFormat(someParams.eFormat);
  DataFormatInfo info(actualFormat);
  myPixelSizeBytes = info.mySizeBytes;

  uint width = someParams.myWidth;
  uint height = glm::max((uint16)1, someParams.myHeight);
  uint depth = glm::max((uint16)1, someParams.myDepthOrArraySize);

  myTotalSizeBytes = myPixelSizeBytes * width * height * depth;
  mySliceSizeBytes = myPixelSizeBytes * width * height;
  myRowSizeBytes = myPixelSizeBytes * width;
}
//---------------------------------------------------------------------------//
}
