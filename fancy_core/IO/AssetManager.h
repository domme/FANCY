#pragma once

#include "MeshImporter.h"
#include "EASTL/hash_map.h"

namespace Fancy
{
  class ShaderPipeline;
  struct Material;
  struct Mesh;
  struct MaterialDesc;
  struct MeshPartData;
  struct MeshDesc;

  class AssetManager
  {
  public:
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

    AssetManager();
  
    SharedPtr<Mesh> GetMesh(const MeshDesc& aDesc);
    SharedPtr<Mesh> CreateMesh(const MeshData& aMeshData);
    uint64 ComputeMeshVertexHash(const MeshPartData* someMeshPartDatas, uint aNumParts);

    SharedPtr<Material> GetMaterial(const MaterialDesc& aDesc);
    SharedPtr<Material> CreateMaterial(const MaterialDesc& aDesc);

    SharedPtr<TextureView> GetTexture(const char* aPath, uint someLoadFlags = 0);
    SharedPtr<TextureView> LoadTexture(const char* aPath, uint someLoadFlags = 0); 
    void ComputeMipmaps(const SharedPtr<Texture>& aTexture, ResampleFilter aFilter = FILTER_LANCZOS);

    const ShaderPipeline* GetMipDownsampleShader() { return ourMipDownsampleShader.get(); }

  private:
    eastl::hash_map<uint64, SharedPtr<TextureView>> ourTextureCache;
    eastl::hash_map<uint64, SharedPtr<Mesh>> ourMeshCache;
    eastl::hash_map<uint64, SharedPtr<Material>> ourMaterialCache;
    SharedPtr<ShaderPipeline> ourMipDownsampleShader;
  };
}
