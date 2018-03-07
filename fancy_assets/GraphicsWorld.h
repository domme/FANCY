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

    Texture* GetTexture(uint64 aDescHash, bool aUseDiskCache = true);
    Texture* CreateTexture(const TextureDesc& aDesc);
    Texture* CreateTexture(const char* aPath);

    Material* CreateMaterial(const MaterialDesc& aDesc);
    Model* CreateModel(const ModelDesc& aDesc);

    Mesh* GetMesh(uint64 aDescHash, bool aUseDiskCache = true);
    Mesh* CreateMesh(const MeshDesc& aDesc,
      const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas,
      const std::vector<uint>& someNumVertices, const std::vector<uint>& someNumIndices);


  private:
    std::map<uint64, UniquePtr<Material>> myMaterials;
    std::map<uint64, UniquePtr<Model>> myModels;
    std::map<uint64, UniquePtr<Texture>> myTextures;
    std::map<uint64, UniquePtr<Mesh>> myMeshes;
  };
//---------------------------------------------------------------------------//
}


