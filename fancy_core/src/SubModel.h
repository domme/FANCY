#ifndef INCLUDE_SUBMODEL_H
#define INCLUDE_SUBMODEL_H

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"
#include "StaticManagedObject.h"

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
	  MaterialDesc myMaterial;
	  MeshDesc myMesh;
	  ObjectNameDesc myName;
  };
//---------------------------------------------------------------------------//
  class SubModel : public StaticManagedHeapObject<SubModel>
  {
  public:
    SubModel();
    ~SubModel();

    const ObjectName& getName() const {return m_Name;}
    void setName(const ObjectName& clNewName) {m_Name = clNewName;}

    Rendering::Material* getMaterial() const {return m_pMaterial;}
    Mesh* getMesh() const {return m_pMesh;}

    void setMesh(Mesh* _pMesh) {m_pMesh = _pMesh;}
    void setMaterial(Rendering::Material* _pMaterial) {m_pMaterial = _pMaterial;}
//---------------------------------------------------------------------------//
  private:
    Rendering::Material* m_pMaterial;
    Mesh* m_pMesh;
    ObjectName m_Name;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

#endif  // INCLUDE_SUBMODEL_H