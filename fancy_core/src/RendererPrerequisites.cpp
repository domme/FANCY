#include "RendererPrerequisites.h"
#include "TimeManager.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  uint32 MultiBuffering::getCurrentBufferIndex()
  {
    return Time::getCurrentFrameIndex() % MultiBuffering::kGpuMultiBufferingCount;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
 } }  // end of namespace Fancy::Rendering
