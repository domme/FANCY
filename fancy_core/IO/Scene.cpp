#include "fancy_core_precompile.h"
#include "Scene.h"
#include "Rendering/RenderCore.h"
#include "ObjectCore.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  Scene::Scene(const SceneData& aData)
  {
    myMeshes.reserve(aData.myMeshes.size());
    for (const MeshData& meshData : aData.myMeshes)
      myMeshes.push_back(ObjectCore::CreateMesh(meshData));

    myMaterials.reserve(aData.myMaterials.size());
    for (const MaterialDesc& material : aData.myMaterials)
      myMaterials.push_back(ObjectCore::CreateMaterial(material));

    myVertexInputLayout = RenderCore::CreateVertexInputLayout(aData.myVertexInputLayoutProperties);

    myInstances = aData.myInstances;
  }
//---------------------------------------------------------------------------//
}
