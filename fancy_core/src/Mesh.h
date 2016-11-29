#ifndef INCLUDE_MESH_H
#define INCLUDE_MESH_H

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"
#include "StaticManagedObject.h"
#include "Serializable.h"
#include "MeshDesc.h"

namespace Fancy { namespace Geometry { 
//---------------------------------------------------------------------------//
  class GeometryData;
//---------------------------------------------------------------------------//
  typedef FixedArray<GeometryData*, Rendering::Constants::kMaxNumGeometriesPerSubModel> GeometryDataList;
//---------------------------------------------------------------------------//
  /// Represents a collection of raw geometric pieces that can be rendered with a single material
  /// Two GeometryDatas always have different vertex-attributes or primitive types which makes their distinction necessary.
  class Mesh : public StaticManagedHeapObject<Mesh>
  {
  public:
    SERIALIZABLE(Mesh)

    Mesh();
    ~Mesh();

    MeshDesc GetDescription() const;
    bool operator==(const Mesh& anOther) const;
    bool operator==(const MeshDesc& aDesc) const;
  //---------------------------------------------------------------------------//
    uint64 GetHash() const { return GetDescription().GetHash(); }
  //---------------------------------------------------------------------------//
    static ObjectName getTypeName() { return _N(Mesh); }
    void Serialize(IO::Serializer* aSerializer);

    uint32 getNumGeometryDatas() const {return m_vGeometries.size();}
    GeometryData* getGeometryData(uint32 u32Index) {ASSERT(u32Index < m_vGeometries.size()); return m_vGeometries[u32Index];}
    const GeometryData* getGeometryData(uint32 u32Index) const {ASSERT(u32Index < m_vGeometries.size()); return m_vGeometries[u32Index];}
  //---------------------------------------------------------------------------//
    void addGeometryData(GeometryData* _pGeometryData) {m_vGeometries.push_back(_pGeometryData);}
  //---------------------------------------------------------------------------//
    //SubMeshList& getGeometryDataList() {return m_vGeometries;}
    const GeometryDataList& getGeometryDataList() const {return m_vGeometries;}
    void setGeometryDataList(const GeometryDataList& _vGeometries) {m_vGeometries = _vGeometries;}
  //---------------------------------------------------------------------------//
    uint64 GetVertexIndexHash() const { return myVertexAndIndexHash; }
    void SetVertexIndexHash(uint64 aHash) { myVertexAndIndexHash = aHash; }

  private:
    GeometryDataList m_vGeometries;
    uint64 myVertexAndIndexHash;
  };
  //---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

#endif  // INCLUDE_MESH_H