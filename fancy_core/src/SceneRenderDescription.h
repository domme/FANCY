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
    RenderingItem() : pGeometry(nullptr), pMaterialPassInstance(nullptr), pWorldMat(nullptr) {}
    RenderingItem(const Geometry::GeometryData* _pGeometry, const MaterialPassInstance* _pMaterialPassInstance, const glm::mat4* _pWorldMat) :
      pGeometry(_pGeometry), pMaterialPassInstance(_pMaterialPassInstance), pWorldMat(_pWorldMat) {}

    const glm::mat4* pWorldMat;
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
  typedef FixedArray<Rendering::RenderingItem, kMaxNumRenderingItems> RenderingItemList;
//---------------------------------------------------------------------------//
    class SceneRenderDescription
    {
    public:
      RenderingItemList techniqueItemList[(uint32)Rendering::EMaterialPass::NUM];
    };
  //---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene


#endif // INCLUDE_SCENERENDERDESCRIPTION_H