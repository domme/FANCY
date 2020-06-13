#pragma once

#include "FancyCoreDefines.h"
#include "DynamicArray.h"
#include "VertexInputLayoutProperties.h"

namespace Fancy
{
  struct MeshData
  {
    DynamicArray<uint8> myVertexData;
    DynamicArray<uint8> myIndexData;
    VertexInputLayoutProperties myVertexLayout;
  };
}