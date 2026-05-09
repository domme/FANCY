#pragma once

#include "Common/FancyCoreDefines.h"

namespace Fancy {
  class GpuMemoryViewer {
  public:
    GpuMemoryViewer();
    void Render();

  private:
    float myScale;
  };
}  // namespace Fancy
