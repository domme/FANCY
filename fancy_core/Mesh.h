#pragma once

#include "FancyCorePrerequisites.h"
#include "MeshDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GeometryData;
//---------------------------------------------------------------------------//
  class Mesh
  {
  public:
    Mesh();

    const MeshDesc& GetDescription() const { return myDesc; }

    MeshDesc myDesc;
    uint64 myVertexAndIndexHash;
    DynamicArray<SharedPtr<GeometryData>> myGeometryDatas;
  };
  //---------------------------------------------------------------------------//
}