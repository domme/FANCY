#pragma once
#include "fancy_core/FancyCorePrerequisites.h"

namespace Fancy {
  struct MeshData;
  //---------------------------------------------------------------------------//
  class Mesh;
  struct MeshDesc;
  struct Material;
  struct Model;
  struct ModelDesc;
  struct MaterialDesc;
  class Texture;
  struct TextureDesc;
  struct TextureView;
//---------------------------------------------------------------------------//
  class AssetStorage
  {
  public:
    AssetStorage();
    ~AssetStorage();

    SharedPtr<TextureView> GetTexture(const TextureDesc& aDesc);
    SharedPtr<TextureView> CreateTexture(const char* aPath);
    SharedPtr<TextureView> CreateTexture(const TextureDesc& aDesc);

    SharedPtr<Material> CreateMaterial(const MaterialDesc& aDesc);
    SharedPtr<Model> CreateModel(const ModelDesc& aDesc);

    SharedPtr<Mesh> GetMesh(const MeshDesc& aDesc);
    SharedPtr<Mesh> CreateMesh(const MeshDesc& aDesc, MeshData* someMeshDatas, uint aNumMeshDatas, uint64 aMeshFileTimestamp = 0u);

  private:
    std::map<uint64, SharedPtr<Material>> myMaterials;
    std::map<uint64, SharedPtr<Model>> myModels;
    std::map<uint64, SharedPtr<TextureView>> myTextures;
    std::map<uint64, SharedPtr<Mesh>> myMeshes;
  };
//---------------------------------------------------------------------------//
}


