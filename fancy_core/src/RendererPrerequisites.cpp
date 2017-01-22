#include "RendererPrerequisites.h"
#include "Renderer.h"
#include "Fancy.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  uint32 MultiBuffering::getCurrentBufferIndex()
  {
    return Fancy::FancyRuntime::GetInstance()->GetCurrentFrameIndex() % MultiBuffering::kGpuMultiBufferingCount;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
TextureUploadData::TextureUploadData(const TextureParams& someParams)
  : myData(nullptr)
{
  DataFormat actualFormat = RenderCore::ResolveFormat(someParams.eFormat);
  DataFormatInfo info(actualFormat);
  myPixelSizeBytes = info.mySizeBytes;

  uint32 width = someParams.u16Width;
  uint32 height = glm::max((uint16)1, someParams.u16Height);
  uint32 depth = glm::max((uint16)1, someParams.u16Depth);

  myTotalSizeBytes = myPixelSizeBytes * width * height * depth;
  mySliceSizeBytes = myPixelSizeBytes * width * height;
  myRowSizeBytes = myPixelSizeBytes * width;
}
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering
