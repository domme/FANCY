#pragma once
#include <fancy_core/FancyCoreDefines.h>
#include <fancy_core/GpuProgram.h>
#include <fancy_core/Ptr.h>

#include <map>

namespace Fancy {  
//---------------------------------------------------------------------------//
  struct MeshData;
  class Mesh;
  struct MeshDesc;
  struct Material;
  struct Model;
  struct ModelDesc;
  struct MaterialDesc;
  class Texture;
  class GpuProgram;
//---------------------------------------------------------------------------//
  class AssetManager
  {
  public:
    AssetManager();
    ~AssetManager() = default;
    void Clear();

    enum TextureLoadFlags
    {
      NO_DISK_CACHE = 1 << 0,
      NO_MEM_CACHE = 1 << 1,
      SHADER_WRITABLE = 1 << 2
    };

    enum ResampleFilter
    {
      FILTER_LINEAR = 0,
      FILTER_LANCZOS,

      FILTER_NUM
    };

    SharedPtr<Texture> GetTexture(const char* aPath, uint someFlags = 0);
    SharedPtr<Texture> CreateTexture(const char* aPath, uint someLoadFlags = 0);
    void ComputeMipmaps(const SharedPtr<Texture>& aTexture, ResampleFilter aFilter = FILTER_LANCZOS);

    GpuProgram* GetResizeShader() const { return myTextureResizeShader.get(); }

    SharedPtr<Material> CreateMaterial(const MaterialDesc& aDesc);
    SharedPtr<Model> CreateModel(const ModelDesc& aDesc);

    SharedPtr<Mesh> GetMesh(const MeshDesc& aDesc);
    SharedPtr<Mesh> CreateMesh(const MeshDesc& aDesc, MeshData* someMeshDatas, uint aNumMeshDatas, uint64 aMeshFileTimestamp = 0u);

  private:
    std::map<uint64, SharedPtr<Material>> myMaterials;
    std::map<uint64, SharedPtr<Model>> myModels;
    std::map<uint64, SharedPtr<Texture>> myTextures;
    std::map<uint64, SharedPtr<Mesh>> myMeshes;

    SharedPtr<GpuProgram> myTextureResizeShader;
  };
//---------------------------------------------------------------------------//
}


