#pragma once

#include "FancyCoreDefines.h"
#include "RenderEnums.h"
#include "StaticArray.h"
#include "DataFormat.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct VertexInputAttributeDesc 
  {
    bool operator==(const VertexInputAttributeDesc& anOther) const { return memcmp(this, &anOther, sizeof(VertexInputAttributeDesc)) == 0u; }
    VertexAttributeSemantic mySemantic = VertexAttributeSemantic::NONE;
    DataFormat myFormat = DataFormat::NONE;
    uint mySemanticIndex = 0u;
    uint myBufferIndex = 0u;
    uint myOffsetInBuffer = 0u;
  };
//---------------------------------------------------------------------------//
  struct VertexBufferBindDesc
  {
    bool operator==(const VertexBufferBindDesc& anOther) const { return memcmp(this, &anOther, sizeof(VertexBufferBindDesc)) == 0; }
    uint myStride = 0u;
    VertexInputRate myInputRate = VertexInputRate::PER_VERTEX;
  };
//---------------------------------------------------------------------------//
  struct VertexInputLayoutProperties
  {
    bool operator==(const VertexInputLayoutProperties& anOther) const;
    bool operator !=(const VertexInputLayoutProperties& anOther) const { return !operator==(anOther); }
    uint64 GetHash() const;

    StaticArray<VertexInputAttributeDesc, 16> myAttributes;
    StaticArray<VertexBufferBindDesc, 16> myBufferBindings;
  };
//---------------------------------------------------------------------------//
  struct VertexInputLayout
  {
    VertexInputLayout(const VertexInputLayoutProperties& aDesc) 
    : myDesc(aDesc)
    { }

    VertexInputLayoutProperties myDesc;
  };
//---------------------------------------------------------------------------//
}
