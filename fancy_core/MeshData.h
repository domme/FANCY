#pragma once

#include "FancyCoreDefines.h"
#include "GeometryVertexLayout.h"
#include "DynamicArray.h"

namespace Fancy
{
  struct MeshData
  {
    DynamicArray<uint8> myVertexData;
    DynamicArray<uint8> myIndexData;
    GeometryVertexLayout myLayout;
  };
}