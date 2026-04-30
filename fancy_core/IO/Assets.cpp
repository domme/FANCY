#include "fancy_core_precompile.h"
#include "Assets.h"
#include "Rendering//RenderCore.h"
#include "Rendering/ShaderPipeline.h"
#include "PathService.h"
#include "Material.h"
#include "Mesh.h"
#include "Rendering/GpuBuffer.h"
#include "Rendering/CommandList.h"
#include "Rendering/Texture.h"
#include "BinaryCache.h"
#include "ImageLoader.h"
#include "Rendering/TextureReadbackTask.h"
#include "Rendering/GraphicsResources.h"
#include "Common/CommandLine.h"

namespace Fancy {
  eastl::hash_map< uint64, TextureViewHandle > Assets::ourTextureCache;
  eastl::hash_map< uint64, SharedPtr< Mesh > > Assets::ourMeshCache;
  eastl::hash_map< uint64, SharedPtr< Material > > Assets::ourMaterialCache;
  ShaderPipelineHandle Assets::ourMipDownsampleShader;
  //---------------------------------------------------------------------------//
  void Assets::Init() {
    ShaderPipelineDesc pipelineDesc;
    ShaderDesc & shaderDesc = pipelineDesc.myShader[ ( uint ) ShaderStage::SHADERSTAGE_COMPUTE ];
    shaderDesc.myPath = "fancy/resources/shaders/Downsample.hlsl";
    shaderDesc.myShaderStage = ( uint ) ShaderStage::SHADERSTAGE_COMPUTE;
    shaderDesc.myMainFunction = "main";

    ourMipDownsampleShader = RenderCore::CreateShaderPipeline( pipelineDesc );
    ASSERT( ourMipDownsampleShader.IsValid() );
  }
  //---------------------------------------------------------------------------//
  SharedPtr< Mesh > Assets::GetMesh( const MeshDesc & aDesc ) {
    auto it = ourMeshCache.find( aDesc.myHash );
    if ( it != ourMeshCache.end() )
      return it->second;

    return nullptr;
  }
  //---------------------------------------------------------------------------//
  SharedPtr< Mesh > Assets::CreateMesh( const MeshData & aMeshData ) {
    SharedPtr< Mesh > cachedMesh = GetMesh( aMeshData.myDesc );
    if ( cachedMesh )
      return cachedMesh;

    SharedPtr< Mesh > mesh( new Mesh() );
    mesh->myDesc = aMeshData.myDesc;

    for ( uint i = 0u; i < aMeshData.myParts.size(); ++i ) {
      const MeshPartData & partData = aMeshData.myParts[ i ];
      SharedPtr< MeshPart > meshPart( new MeshPart() );

      const VertexInputLayoutProperties & vertexLayoutProperties = partData.myVertexLayoutProperties;
      meshPart->myVertexInputLayout = RenderCore::CreateVertexInputLayout( vertexLayoutProperties );

      const uint64 overallVertexSize = vertexLayoutProperties.GetOverallVertexSize();

      const uint8 * ptrToVertexData = partData.myVertexData.data();
      const uint64 numVertices = VECTOR_BYTESIZE( partData.myVertexData ) / overallVertexSize;
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
      const uint64 numIndices = ( partData.myIndexData.size() * sizeof( uint8 ) ) / sizeof( uint );

      GpuBufferProperties indexBufParams;
      indexBufParams.myBindFlags = ( uint ) GpuBufferBindFlags::INDEX_BUFFER;
      indexBufParams.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
      indexBufParams.myNumElements = numIndices;
      indexBufParams.myElementSizeBytes = sizeof( uint );

      name.Format( "IndexBuffer_Mesh_%d_%s_%lld", i, aMeshData.myDesc.myName.c_str(), aMeshData.myDesc.myHash );
      meshPart->myIndexBuffer = RenderCore::CreateBuffer( indexBufParams, name, ptrToIndexData );

      mesh->myParts.push_back( meshPart );
    }

    ourMeshCache[ aMeshData.myDesc.myHash ] = mesh;

    return mesh;
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
  SharedPtr< Material > Assets::GetMaterial( const MaterialDesc & aDesc ) {
    const uint64 hash = aDesc.GetHash();

    auto it = ourMaterialCache.find( hash );
    if ( it != ourMaterialCache.end() )
      return it->second;

    return nullptr;
  }
  //---------------------------------------------------------------------------//
  SharedPtr< Material > Assets::CreateMaterial( const MaterialDesc & aDesc ) {
    SharedPtr< Material > mat = GetMaterial( aDesc );
    if ( mat )
      return mat;

    mat.reset( new Material() );

    for ( uint i = 0u; i < ( uint ) MaterialTextureType::NUM; ++i )
      if ( !aDesc.myTextures[ i ].empty() )
        mat->myTextures[ i ] = LoadTexture( aDesc.myTextures[ i ].c_str() );

    static_assert( sizeof( mat->myParameters ) == sizeof( aDesc.myParameters ), "Mismatch in parameter data size" );
    memcpy( mat->myParameters, aDesc.myParameters, sizeof( mat->myParameters ) );

    ourMaterialCache[ aDesc.GetHash() ] = mat;
    return mat;
  }
  //---------------------------------------------------------------------------//
  TextureViewHandle Assets::GetTexture( const char * aPath, uint someFlags ) {
    eastl::string texPathAbs = aPath;
    eastl::string texPathRel = aPath;
    if ( !Path::IsPathAbsolute( texPathAbs ) )
      texPathAbs = Path::GetAbsolutePath( texPathAbs );
    else
      texPathRel = Path::GetRelativePath( texPathAbs );

    uint64 texPathRelHash = MathUtil::Hash( texPathRel );
    MathUtil::hash_combine( texPathRelHash, ( ( uint64 ) someFlags & SHADER_WRITABLE ) );

    auto it = ourTextureCache.find( texPathRelHash );
    if ( it != ourTextureCache.end() )
      return it->second;

    return TextureViewHandle{};
  }
  //---------------------------------------------------------------------------//
  TextureViewHandle Assets::LoadTexture( const char * aPath, uint someLoadFlags /* = 0 */ ) {
    if ( strlen( aPath ) == 0 )
      return TextureViewHandle{};

    eastl::string texPathAbs = aPath;
    eastl::string texPathRel = aPath;
    if ( !Path::IsPathAbsolute( texPathAbs ) )
      texPathAbs = Path::GetAbsolutePath( texPathAbs );
    else
      texPathRel = Path::GetRelativePath( texPathAbs );

    if ( ( someLoadFlags & NO_MEM_CACHE ) == 0 ) {
      TextureViewHandle cached = GetTexture( texPathRel.c_str() );
      if ( cached.IsValid() && RenderCore::GetTextureView( cached )->GetProperties().myIsShaderWritable ==
                                   ( ( someLoadFlags & SHADER_WRITABLE ) != 0 ) )
        return cached;
    }

    uint64 texPathRelHash = MathUtil::Hash( texPathRel );
    MathUtil::hash_combine( texPathRelHash, ( ( uint64 ) someLoadFlags & SHADER_WRITABLE ) );

    ImageData image;
    if ( !ImageLoader::Load( texPathAbs.c_str(), someLoadFlags, image ) ) {
      LOG_ERROR( "Failed to load texture at path %s", texPathAbs.c_str() );
      return TextureViewHandle{};
    }

    TextureHandle texHandle = RenderCore::CreateTexture(
        image.myProperties, texPathRel.c_str(), image.myData.mySubDatas.data(), image.myData.mySubDatas.size() );
    if ( !texHandle.IsValid() ) {
      LOG_ERROR( "Failed to create loaded texture at path %s", texPathAbs.c_str() );
      return TextureViewHandle{};
    }

    if ( RenderCore::GetTexture( texHandle )->GetProperties().myNumMipLevels == 1 &&
         ( someLoadFlags & NO_MIP_GENERATION ) == 0 )
      ComputeMipmaps( texHandle );

    Texture * tex = RenderCore::GetTexture( texHandle );
    TextureViewProperties viewProps;
    viewProps.mySubresourceRange = tex->mySubresources;
    viewProps.myFormat = tex->GetProperties().myFormat;
    TextureViewHandle texViewHandle = RenderCore::CreateTextureView( tex, viewProps, aPath );

    ourTextureCache[ texPathRelHash ] = texViewHandle;
    return texViewHandle;
  }
  //---------------------------------------------------------------------------//
  void Assets::ComputeMipmaps( TextureHandle aTextureHandle, Assets::ResampleFilter /*aFilter*/ ) {
    if ( !aTextureHandle.IsValid() )
      return;

    Texture * aTexture = RenderCore::GetTexture( aTextureHandle );
    const TextureProperties & texProps = aTexture->GetProperties();
    const uint numMips = texProps.myNumMipLevels;

    TextureHandle textureHandle = aTextureHandle;
    if ( !texProps.myIsShaderWritable ) {
      TextureProperties props = texProps;
      props.myIsRenderTarget = false;
      props.myIsShaderWritable = true;
      textureHandle = RenderCore::CreateTexture( props, "Mipmap target UAV copy" );
    }
    Texture * texture = RenderCore::GetTexture( textureHandle );

    TextureResourceProperties tempTexProps;
    tempTexProps.myIsShaderWritable = true;
    tempTexProps.myIsRenderTarget = false;
    tempTexProps.myIsTexture = true;
    tempTexProps.myTextureProperties = texProps;
    tempTexProps.myTextureProperties.myWidth = texProps.myWidth / 2;
    tempTexProps.myTextureProperties.myHeight = texProps.myHeight / 2;
    tempTexProps.myTextureProperties.myNumMipLevels = 1u;
    TempTextureResource tempTexture = RenderCore::AllocateTempTexture( tempTexProps, 0u, "Temp mipmapping texture" );

    eastl::fixed_vector< TextureViewHandle, 10 > readViews;
    eastl::fixed_vector< TextureViewHandle, 10 > writeViews;
    readViews.resize( numMips );
    writeViews.resize( numMips );

    TextureViewProperties props;
    props.mySubresourceRange.myNumMipLevels = 1;
    for ( uint mip = 0u; mip < numMips; ++mip ) {
      props.mySubresourceRange.myFirstMipLevel = mip;
      props.myIsShaderWritable = false;
      props.myFormat = texture->GetProperties().myFormat;
      readViews[ mip ] = RenderCore::CreateTextureView( texture, props );

      props.myIsShaderWritable = true;
      props.myFormat = DataFormatInfo::GetNonSRGBformat( props.myFormat );
      writeViews[ mip ] = RenderCore::CreateTextureView( texture, props );
    }

    CommandList * ctx = RenderCore::BeginCommandList( CommandListType::Graphics );
    if ( textureHandle != aTextureHandle )
      ctx->CopyTexture( texture, SubresourceLocation( 0 ), aTexture, SubresourceLocation( 0 ) );

    ctx->SetShaderPipeline( RenderCore::GetShaderPipeline( ourMipDownsampleShader ) );

    struct CBuffer {
      glm::int2 mySrcTextureSize;
      int myIsSRGB;
      int mySrcTextureIdx;
      int myDstTextureIdx;
    } cBuffer;

    const DataFormatInfo & formatInfo = DataFormatInfo::GetFormatInfo( texProps.myFormat );
    cBuffer.myIsSRGB = formatInfo.mySRGB ? 1 : 0;

    const glm::int2 fullSize( texProps.myWidth, texProps.myHeight );
    for ( uint mip = 1u; mip < numMips; ++mip ) {
      const glm::int2 srcSize = fullSize >> ( int ) ( mip - 1 );
      const glm::int2 dstSize = fullSize >> ( int ) mip;

      TextureView * readView = RenderCore::GetTextureView( readViews[ mip - 1 ] );
      TextureView * writeView = RenderCore::GetTextureView( writeViews[ mip ] );
      cBuffer.mySrcTextureIdx = readView->GetGlobalDescriptorIndex();
      cBuffer.myDstTextureIdx = writeView->GetGlobalDescriptorIndex();
      cBuffer.mySrcTextureSize = glm::int2( ( int ) srcSize.x, ( int ) srcSize.y );
      ctx->BindConstantBuffer( &cBuffer, sizeof( cBuffer ), 0 );
      ctx->PrepareResourceShaderAccess( readView );
      ctx->PrepareResourceShaderAccess( writeView );
      ctx->Dispatch( glm::int3( dstSize.x, dstSize.y, 1 ) );
      ctx->ResourceUAVbarrier();
    }

    if ( aTextureHandle != textureHandle )
      ctx->CopyResource( aTexture, texture );

    RenderCore::ExecuteAndFreeCommandList( ctx, SyncMode::BLOCKING );
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy
