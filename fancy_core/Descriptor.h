#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class DescriptorType
  {
    DEFAULT_READ = 0,
    DEFAULT_READ_DEPTH,
    DEFAULT_READ_STENCIL,
    READ_WRITE,
    RENDER_TARGET,
    DEPTH_STENCIL,
    DEPTH_STENCIL_READONLY,
    CONSTANT_BUFFER
  };
//---------------------------------------------------------------------------//
  class Descriptor
  {
    
  };
//---------------------------------------------------------------------------//
}
