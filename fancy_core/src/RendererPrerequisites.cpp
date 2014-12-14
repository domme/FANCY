#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  namespace MultiBuffering 
  {
    uint32 uCurrentBufferIndex = 0u;
  }
//---------------------------------------------------------------------------//
  uint32 MultiBuffering::getCurrentBufferIndex()
  {
    return MultiBuffering::uCurrentBufferIndex;
  }
//---------------------------------------------------------------------------//
  void MultiBuffering::beginFrame()
  {
    ++MultiBuffering::uCurrentBufferIndex;

    if (MultiBuffering::uCurrentBufferIndex >= MultiBuffering::kGpuMultiBufferingCount)
    {
      MultiBuffering::uCurrentBufferIndex = 0;
    }
  }  
//---------------------------------------------------------------------------//
 } }  // end of namespace Fancy::Rendering
