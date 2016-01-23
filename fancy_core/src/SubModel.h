#ifndef INCLUDE_SUBMODEL_H
#define INCLUDE_SUBMODEL_H

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "StaticManagedObject.h"
#include "Material.h"
#include "Serializable.h"
#include "MeshDesc.h"
#include "MaterialDesc.h"

namespace Fancy {
  namespace IO {
    class Serializer;
  }
}

namespace Fancy { namespace Rendering {
  class Material;
} }  // end of namespace Fancy::Rendering
//---------------------------------------------------------------------------//
namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  class Mesh;
//---------------------------------------------------------------------------//
  struct SubModelDesc
  {
    Rendering::MaterialDesc myMaterial;
    MeshDesc myMesh;

    bool operator==(const SubModelDesc& anOther) const;
    uint64 GetHash() const;
  };
//---------------------------------------------------------------------------//
  class SubModel : public StaticManagedHeapObject<SubModel>
  {
  public:
    SERIALIZABLE(SubModel)

    SubModel();
    ~SubModel();
    bool operator==(const SubModel& anOther) const;
    bool operator==(const SubModelDesc& aDesc) const;

    SubModelDesc GetDescription() const;
    void SetFromDescription(const SubModelDesc& aDesc);

    static ObjectName getTypeName() { return _N(SubModel); }
    void serialize(IO::Serializer* aSerializer);
    uint64 GetHash() const { return GetDescription().GetHash(); }

    Rendering::Material* getMaterial() const {return m_pMaterial;}
    Mesh* getMesh() const {return m_pMesh;}

    void setMesh(Mesh* _pMesh) {m_pMesh = _pMesh;}
    void setMaterial(Rendering::Material* _pMaterial) {m_pMaterial = _pMaterial;}
//---------------------------------------------------------------------------//
  private:
    Rendering::Material* m_pMaterial;
    Mesh* m_pMesh;
  }; 
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

#endif  // INCLUDE_SUBMODEL_H