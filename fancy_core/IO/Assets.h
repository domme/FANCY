#pragma once

#include "MeshImporter.h"
#include "EASTL/hash_map.h"
#include "Rendering/ResourceHandle.h"
#include "Rendering/ResourcePool.h"

namespace Fancy {
  struct MeshData;
  class ShaderPipeline;
  struct Material;
  struct Mesh;
  struct MaterialDesc;
  struct MeshPartData;
  struct MeshDesc;

  // Handle declarations for Mesh and Material (high-level asset concepts)
  using MeshHandle = ResourceHandle< Mesh >;
  using MaterialHandle = ResourceHandle< Material >;

  class Assets {
  public:
    enum TextureLoadFlags {
      NO_DISK_CACHE = 1 << 0,
      NO_MEM_CACHE = 1 << 1,
      SHADER_WRITABLE = 1 << 2,
      NO_MIP_GENERATION = 1 << 3,
    };

    enum ResampleFilter {
      FILTER_LINEAR = 0,
      FILTER_LANCZOS,

      FILTER_NUM
    };

    Assets() = delete;  // Static-only class

    static void Init();

    static Mesh *     GetMesh( MeshHandle aHandle );
    static MeshHandle GetMeshHandle( const MeshDesc & aDesc );
    static MeshHandle CreateMeshHandle( const MeshData & aMeshData );
    static uint64     ComputeMeshVertexHash( const MeshPartData * someMeshPartDatas, uint aNumParts );

    static Material *     GetMaterial( MaterialHandle aHandle );
    static MaterialHandle GetMaterialHandle( const MaterialDesc & aDesc );
    static MaterialHandle CreateMaterialHandle( const MaterialDesc & aDesc );

    static const eastl::hash_map< uint64, TextureViewHandle > & GetTextures();
    static TextureViewHandle GetTexture( const char * aPath, uint someLoadFlags = 0 );
    static TextureViewHandle LoadTexture( const char * aPath, uint someLoadFlags = 0 );
    static void              ComputeMipmaps( TextureHandle aTexture, ResampleFilter aFilter = FILTER_LANCZOS );

    static const ShaderPipeline * GetMipDownsampleShader();

  private:
    static ResourcePool< Mesh, 2048, MeshDesc >           ourMeshPool;
    static ResourcePool< Material, 2048, MaterialDesc >   ourMaterialPool;
  };
}  // namespace Fancy
