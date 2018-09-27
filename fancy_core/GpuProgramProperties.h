#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "VertexInputLayout.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ConstantBufferElement
  {
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
      : myRegisterIndex(0u)
      , myIsUnorderedWrite(false)
      , myDimension(GpuResourceDimension::UNKONWN)
    {}

    String myName;
    uint myRegisterIndex;
    bool myIsUnorderedWrite;
    GpuResourceDimension myDimension;
  };
//---------------------------------------------------------------------------//
  struct GpuProgramProperties
  {
    GpuProgramProperties()
      : myShaderStage(ShaderStage::NONE)
      , myHasUnorderedWrites(false)
    {}

    ShaderVertexInputLayout myVertexInputLayout;
    DynamicArray<GpuProgramResourceInfo> myResourceInfos;
    DynamicArray<ConstantBufferElement> myConstantBufferElements;
    ShaderStage myShaderStage;
    bool myHasUnorderedWrites;
  };
//---------------------------------------------------------------------------//
}