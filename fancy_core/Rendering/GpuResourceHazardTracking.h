#pragma once

#include "Common/FancyCoreDefines.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  // Note: DX12 uses explicit programmer-driven GlobalBarrier / TextureBarrier calls (see CommandList.h).
  // This struct is kept for Vulkan path parity and potential future use.
  struct GpuResourceHazardTrackingVk {
    // Uints are VkAccessFlags
    uint         myReadAccessMask;
    uint         myWriteAccessMask;
    bool         myHasExclusiveQueueAccess;
    mutable bool myHasInitialImageLayout;  // True if the image has never been used in a resourceBarrier and still has
                                           // its initial layout from creation.
    uint myInitialImageLayout;             // Initial layout given to an image upon creation.
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy
