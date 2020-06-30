#pragma once

#include "FancyCoreDefines.h"
#include "RenderEnums.h"
#include "StaticArray.h"
#include "DataFormat.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct VertexInputAttributeDesc 
  {
    VertexInputAttributeDesc(DataFormat aFormat = DataFormat::NONE, VertexAttributeSemantic aSemantic = VertexAttributeSemantic::NONE, 
      uint aSemanticIndex = 0u, uint aBufferIndex = 0u)
      : myFormat(aFormat)
      , mySemantic(aSemantic)
      , mySemanticIndex(aSemanticIndex)
      , myBufferIndex(aBufferIndex)
    { }

    DataFormat myFormat;
    VertexAttributeSemantic mySemantic;
    uint mySemanticIndex;
    uint myBufferIndex;
  };
//---------------------------------------------------------------------------//
  struct VertexBufferBindDesc
  {
    uint myStride = 0u;
    VertexInputRate myInputRate = VertexInputRate::PER_VERTEX;
  };
//---------------------------------------------------------------------------//
  struct VertexInputLayoutProperties
  {
    uint64 GetHash() const;
    uint64 GetOverallVertexSize() const;

    StaticArray<VertexInputAttributeDesc, 16> myAttributes;
    StaticArray<VertexBufferBindDesc, 16> myBufferBindings;
  };
//---------------------------------------------------------------------------//
  struct VertexInputLayout
  {
    VertexInputLayout(const VertexInputLayoutProperties& someProperties);
    StaticArray<uint, 16> myAttributeOffsetsInBuffer;
    VertexInputLayoutProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
