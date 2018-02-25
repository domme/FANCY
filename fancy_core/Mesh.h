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

    MeshDesc GetDescription() const;
    bool operator==(const Mesh& anOther) const;
    bool operator==(const MeshDesc& aDesc) const;
  //---------------------------------------------------------------------------//
    uint64 GetGeometryHash() const { return myVertexAndIndexHash; }
  //---------------------------------------------------------------------------//
    uint getNumGeometryDatas() const {return m_vGeometries.size();}
    GeometryData* getGeometryData(uint u32Index) {ASSERT(u32Index < m_vGeometries.size()); return m_vGeometries[u32Index];}
    const GeometryData* getGeometryData(uint u32Index) const {ASSERT(u32Index < m_vGeometries.size()); return m_vGeometries[u32Index];}
  //---------------------------------------------------------------------------//
    void addGeometryData(GeometryData* _pGeometryData) {m_vGeometries.push_back(_pGeometryData);}
  //---------------------------------------------------------------------------//
    const DynamicArray<GeometryData*>& getGeometryDataList() const {return m_vGeometries;}
    void setGeometryDataList(const DynamicArray<GeometryData*>& _vGeometries) {m_vGeometries = _vGeometries;}
  //---------------------------------------------------------------------------//
    uint64 GetVertexIndexHash() const { return myVertexAndIndexHash; }
    void SetVertexIndexHash(uint64 aHash) { myVertexAndIndexHash = aHash; }

  private:
    std::vector<GeometryData*> m_vGeometries;
    uint64 myVertexAndIndexHash;
  };
  //---------------------------------------------------------------------------//
}