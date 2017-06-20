#ifndef INCLUDE_SUBMODEL_H
#define INCLUDE_SUBMODEL_H

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "StaticManagedObject.h"
#include "Material.h"
#include "Serializable.h"
#include "MeshDesc.h"
#include "MaterialDesc.h"
#include "DescriptionBase.h"

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
  struct SubModelDesc : DescriptionBase
  {
    Rendering::MaterialDesc myMaterial;
    MeshDesc myMesh;

    ~SubModelDesc() override { }
    ObjectName GetTypeName() const override { return _N(SubModel); }
    void Serialize(IO::Serializer* aSerializer) override;
    uint64 GetHash() const override;
    bool IsEmpty() const override { return myMaterial.IsEmpty() || myMesh.IsEmpty(); }

    bool operator==(const SubModelDesc& anOther) const;
  };
//---------------------------------------------------------------------------//
  class SubModel
  {
  public:
    SERIALIZABLE_RESOURCE(SubModel)

    SubModel() = default;
    ~SubModel() = default;
    bool operator==(const SubModel& anOther) const;
    bool operator==(const SubModelDesc& aDesc) const;

    SubModelDesc GetDescription() const;
    void SetFromDescription(const SubModelDesc& aDesc, GraphicsWorld* aWorld);

    uint64 GetHash() const { return GetDescription().GetHash(); }

    Rendering::Material* getMaterial() const {return m_pMaterial.get();}
    Mesh* getMesh() const {return m_pMesh.get();}

//---------------------------------------------------------------------------//
  private:
    SharedPtr<Rendering::Material> m_pMaterial;
    SharedPtr<Mesh> m_pMesh;
  }; 
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

#endif  // INCLUDE_SUBMODEL_H