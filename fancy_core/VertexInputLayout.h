#pragma once

#include "FancyCoreDefines.h"
#include "DataFormat.h"
#include "RenderEnums.h"
#include "DynamicArray.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderVertexInputElement 
  {
    uint64 GetHash() const;

    String myName;
    VertexSemantics mySemantics = VertexSemantics::NONE;
    uint mySemanticIndex = 0u;
    uint mySizeBytes = 0u;
    DataFormat myFormat = DataFormat::NONE;
    uint8 myFormatComponentCount = 1u;
  };
//---------------------------------------------------------------------------//
  class ShaderVertexInputLayout
  {
  public:
    static ShaderVertexInputLayout ourDefaultModelLayout;

    DynamicArray<ShaderVertexInputElement> myVertexInputElements;
  };
//---------------------------------------------------------------------------//
}
