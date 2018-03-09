#pragma once

#include "FancyCorePrerequisites.h"
#include "GeometryVertexLayout.h"

namespace Fancy
{
  struct MeshData
  {
    DynamicArray<uint8> myVertexData;
    DynamicArray<uint8> myIndexData;
    GeometryVertexLayout myLayout;
  };
}


