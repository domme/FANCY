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
//---------------------------------------------------------------------------//
  class AssetStorage
  {
  public:
    AssetStorage() = default;
    ~AssetStorage() = default;
    void Clear();

    enum TextureLoadFlags
    {
      NO_DISK_CACHE = 1 << 0,
      NO_MEM_CACHE = 1 << 1,
      SHADER_WRITABLE = 1 << 2
    };

    SharedPtr<Texture> GetTexture(const char* aPath, uint someFlags = 0);
    SharedPtr<Texture> CreateTexture(const char* aPath, uint someLoadFlags = 0);

    SharedPtr<Material> CreateMaterial(const MaterialDesc& aDesc);
    SharedPtr<Model> CreateModel(const ModelDesc& aDesc);

    SharedPtr<Mesh> GetMesh(const MeshDesc& aDesc);
    SharedPtr<Mesh> CreateMesh(const MeshDesc& aDesc, MeshData* someMeshDatas, uint aNumMeshDatas, uint64 aMeshFileTimestamp = 0u);

  private:
    std::map<uint64, SharedPtr<Material>> myMaterials;
    std::map<uint64, SharedPtr<Model>> myModels;
    std::map<uint64, SharedPtr<Texture>> myTextures;
    std::map<uint64, SharedPtr<Mesh>> myMeshes;
  };
//---------------------------------------------------------------------------//
}


