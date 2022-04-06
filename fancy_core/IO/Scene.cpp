#include "fancy_core_precompile.h"
#include "Scene.h"
#include "Rendering/RenderCore.h"
#include "AssetManager.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  Scene::Scene(const SceneData& aData, AssetManager* anAssetManager)
    : myInstances(aData.myInstances)
  {
    myMeshes.reserve(aData.myMeshes.size());
    for (const MeshData& meshData : aData.myMeshes)
      myMeshes.push_back(anAssetManager->CreateMesh(meshData));

    myMaterials.reserve(aData.myMaterials.size());
    for (const MaterialDesc& material : aData.myMaterials)
      myMaterials.push_back(anAssetManager->CreateMaterial(material));

    myVertexInputLayout = RenderCore::CreateVertexInputLayout(aData.myVertexInputLayoutProperties);
  }
//---------------------------------------------------------------------------//
}
