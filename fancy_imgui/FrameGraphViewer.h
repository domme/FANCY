#pragma once

#include "Common/FancyCoreDefines.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  class FrameGraphBuilder;
  //---------------------------------------------------------------------------//
  /// Draws a built FrameGraphBuilder as a node graph. Hovering a task lists its resource accesses.
  class FrameGraphViewer {
  public:
    FrameGraphViewer() = default;

    void Render( const FrameGraphBuilder & aFrameGraph, bool * anIsOpen = nullptr );
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy
