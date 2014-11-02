#ifndef INCLUDE_MESH_H
#define INCLUDE_MESH_H

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"

namespace Fancy { namespace Geometry { 
//---------------------------------------------------------------------------//
  class GeometryData;
//---------------------------------------------------------------------------//
  const uint32 kMaxNumGeometriesPerSubModel = 128;
//---------------------------------------------------------------------------//
  typedef FixedArray<GeometryData*, kMaxNumGeometriesPerSubModel> GeometryDataList;
//---------------------------------------------------------------------------//
  /// Represents a collection of raw geometric pieces that can be rendered with a single material
  /// Two GeometryDatas always have different vertex-attributes or primitive types which makes their distinction necessary.
  class Mesh 
  {
  public:
    Mesh();
    ~Mesh();
  //---------------------------------------------------------------------------//
    const ObjectName& getName() {return m_Name;}
    void setName(const ObjectName& clNewName) {m_Name = clNewName;}
  //---------------------------------------------------------------------------//
    uint32 getNumGeometryDatas() const {return m_vGeometries.size();}
    GeometryData* getGeometryData(uint32 u32Index) {ASSERT(u32Index < m_vGeometries.size()); return m_vGeometries[u32Index];}
    const GeometryData* getGeometryData(uint32 u32Index) const {ASSERT(u32Index < m_vGeometries.size()); return m_vGeometries[u32Index];}
  //---------------------------------------------------------------------------//
    //SubMeshList& getGeometryDataList() {return m_vGeometries;}
    const GeometryDataList& getGeometryDataList() const {return m_vGeometries;} 
  //---------------------------------------------------------------------------//
  private:
    GeometryDataList m_vGeometries;
    ObjectName m_Name;
  };
  //---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

#endif  // INCLUDE_MESH_H