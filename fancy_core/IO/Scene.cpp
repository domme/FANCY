#include "fancy_core_precompile.h"
#include "Scene.h"
#include "Rendering/RenderCore.h"
#include "Assets.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  Scene::Scene(const SceneData& aData)
    : myInstances(aData.myInstances)
  {
    myMeshes.reserve(aData.myMeshes.size());
    for (const MeshData& meshData : aData.myMeshes)
      myMeshes.push_back(Assets::CreateMesh(meshData));

    myMaterials.reserve(aData.myMaterials.size());
    for (const MaterialDesc& material : aData.myMaterials)
      myMaterials.push_back(Assets::CreateMaterial(material));

    myVertexInputLayout = RenderCore::CreateVertexInputLayout(aData.myVertexInputLayoutProperties);
  }
//---------------------------------------------------------------------------//
}
