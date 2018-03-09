#pragma once

#include "FancyCorePrerequisites.h"
#include "MeshDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GeometryData;
//---------------------------------------------------------------------------//
  /// Represents a collection of raw geometric pieces that can be rendered with a single material
  /// Two GeometryDatas always have different vertex-attributes or primitive types which makes their distinction necessary.
  class Mesh
  {
  public:
    Mesh();
    ~Mesh();

    uint64 myVertexAndIndexHash;
    DynamicArray<GeometryData*> m_vGeometries;
  };
  //---------------------------------------------------------------------------//
}