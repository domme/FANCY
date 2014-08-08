#ifndef INCLUDE_SUBMODEL_H
#define INCLUDE_SUBMODEL_H

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"

namespace Fancy { namespace Core { namespace Geometry {
//---------------------------------------------------------------------------//
  class Mesh;
  class Material;
//---------------------------------------------------------------------------//
  class SubModel 
  {
  public:
    SubModel();
    ~SubModel();

    const ObjectName& getName() {return m_Name;}
    void setName(const ObjectName& clNewName) {m_Name = clNewName;}

    Material* getMaterial() {return m_pMaterial;}
    Mesh* getMesh() {return m_pMesh;}
    
   //---------------------------------------------------------------------------//
  private:
    Material* m_pMaterial;
    Mesh* m_pMesh;
    ObjectName m_Name;
  };
  //---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Core::Geometry

#endif  // INCLUDE_SUBMODEL_H