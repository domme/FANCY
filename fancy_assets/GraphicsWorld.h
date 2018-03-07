#pragma once
#include "fancy_core/FancyCorePrerequisites.h"

namespace Fancy {
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

    SharedPtr<Texture> GetTexture(uint64 aDescHash);
    SharedPtr<Texture> CreateTexture(const char* aPath);
    SharedPtr<Texture> CreateTexture(const TextureDesc& aDesc);

    SharedPtr<Material> CreateMaterial(const MaterialDesc& aDesc);
    SharedPtr<Model> CreateModel(const ModelDesc& aDesc);

    SharedPtr<Mesh> GetMesh(uint64 aDescHash);
    SharedPtr<Mesh> CreateMesh(const MeshDesc& aDesc,
      const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas,
      const std::vector<uint>& someNumVertices, const std::vector<uint>& someNumIndices);

  private:
    std::map<uint64, SharedPtr<Material>> myMaterials;
    std::map<uint64, SharedPtr<Model>> myModels;
    std::map<uint64, SharedPtr<Texture>> myTextures;
    std::map<uint64, SharedPtr<Mesh>> myMeshes;
  };
//---------------------------------------------------------------------------//
}


