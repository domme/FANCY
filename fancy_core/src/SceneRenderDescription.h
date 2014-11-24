#ifndef INCLUDE_SCENERENDERDESCRIPTION_H
#define INCLUDE_SCENERENDERDESCRIPTION_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "FixedArray.h"

// Forward declarations:
namespace Fancy { namespace Geometry { 
  class GeometryData;
} } 
namespace Fancy { namespace Rendering { 
  class MaterialPassInstance;
} } 

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct RenderingItem
  {
    RenderingItem(const Geometry::GeometryData* _pGeometry, 
      const MaterialPassInstance* _pMaterialPassInstance) :
      pGeometry(_pGeometry), pMaterialPassInstance(_pMaterialPassInstance) {}

    const Geometry::GeometryData* pGeometry;
    const MaterialPassInstance* pMaterialPassInstance;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  enum SceneRenderingConstants {
    kMaxNumRenderingItems = 1024u,
  };
//---------------------------------------------------------------------------//
  typedef FixedArray<Rendering::RenderingItem, kMaxNumRenderingItems> 
    RenderingItemList;
  typedef FixedArray<RenderingItemList, (uint) Rendering::EMaterialPass::NUM> 
    TechniqueRenderingItemList;
//---------------------------------------------------------------------------//
    struct SceneRenderDescription
    {
      TechniqueRenderingItemList techniqueItemList;
    };
  //---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene


#endif // INCLUDE_SCENERENDERDESCRIPTION_H