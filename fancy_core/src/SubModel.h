#ifndef INCLUDE_SUBMODEL_H
#define INCLUDE_SUBMODEL_H

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"

namespace Fancy { namespace Rendering {
  class Material;
} }  // end of namespace Fancy::Rendering
//---------------------------------------------------------------------------//
namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  class Mesh;
//---------------------------------------------------------------------------//
  class SubModel 
  {
  public:
    SubModel();
    ~SubModel();

    const ObjectName& getName() const {return m_Name;}
    void setName(const ObjectName& clNewName) {m_Name = clNewName;}

    Rendering::Material* getMaterial() const {return m_pMaterial;}
    Mesh* getMesh() const {return m_pMesh;}
    
   //---------------------------------------------------------------------------//
  private:
    Rendering::Material* m_pMaterial;
    Mesh* m_pMesh;
    ObjectName m_Name;
  };
  //---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

#endif  // INCLUDE_SUBMODEL_H