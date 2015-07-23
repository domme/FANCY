#ifndef INCLUDE_SUBMODEL_H
#define INCLUDE_SUBMODEL_H

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "StaticManagedObject.h"
#include "Material.h"
#include "Serializable.h"

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
	  //MeshDesc myMesh;
	  ObjectNameDesc myName;
  };
//---------------------------------------------------------------------------//
  class SubModel : public StaticManagedHeapObject<SubModel>
  {
  public:
    SERIALIZABLE(SubModel)

    SubModel();
    ~SubModel();

    static ObjectName getTypeName() { return _N(SubModel); }
    void serialize(IO::Serializer* aSerializer);

    const ObjectName& getName() const {return m_Name;}
    void setName(const ObjectName& clNewName) {m_Name = clNewName;}

    Rendering::Material* getMaterial() const {return m_pMaterial;}
    Mesh* getMesh() const {return m_pMesh;}

    void setMesh(Mesh* _pMesh) {m_pMesh = _pMesh;}
    void setMaterial(Rendering::Material* _pMaterial) {m_pMaterial = _pMaterial;}
//---------------------------------------------------------------------------//
  private:
    ObjectName m_Name;
    Rendering::Material* m_pMaterial;
    Mesh* m_pMesh;
  }; 
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

#endif  // INCLUDE_SUBMODEL_H