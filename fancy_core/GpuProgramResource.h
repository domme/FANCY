#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class GpuResourceAccessType {
    READ_ONLY = 0,
    READ_WRITE
  };
//---------------------------------------------------------------------------//
  enum class GpuResourceType {
    NONE = 0,
    TEXTURE_1D,
    TEXTURE_2D,
    TEXTURE_3D,
    TEXTURE_CUBE,
    TEXTURE_1D_SHADOW,
    TEXTURE_2D_SHADOW,
    TEXTURE_CUBE_SHADOW,
    BUFFER_TEXTURE,
    BUFFER,
    SAMPLER,

    NUM
  };
//---------------------------------------------------------------------------//
  struct ConstantBufferElement
  {
    uint64 GetHash() const { return 0u; }

    ConstantBufferElement() :
      uOffsetBytes(0u), uSizeBytes(0u), eFormat(DataFormat::NONE), uFormatComponentCount(1u) {}

    String name;
    uint uOffsetBytes;  // Byte-offset from the start of the buffer
    uint uSizeBytes;  // Overall size of the element (==sizeof(eFormat) * uFormatComponentCount)
    DataFormat eFormat;
    uint8 uFormatComponentCount;  // Multiplier for eFormat. Used for multi-component elements (e.g. Matrices)
  };
//---------------------------------------------------------------------------//
  /// Describes a resource (texture, buffer, ...) used in a gpuProgram as returned from reflection
  struct GpuProgramResourceInfo {
    GpuProgramResourceInfo()
      : u32RegisterIndex(0u),
        eAccessType(GpuResourceAccessType::READ_ONLY), 
        eResourceType(GpuResourceType::NONE) {}

    uint u32RegisterIndex;
    String name;
    GpuResourceAccessType eAccessType;
    GpuResourceType eResourceType;
  };
//---------------------------------------------------------------------------//
}