#pragma once
#include "fancy_core/FancyCorePrerequisites.h"

namespace Fancy {
  struct MeshData;
  //---------------------------------------------------------------------------//
  class Mesh;
  struct MeshDesc;
  class Material;
  struct Model;
  struct ModelDesc;
  struct MaterialDesc;
  class Texture;
  struct TextureDesc;
//---------------------------------------------------------------------------//
  class GraphicsWorld
  {
  public:
    GraphicsWorld();
    ~GraphicsWorld();

    SharedPtr<Texture> GetTexture(const TextureDesc& aDesc);
    SharedPtr<Texture> CreateTexture(const char* aPath);
    SharedPtr<Texture> CreateTexture(const TextureDesc& aDesc);

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


