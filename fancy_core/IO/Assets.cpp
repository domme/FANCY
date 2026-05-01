#include "fancy_core_precompile.h"
#include "Assets.h"
#include "Rendering//RenderCore.h"
#include "Rendering/ShaderPipeline.h"
#include "PathService.h"
#include "Material.h"
#include "Mesh.h"
#include "Rendering/GpuBuffer.h"
#include "Common/CommandLine.h"

namespace Fancy {
  ResourcePool< Mesh, 2048, MeshDesc >           Assets::ourMeshPool;
  ResourcePool< Material, 2048, MaterialDesc >   Assets::ourMaterialPool;
  //---------------------------------------------------------------------------//
  void Assets::Init() {
    // Shader resources and texture caching are now managed by RenderCore
  }
  //---------------------------------------------------------------------------//
  const eastl::hash_map< uint64, TextureViewHandle > & Assets::GetTextures() {
    return RenderCore::GetTexturePathCache();
  }
  //---------------------------------------------------------------------------//
  Mesh * Assets::GetMesh( MeshHandle aHandle ) {
    if ( !ourMeshPool.IsValid( aHandle ) )
      return nullptr;
    return ourMeshPool.Get( aHandle );
  }
  //---------------------------------------------------------------------------//
  MeshHandle Assets::GetMeshHandle( const MeshDesc & aDesc ) {
    return ourMeshPool.Get( aDesc );
  }
  //---------------------------------------------------------------------------//
  MeshHandle Assets::CreateMeshHandle( const MeshData & aMeshData ) {
    MeshHandle cached = ourMeshPool.Get( aMeshData.myDesc );
    if ( ourMeshPool.IsValid( cached ) )
      return cached;

    Mesh * mesh = new Mesh();
    mesh->myDesc = aMeshData.myDesc;

    for ( uint i = 0u; i < aMeshData.myParts.size(); ++i ) {
      const MeshPartData &  partData = aMeshData.myParts[ i ];
      SharedPtr< MeshPart > meshPart( new MeshPart() );

      const VertexInputLayoutProperties & vertexLayoutProperties = partData.myVertexLayoutProperties;
      meshPart->myVertexInputLayout = RenderCore::CreateVertexInputLayout( vertexLayoutProperties );

      const uint64 overallVertexSize = vertexLayoutProperties.GetOverallVertexSize();

      const uint8 * ptrToVertexData = partData.myVertexData.data();
      const uint64  numVertices = VECTOR_BYTESIZE( partData.myVertexData ) / overallVertexSize;
      ;

      GpuBufferProperties bufferParams;
      bufferParams.myBindFlags = ( uint ) GpuBufferBindFlags::VERTEX_BUFFER;
      bufferParams.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
      bufferParams.myNumElements = numVertices;
      bufferParams.myElementSizeBytes = overallVertexSize;

      StaticString< 256 > name( "VertexBuffer_Mesh_%d_%s_%lld", i, aMeshData.myDesc.myName.c_str(),
                                aMeshData.myDesc.myHash );
      meshPart->myVertexBuffer = RenderCore::CreateBuffer( bufferParams, name, ptrToVertexData );

      const uint8 * ptrToIndexData = partData.myIndexData.data();
      const uint64  numIndices = ( partData.myIndexData.size() * sizeof( uint8 ) ) / sizeof( uint );

      GpuBufferProperties indexBufParams;
      indexBufParams.myBindFlags = ( uint ) GpuBufferBindFlags::INDEX_BUFFER;
      indexBufParams.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
      indexBufParams.myNumElements = numIndices;
      indexBufParams.myElementSizeBytes = sizeof( uint );

      name.Format( "IndexBuffer_Mesh_%d_%s_%lld", i, aMeshData.myDesc.myName.c_str(), aMeshData.myDesc.myHash );
      meshPart->myIndexBuffer = RenderCore::CreateBuffer( indexBufParams, name, ptrToIndexData );

      mesh->myParts.push_back( meshPart );
    }

    return ourMeshPool.Add( mesh, aMeshData.myDesc );
  }
  //---------------------------------------------------------------------------//
  uint64 Assets::ComputeMeshVertexHash( const MeshPartData * someMeshPartDatas, uint aNumParts ) {
    MathUtil::BeginMultiHash();

    for ( uint i = 0u; i < aNumParts; ++i )
      MathUtil::AddToMultiHash( someMeshPartDatas[ i ].myVertexData.data(),
                                VECTOR_BYTESIZE( someMeshPartDatas[ i ].myVertexData ) );

    return MathUtil::EndMultiHash();
  }
  //---------------------------------------------------------------------------//
  Material * Assets::GetMaterial( MaterialHandle aHandle ) {
    if ( !ourMaterialPool.IsValid( aHandle ) )
      return nullptr;
    return ourMaterialPool.Get( aHandle );
  }
  //---------------------------------------------------------------------------//
  MaterialHandle Assets::GetMaterialHandle( const MaterialDesc & aDesc ) {
    return ourMaterialPool.Get( aDesc );
  }
  //---------------------------------------------------------------------------//
  MaterialHandle Assets::CreateMaterialHandle( const MaterialDesc & aDesc ) {
    MaterialHandle cached = ourMaterialPool.Get( aDesc );
    if ( ourMaterialPool.IsValid( cached ) )
      return cached;

    Material * mat = new Material();

    for ( uint i = 0u; i < ( uint ) MaterialTextureType::NUM; ++i )
      if ( !aDesc.myTextures[ i ].empty() )
        mat->myTextures[ i ] = LoadTexture( aDesc.myTextures[ i ].c_str() );

    static_assert( sizeof( mat->myParameters ) == sizeof( aDesc.myParameters ), "Mismatch in parameter data size" );
    memcpy( mat->myParameters, aDesc.myParameters, sizeof( mat->myParameters ) );

    return ourMaterialPool.Add( mat, aDesc );
  }
  //---------------------------------------------------------------------------//
  TextureViewHandle Assets::GetTexture( const char * aPath, uint someFlags ) {
    return RenderCore::GetTextureByPath( aPath, someFlags );
  }
  //---------------------------------------------------------------------------//
  TextureViewHandle Assets::LoadTexture( const char * aPath, uint someLoadFlags /* = 0 */ ) {
    return RenderCore::LoadTexture( aPath, someLoadFlags );
  }
  //---------------------------------------------------------------------------//
  void Assets::ComputeMipmaps( TextureHandle aTextureHandle, Assets::ResampleFilter /*aFilter*/ ) {
    RenderCore::ComputeMipmaps( aTextureHandle );
  }
  //---------------------------------------------------------------------------//
  const ShaderPipeline * Assets::GetMipDownsampleShader() {
    return RenderCore::GetMipDownsampleShader();
  }
}  // namespace Fancy
