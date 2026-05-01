#include "fancy_core_precompile.h"
#include "RenderCore.h"

#include "DX12/RenderCore_PlatformDX12.h"

#include "DepthStencilState.h"
#include "GpuBuffer.h"
#include "GpuRingBuffer.h"
#include "GpuReadbackBuffer.h"
#include "ShaderCompiler.h"

#include "ShaderPipeline.h"
#include "BlendState.h"
#include "Shader.h"
#include "RenderOutput.h"
#include "CommandList.h"
#include "TextureProperties.h"
#include "Texture.h"
#include "TempResourcePool.h"
#include "GpuQueryHeap.h"
#include "TextureReadbackTask.h"

#include "TextureSampler.h"
#include "RtAccelerationStructure.h"
#include "RtPipelineState.h"
#include "RtShaderBindingTable.h"
#include "Common/CircularArray.h"

#include "Common/TimeManager.h"
#include "Common/CommandLine.h"

#include "IO/FileWatcher.h"
#include "IO/PathService.h"
#include "IO/Mesh.h"
#include "IO/ImageLoader.h"
#include "IO/Assets.h"

using namespace Fancy;

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
namespace {
  const char * locGetQueryTypeName( GpuQueryType aQueryType ) {
    switch ( aQueryType ) {
      case GpuQueryType::TIMESTAMP:
        return "Timestamp";
      case GpuQueryType::OCCLUSION:
        return "Occlusion";
      case GpuQueryType::NUM:
      default:
        ASSERT( false );
        return "";
    }
  }
}  // namespace
//---------------------------------------------------------------------------//
Slot< void( const ShaderPipeline * ) >  RenderCore::ourOnShaderPipelineRecompiled;
Slot< void( const RtPipelineState * ) > RenderCore::ourOnRtPipelineStateRecompiled;
bool                                    RenderCore::ourDebugLogResourceBarriers = false;
bool                                    RenderCore::ourDebugWaitAfterEachSubmit = false;

UniquePtr< RenderCore_Platform > RenderCore::ourPlatformImpl;
UniquePtr< TempResourcePool >    RenderCore::ourTempResourcePool;
UniquePtr< FileWatcher >         RenderCore::ourShaderFileWatcher;
UniquePtr< ShaderCompiler >      RenderCore::ourShaderCompiler;

TextureSamplerHandle RenderCore::ourLinearClampSampler;

DepthStencilStateHandle RenderCore::ourDefaultDepthStencilState;
BlendStateHandle        RenderCore::ourDefaultBlendState;
TextureHandle           RenderCore::ourDefaultDiffuseTexture;
TextureHandle           RenderCore::ourDefaultNormalTexture;
TextureHandle           RenderCore::ourDefaultSpecularTexture;
TextureHandle           RenderCore::ourMissingTexture;
ShaderPipelineHandle    RenderCore::ourMipDownsampleShader;

eastl::hash_map< uint64, TextureViewHandle > RenderCore::ourTexturePathCache;

ResourcePool< Texture, 2048 >     RenderCore::ourTexturePool;
ResourcePool< GpuBuffer, 2048 >   RenderCore::ourBufferPool;
ResourcePool< TextureView, 2048 > RenderCore::ourTextureViewPool;
ResourcePool< GpuBufferView, 2048 > RenderCore::ourBufferViewPool;
ResourcePool< Shader, 1024, ShaderDesc >                             RenderCore::ourShaderPool;
ResourcePool< ShaderPipeline, 1024, ShaderPipelineDesc >             RenderCore::ourShaderPipelinePool;
ResourcePool< BlendState, 1024, BlendStateProperties >               RenderCore::ourBlendStatePool;
ResourcePool< DepthStencilState, 1024, DepthStencilStateProperties > RenderCore::ourDepthStencilStatePool;
ResourcePool< TextureSampler, 1024, TextureSamplerProperties >       RenderCore::ourSamplerPool;
ResourcePool< VertexInputLayout, 1024, VertexInputLayoutProperties > RenderCore::ourVertexInputLayoutPool;
ResourcePool< RtAccelerationStructure, 1024 >                        RenderCore::ourRtAccelerationStructurePool;
ResourcePool< RtPipelineState, 1024, RtPipelineStateProperties >     RenderCore::ourRtPipelineStatePool;
ResourcePool< RtShaderBindingTable, 1024 >                           RenderCore::ourRtShaderBindingTablePool;
ResourcePool< RenderOutput, 1024 >                                   RenderCore::ourRenderOutputPool;

eastl::hash_map< eastl::string, eastl::vector< eastl::string > > RenderCore::ourShaderIncludeHeaderToShaderPaths;

eastl::vector< UniquePtr< GpuRingBuffer > >                      RenderCore::ourRingBufferPool;
eastl::fixed_list< GpuRingBuffer *, 128 >                        RenderCore::ourAvailableRingBuffers;
eastl::fixed_list< eastl::pair< uint64, GpuRingBuffer * >, 128 > RenderCore::ourUsedRingBuffers;

eastl::fixed_vector< UniquePtr< GpuReadbackBuffer >, 64 > RenderCore::ourReadbackBuffers;

UniquePtr< CommandQueue > RenderCore::ourCommandQueues[ ( uint ) CommandListType::NUM ];

StaticCircularArray< uint64, RenderCore::NUM_QUEUED_FRAMES > RenderCore::ourQueuedFrameDoneFences;
StaticCircularArray< eastl::pair< uint64, uint64 >, 256 >    RenderCore::ourLastFrameDoneFences;

UniquePtr< GpuQueryHeap > RenderCore::ourQueryHeaps[ NUM_QUEUED_FRAMES ][ ( uint ) GpuQueryType::NUM ];
uint                      RenderCore::ourCurrQueryHeapIdx = 0;

eastl::fixed_vector< eastl::pair< uint, uint >, 64 > RenderCore::ourUsedQueryRanges[ ( uint ) GpuQueryType::NUM ];

UniquePtr< GpuBuffer > RenderCore::ourQueryBuffers[ NUM_QUERY_BUFFERS ][ ( uint ) GpuQueryType::NUM ];
uint64                 RenderCore::ourQueryBufferFrames[ NUM_QUERY_BUFFERS ] = { UINT64_MAX };
uint                   RenderCore::ourCurrQueryBufferIdx = 0u;

const uint8 * RenderCore::ourMappedQueryBufferData[ ( uint ) GpuQueryType::NUM ] = { nullptr };
uint          RenderCore::ourMappedQueryBufferIdx[ ( uint ) GpuQueryType::NUM ] = { 0u };

eastl::vector< eastl::string > RenderCore::ourChangedShaderFiles;
//---------------------------------------------------------------------------//
bool RenderCore::IsInitialized() {
  return ourPlatformImpl != nullptr && ourPlatformImpl->IsInitialized();
}

TextureHandle RenderCore::GetDefaultDiffuseTexture() {
  return ourDefaultDiffuseTexture;
}

TextureHandle RenderCore::GetDefaultNormalTexture() {
  return ourDefaultNormalTexture;
}

TextureHandle RenderCore::GetDefaultMaterialTexture() {
  return ourDefaultSpecularTexture;
}

const ShaderCompiler * RenderCore::GetShaderCompiler() {
  return ourShaderCompiler.get();
}
//---------------------------------------------------------------------------//
const ShaderPipeline * RenderCore::GetMipDownsampleShader() {
  return GetShaderPipeline( ourMipDownsampleShader );
}
//---------------------------------------------------------------------------//
TextureViewHandle RenderCore::GetTextureByPath( const char * aPath, uint someFlags ) {
  eastl::string texPathAbs = aPath;
  eastl::string texPathRel = aPath;
  if ( !Path::IsPathAbsolute( texPathAbs ) )
    texPathAbs = Path::GetAbsolutePath( texPathAbs );
  else
    texPathRel = Path::GetRelativePath( texPathAbs );

  uint64 texPathRelHash = MathUtil::Hash( texPathRel );
  MathUtil::hash_combine( texPathRelHash, ( ( uint64 ) someFlags & Assets::SHADER_WRITABLE ) );

  auto it = ourTexturePathCache.find( texPathRelHash );
  if ( it != ourTexturePathCache.end() )
    return it->second;

  return TextureViewHandle{};
}
//---------------------------------------------------------------------------//
TextureViewHandle RenderCore::LoadTexture( const char * aPath, uint someLoadFlags ) {
  if ( strlen( aPath ) == 0 )
    return TextureViewHandle{};

  eastl::string texPathAbs = aPath;
  eastl::string texPathRel = aPath;
  if ( !Path::IsPathAbsolute( texPathAbs ) )
    texPathAbs = Path::GetAbsolutePath( texPathAbs );
  else
    texPathRel = Path::GetRelativePath( texPathAbs );

  if ( ( someLoadFlags & Assets::NO_MEM_CACHE ) == 0 ) {
    TextureViewHandle cached = GetTextureByPath( texPathRel.c_str(), someLoadFlags );
    if ( cached.IsValid() && GetTextureView( cached )->GetProperties().myIsShaderWritable ==
                                 ( ( someLoadFlags & Assets::SHADER_WRITABLE ) != 0 ) )
      return cached;
  }

  uint64 texPathRelHash = MathUtil::Hash( texPathRel );
  MathUtil::hash_combine( texPathRelHash, ( ( uint64 ) someLoadFlags & Assets::SHADER_WRITABLE ) );

  ImageData image;
  if ( !ImageLoader::Load( texPathAbs.c_str(), someLoadFlags, image ) ) {
    LOG_ERROR( "Failed to load texture at path %s", texPathAbs.c_str() );
    return TextureViewHandle{};
  }

  TextureHandle texHandle = CreateTexture( image.myProperties, texPathRel.c_str(), image.myData.mySubDatas.data(),
                                           ( uint ) image.myData.mySubDatas.size() );
  if ( !texHandle.IsValid() ) {
    LOG_ERROR( "Failed to create loaded texture at path %s", texPathAbs.c_str() );
    return TextureViewHandle{};
  }

  if ( GetTexture( texHandle )->GetProperties().myNumMipLevels == 1 &&
       ( someLoadFlags & Assets::NO_MIP_GENERATION ) == 0 )
    ComputeMipmaps( texHandle );

  Texture *             tex = GetTexture( texHandle );
  TextureViewProperties viewProps;
  viewProps.mySubresourceRange = tex->mySubresources;
  viewProps.myFormat = tex->GetProperties().myFormat;
  TextureViewHandle texViewHandle = CreateTextureView( tex, viewProps, aPath );

  ourTexturePathCache[ texPathRelHash ] = texViewHandle;
  return texViewHandle;
}
//---------------------------------------------------------------------------//
void RenderCore::ComputeMipmaps( TextureHandle aTextureHandle ) {
  if ( !aTextureHandle.IsValid() )
    return;

  Texture *                 aTexture = GetTexture( aTextureHandle );
  const TextureProperties & texProps = aTexture->GetProperties();
  const uint                numMips = texProps.myNumMipLevels;

  TextureHandle textureHandle = aTextureHandle;
  if ( !texProps.myIsShaderWritable ) {
    TextureProperties props = texProps;
    props.myIsRenderTarget = false;
    props.myIsShaderWritable = true;
    textureHandle = CreateTexture( props, "Mipmap target UAV copy" );
  }
  Texture * texture = GetTexture( textureHandle );

  TextureResourceProperties tempTexProps;
  tempTexProps.myIsShaderWritable = true;
  tempTexProps.myIsRenderTarget = false;
  tempTexProps.myIsTexture = true;
  tempTexProps.myTextureProperties = texProps;
  tempTexProps.myTextureProperties.myWidth = texProps.myWidth / 2;
  tempTexProps.myTextureProperties.myHeight = texProps.myHeight / 2;
  tempTexProps.myTextureProperties.myNumMipLevels = 1u;
  TempTextureResource tempTexture = AllocateTempTexture( tempTexProps, 0u, "Temp mipmapping texture" );

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
    readViews[ mip ] = CreateTextureView( texture, props );

    props.myIsShaderWritable = true;
    props.myFormat = DataFormatInfo::GetNonSRGBformat( props.myFormat );
    writeViews[ mip ] = CreateTextureView( texture, props );
  }

  CommandList * ctx = BeginCommandList( CommandListType::Graphics );
  if ( textureHandle != aTextureHandle )
    ctx->CopyTexture( texture, SubresourceLocation( 0 ), aTexture, SubresourceLocation( 0 ) );

  ctx->SetShaderPipeline( GetShaderPipeline( ourMipDownsampleShader ) );

  struct CBuffer {
    glm::int2 mySrcTextureSize;
    int       myIsSRGB;
    int       mySrcTextureIdx;
    int       myDstTextureIdx;
  } cBuffer;

  const DataFormatInfo & formatInfo = DataFormatInfo::GetFormatInfo( texProps.myFormat );
  cBuffer.myIsSRGB = formatInfo.mySRGB ? 1 : 0;

  const glm::int2 fullSize( texProps.myWidth, texProps.myHeight );
  for ( uint mip = 1u; mip < numMips; ++mip ) {
    const glm::int2 srcSize = fullSize >> ( int ) ( mip - 1 );
    const glm::int2 dstSize = fullSize >> ( int ) mip;

    TextureView * readView = GetTextureView( readViews[ mip - 1 ] );
    TextureView * writeView = GetTextureView( writeViews[ mip ] );
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

  ExecuteAndFreeCommandList( ctx, SyncMode::BLOCKING );
}
//---------------------------------------------------------------------------//
uint RenderCore::GetNumDescriptors( GlobalResourceType aType, const RenderPlatformProperties & someProperties ) {
  switch ( aType ) {
    case GLOBAL_RESOURCE_TEXTURE_1D:
    case GLOBAL_RESOURCE_TEXTURE_1D_UINT:
    case GLOBAL_RESOURCE_TEXTURE_1D_INT:
    case GLOBAL_RESOURCE_RWTEXTURE_1D:
    case GLOBAL_RESOURCE_RWTEXTURE_1D_UINT:
    case GLOBAL_RESOURCE_RWTEXTURE_1D_INT:
      return someProperties.myNumGlobalTextures1D;
    case GLOBAL_RESOURCE_TEXTURE_2D:
    case GLOBAL_RESOURCE_TEXTURE_2D_UINT:
    case GLOBAL_RESOURCE_TEXTURE_2D_INT:
    case GLOBAL_RESOURCE_RWTEXTURE_2D:
    case GLOBAL_RESOURCE_RWTEXTURE_2D_UINT:
    case GLOBAL_RESOURCE_RWTEXTURE_2D_INT:
      return someProperties.myNumGlobalTextures2D;
    case GLOBAL_RESOURCE_TEXTURE_3D:
    case GLOBAL_RESOURCE_TEXTURE_3D_UINT:
    case GLOBAL_RESOURCE_TEXTURE_3D_INT:
    case GLOBAL_RESOURCE_RWTEXTURE_3D:
    case GLOBAL_RESOURCE_RWTEXTURE_3D_UINT:
    case GLOBAL_RESOURCE_RWTEXTURE_3D_INT:
      return someProperties.myNumGlobalTextures3D;
    case GLOBAL_RESOURCE_TEXTURE_CUBE:
    case GLOBAL_RESOURCE_TEXTURE_CUBE_UINT:
    case GLOBAL_RESOURCE_TEXTURE_CUBE_INT:
      return someProperties.myNumGlobalTexturesCube;
    case GLOBAL_RESOURCE_BUFFER:
    case GLOBAL_RESOURCE_RWBUFFER:
    case GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE:
      return someProperties.myNumGlobalBuffers;
    case GLOBAL_RESOURCE_SAMPLER:
      return someProperties.myNumGlobalSamplers;
    default:
      assert( false );
      return 0;
  }
}
//---------------------------------------------------------------------------//
void RenderCore::Init( const RenderPlatformProperties & someProperties, const SharedPtr< Time > & aTimeClock ) {
  Init_0_Platform( someProperties );
  Init_1_Services( aTimeClock );
  Init_2_Resources();
}
//---------------------------------------------------------------------------//
void RenderCore::BeginFrame() {
  if ( ourQueuedFrameDoneFences.IsFull() ) {
    CommandQueue * graphicsQueue = GetCommandQueue( CommandListType::Graphics );
    graphicsQueue->WaitForFence( ourQueuedFrameDoneFences[ 0 ] );
    ourQueuedFrameDoneFences.RemoveFirstElement();
  }

  ourTempResourcePool->Reset();

  ourCurrQueryHeapIdx = ( ourCurrQueryHeapIdx + 1 ) % NUM_QUEUED_FRAMES;
  for ( uint i = 0u; i < ( uint ) GpuQueryType::NUM; ++i ) {
    const uint64 lastUsedFrame = ourQueryHeaps[ ourCurrQueryHeapIdx ][ i ]->myLastUsedFrame;
    ASSERT( lastUsedFrame == UINT64_MAX || IsFrameDone( lastUsedFrame ) );
    ourQueryHeaps[ ourCurrQueryHeapIdx ][ i ]->Reset( Time::ourFrameIdx );
    ourUsedQueryRanges[ i ].clear();
  }

  ourCurrQueryBufferIdx = ( ourCurrQueryBufferIdx + 1 ) % NUM_QUERY_BUFFERS;
  ourQueryBufferFrames[ ourCurrQueryBufferIdx ] = Time::ourFrameIdx;

  UpdateChangedShaders();

  ourPlatformImpl->BeginFrame();

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
  if ( ourDebugLogResourceBarriers )
    LOG_DEBUG( "\n---Frame Begin---" );
#endif
}
//---------------------------------------------------------------------------//
void RenderCore::EndFrame() {
  for ( uint i = 0u; i < ( uint ) GpuQueryType::NUM; ++i )
    ASSERT( ourMappedQueryBufferData[ i ] == nullptr, "Open query readback detected at end of frame" );

  ResolveUsedQueryData();

  CommandQueue * graphicsQueue = GetCommandQueue( CommandListType::Graphics );
  const uint64   completedFrameFence = graphicsQueue->GetLastRequestedFenceVal();

  ASSERT( !ourQueuedFrameDoneFences.IsFull() );
  ourQueuedFrameDoneFences.Add( completedFrameFence );

  if ( ourLastFrameDoneFences.IsFull() )
    ourLastFrameDoneFences.RemoveFirstElement();
  ourLastFrameDoneFences.Add( { Time::ourFrameIdx, completedFrameFence } );

  ourPlatformImpl->EndFrame();
}
//---------------------------------------------------------------------------//
void RenderCore::Shutdown() {
  Shutdown_0_Resources();
  Shutdown_1_Services();
  Shutdown_2_Platform();
}
//---------------------------------------------------------------------------//
const char * RenderCore::CommandListTypeToString( CommandListType aType ) {
  switch ( aType ) {
    case CommandListType::Graphics:
      return "Graphics";
    case CommandListType::Compute:
      return "Compute";
    case CommandListType::DMA:
      return "Copy";
    default:
      ASSERT( false );
      return "";
  }
}
//---------------------------------------------------------------------------//
CommandListType RenderCore::ResolveSupportedCommandListType( CommandListType aType ) {
  const RenderPlatformCaps & caps = GetPlatformCaps();

  if ( aType == CommandListType::Graphics || ( aType == CommandListType::Compute && !caps.myHasAsyncCompute ) ||
       ( aType == CommandListType::DMA && !caps.myHasAsyncCopy ) ) {
    return CommandListType::Graphics;
  }

  return aType;
}
//---------------------------------------------------------------------------//
RenderCore_PlatformDX12 * RenderCore::GetPlatformDX12() {
#if FANCY_ENABLE_DX12
  return GetPlatformType() == RenderPlatformType::DX12
             ? static_cast< RenderCore_PlatformDX12 * >( ourPlatformImpl.get() )
             : nullptr;
#else
  return nullptr;
#endif
}
//---------------------------------------------------------------------------//
GpuRingBuffer * RenderCore::AllocateRingBuffer( CpuMemoryAccessType aCpuAccess, uint someBindFlags,
                                                uint64 aNeededByteSize, const char * aName /*= nullptr*/ ) {
  ASSERT( aCpuAccess != CpuMemoryAccessType::NO_CPU_ACCESS,
          "Ring buffers are expected to be either readable or writable from CPU" );

  UpdateAvailableRingBuffers();

  for ( auto it = ourAvailableRingBuffers.begin(); it != ourAvailableRingBuffers.end(); ++it ) {
    GpuRingBuffer *             buffer = *it;
    const GpuBufferProperties & bufferProps = buffer->GetBuffer()->GetProperties();
    if ( buffer->GetBuffer()->GetByteSize() >= aNeededByteSize && bufferProps.myCpuAccess == aCpuAccess &&
         bufferProps.myBindFlags ==
             someBindFlags )  // We could also re-use buffers with more general bind flags, but since this method is
                              // likely to be called with the same arguments each frame, it might be beneficial to use
                              // the best match all the time
    {
      ourAvailableRingBuffers.erase( it );
      return buffer;
    }
  }

  // Create a new buffer
  UniquePtr< GpuRingBuffer > buf = eastl::make_unique< GpuRingBuffer >();

  GpuBufferProperties params;
  ASSERT( aNeededByteSize <= UINT_MAX, "Buffer size overflow. Consider making numElements 64 bit wide" );
  params.myNumElements = aNeededByteSize;
  params.myElementSizeBytes = 1u;
  params.myBindFlags = someBindFlags;
  params.myCpuAccess = aCpuAccess;

  buf->Create( params, aName );
  ourRingBufferPool.push_back( eastl::move( buf ) );

  return ourRingBufferPool.back().get();
}
//---------------------------------------------------------------------------//
void RenderCore::ReleaseRingBuffer( GpuRingBuffer * aBuffer, uint64 aFenceVal ) {
#if FANCY_RENDERER_USE_VALIDATION
  auto predicate = [ aBuffer ]( const eastl::pair< uint64, GpuRingBuffer * > & aPair ) {
    return aPair.second == aBuffer;
  };
  ASSERT( eastl::find_if( ourUsedRingBuffers.begin(), ourUsedRingBuffers.end(), predicate ) ==
          ourUsedRingBuffers.end() );
  ASSERT( eastl::find( ourAvailableRingBuffers.begin(), ourAvailableRingBuffers.end(), aBuffer ) ==
          ourAvailableRingBuffers.end() );
#endif

  aBuffer->Reset();
  ourUsedRingBuffers.push_back( eastl::make_pair( aFenceVal, aBuffer ) );
}
//---------------------------------------------------------------------------//
GpuBuffer * RenderCore::AllocateReadbackBuffer( uint64 aBlockSize, uint anOffsetAlignment,
                                                uint64 & anOffsetToBlockOut ) {
  for ( UniquePtr< GpuReadbackBuffer > & readbackBuffer : ourReadbackBuffers ) {
    uint64      offset;
    GpuBuffer * buffer = readbackBuffer->AllocateBlock( aBlockSize, anOffsetAlignment, offset );
    if ( buffer != nullptr ) {
      anOffsetToBlockOut = offset;
      return buffer;
    }
  }

  const uint64 newBufferSize = MathUtil::Align( aBlockSize, MathUtil::Align( 2 * SIZE_MB, anOffsetAlignment ) );
#if FANCY_HEAVY_DEBUG
  LOG_INFO( "Allocating new readback buffer of size %d", newBufferSize );
#endif  // FANCY_HEAVY_DEBUG
  ourReadbackBuffers.push_back( eastl::make_unique< GpuReadbackBuffer >( newBufferSize ) );

  uint64      offset;
  GpuBuffer * buffer = ourReadbackBuffers.back()->AllocateBlock( aBlockSize, anOffsetAlignment, offset );
  ASSERT( buffer != nullptr );

  anOffsetToBlockOut = offset;
  return buffer;
}
//---------------------------------------------------------------------------//
void RenderCore::FreeReadbackBuffer( GpuBuffer * aBuffer, uint64 aBlockSize, uint64 anOffsetToBlock ) {
  for ( auto it = ourReadbackBuffers.begin(); it != ourReadbackBuffers.end(); ++it ) {
    UniquePtr< GpuReadbackBuffer > & readbackBuffer = *it;
    if ( readbackBuffer->FreeBlock( aBuffer, anOffsetToBlock, aBlockSize ) ) {
      if ( readbackBuffer->IsEmpty() && it != ourReadbackBuffers.begin() )  // Always keep one readback buffer around
      {
#if FANCY_HEAVY_DEBUG
        LOG_INFO( "Deleting readback buffer of size %d", readbackBuffer->GetFreeSize() );
#endif  // FANCY_HEAVY_DEBUG

        ourReadbackBuffers.erase( it );
      }

      return;
    }
  }

  ASSERT( false, "Readback-buffer allocation not found" );
}
//---------------------------------------------------------------------------//
TextureReadbackTask RenderCore::ReadbackTexture( Texture * aTexture, const SubresourceRange & aSubresourceRange,
                                                 CommandListType aCommandListType /*= CommandListType::Graphics*/ ) {
  const TextureProperties &  texProps = aTexture->GetProperties();
  const DataFormatInfo &     dataFormatInfo = DataFormatInfo::GetFormatInfo( texProps.myFormat );
  const RenderPlatformCaps & caps = GetPlatformCaps();

  uint64 * alignedSubresourceSizes =
      static_cast< uint64 * >( alloca( sizeof( uint64 ) * aSubresourceRange.GetNumSubresources() ) );

  uint64 requiredBufferSize = 0u;
  uint   i = 0u;
  for ( SubresourceIterator it = aSubresourceRange.Begin(), end = aSubresourceRange.End(); it != end; ++it ) {
    const SubresourceLocation & subResource = *it;

    uint width, height, depth;
    texProps.GetSize( subResource.myMipLevel, width, height, depth );

    const uint64 rowSize = MathUtil::Align(
        BITS_TO_BYTES( width * dataFormatInfo.myCopyableBitsPerPixelPerPlane[ subResource.myPlaneIndex ] ),
        caps.myTextureRowAlignment );
    const uint64 subresourceSize =
        MathUtil::Align( rowSize * height * depth, ( uint64 ) caps.myTextureSubresourceBufferAlignment );
    alignedSubresourceSizes[ i++ ] = subresourceSize;

    requiredBufferSize += subresourceSize;
  }

  uint64      offsetToReadbackBuffer;
  GpuBuffer * readbackBuffer =
      AllocateReadbackBuffer( requiredBufferSize, caps.myTextureSubresourceBufferAlignment, offsetToReadbackBuffer );
  ASSERT( readbackBuffer != nullptr );

  CommandList * ctx = BeginCommandList( aCommandListType );

  uint64 dstOffset = offsetToReadbackBuffer;
  i = 0u;
  for ( SubresourceIterator it = aSubresourceRange.Begin(), end = aSubresourceRange.End(); it != end; ++it ) {
    const SubresourceLocation & subResource = *it;
    ctx->CopyTextureToBuffer( readbackBuffer, dstOffset, aTexture, subResource );
    dstOffset += alignedSubresourceSizes[ i++ ];
  }

  const uint64 fence = ExecuteAndFreeCommandList( ctx );

  SharedPtr< ReadbackBufferAllocation > bufferAlloc( new ReadbackBufferAllocation );
  bufferAlloc->myBlockSize = requiredBufferSize;
  bufferAlloc->myOffsetToBlock = offsetToReadbackBuffer;
  bufferAlloc->myBuffer = readbackBuffer;

  return TextureReadbackTask( texProps, aSubresourceRange, bufferAlloc, fence );
}
//---------------------------------------------------------------------------//
ReadbackTask RenderCore::ReadbackBuffer( GpuBuffer * aBuffer, uint64 anOffset, uint64 aSize,
                                         CommandListType aCommandListType /*= CommandListType::Graphics*/ ) {
  uint64      offsetToReadbackBuffer;
  GpuBuffer * readbackBuffer = AllocateReadbackBuffer( aSize, 1u, offsetToReadbackBuffer );
  ASSERT( readbackBuffer != nullptr );

  CommandList * ctx = BeginCommandList( aCommandListType );

  ctx->CopyBuffer( readbackBuffer, offsetToReadbackBuffer, aBuffer, anOffset, aSize );

  const uint64 fence = ExecuteAndFreeCommandList( ctx );

  SharedPtr< ReadbackBufferAllocation > bufferAlloc( new ReadbackBufferAllocation );
  bufferAlloc->myBlockSize = aSize;
  bufferAlloc->myOffsetToBlock = offsetToReadbackBuffer;
  bufferAlloc->myBuffer = readbackBuffer;

  return ReadbackTask( bufferAlloc, fence );
}
//---------------------------------------------------------------------------//
void RenderCore::Init_0_Platform( const RenderPlatformProperties & someProperties ) {
  ASSERT( ourPlatformImpl == nullptr );

  const CommandLine * commandLine = CommandLine::GetInstance();

  ourPlatformImpl = eastl::make_unique< RenderCore_PlatformDX12 >( someProperties );

  ourCommandQueues[ ( uint ) CommandListType::Graphics ].reset(
      ourPlatformImpl->CreateCommandQueue( CommandListType::Graphics ) );
  if ( GetPlatformCaps().myHasAsyncCopy )
    ourCommandQueues[ ( uint ) CommandListType::Compute ].reset(
        ourPlatformImpl->CreateCommandQueue( CommandListType::Compute ) );

  // From here, resources can be created that depend on ourPlatformImpl
  ourPlatformImpl->InitInternalResources();
}
//---------------------------------------------------------------------------//
void RenderCore::Init_1_Services( const SharedPtr< Time > & aTimeClock ) {
  ASSERT( ourPlatformImpl != nullptr );

  ourShaderFileWatcher = eastl::make_unique< FileWatcher >( aTimeClock );
  std::function< void( const eastl::string & ) > onUpdatedFn( &RenderCore::OnShaderFileUpdated );
  ourShaderFileWatcher->myOnFileUpdated.Connect( onUpdatedFn );

  std::function< void( const eastl::string & ) > onDeletedFn( &RenderCore::OnShaderFileDeletedMoved );
  ourShaderFileWatcher->myOnFileDeletedMoved.Connect( onDeletedFn );

  ourShaderCompiler.reset( ourPlatformImpl->CreateShaderCompiler() );

  const CommandLine * cmdLine = CommandLine::GetInstance();
#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
  ourDebugLogResourceBarriers = cmdLine->HasArgument( "logBarriers" );
#endif
  ourDebugWaitAfterEachSubmit = cmdLine->HasArgument( "waitAfterSubmit" );
}
//---------------------------------------------------------------------------//
void RenderCore::Init_2_Resources() {
  ASSERT( ourPlatformImpl != nullptr );

  ourDefaultDepthStencilState = CreateDepthStencilState( DepthStencilStateProperties() );
  ourDefaultBlendState = CreateBlendState( BlendStateProperties() );

  // Linear clamp sampler
  {
    TextureSamplerProperties samplerProps;
    samplerProps.myMinFiltering = SamplerFilterMode::BILINEAR;
    samplerProps.myMagFiltering = SamplerFilterMode::BILINEAR;
    ourLinearClampSampler = RenderCore::CreateTextureSampler( samplerProps );
  }

  {
    TextureProperties props;
    props.myFormat = DataFormat::SRGB_8_A_8;
    props.myDimension = GpuResourceDimension::TEXTURE_2D;
    props.myHeight = 1u;
    props.myWidth = 1u;
    props.myPath = "default_diffuse";

    TextureSubData data( props );
    uint8          color[ 4 ] = { 0, 0, 0, 255 };
    data.myData = color;

    ourDefaultDiffuseTexture = CreateTexture( props, "Default_Diffuse", &data, 1 );

    props.myPath = "default_specular";
    ourDefaultSpecularTexture = CreateTexture( props, "Default_Specular", &data, 1 );
  }

  {
    TextureProperties props;
    props.myFormat = DataFormat::RGBA_8;
    props.myDimension = GpuResourceDimension::TEXTURE_2D;
    props.myHeight = 1u;
    props.myWidth = 1u;
    props.myPath = "default_normal";

    TextureSubData data( props );
    uint8          color[ 4 ] = { 128, 128, 128, 255 };
    data.myData = color;

    ourDefaultNormalTexture = CreateTexture( props, "Default_Normal", &data, 1 );
  }

  {
    ShaderPipelineDesc pipelineDesc;
    ShaderDesc &       shaderDesc = pipelineDesc.myShader[ ( uint ) ShaderStage::SHADERSTAGE_COMPUTE ];
    shaderDesc.myPath = "fancy/resources/shaders/Downsample.hlsl";
    shaderDesc.myShaderStage = ( uint ) ShaderStage::SHADERSTAGE_COMPUTE;
    shaderDesc.myMainFunction = "main";
    ourMipDownsampleShader = CreateShaderPipeline( pipelineDesc );
    ASSERT( ourMipDownsampleShader.IsValid() );
  }

  ourTempResourcePool.reset( new TempResourcePool );

  const uint numQueriesPerType[] = {
    4096,  // Timestamp
    2048   // Occlusion
  };
  static_assert( ARRAYSIZE( numQueriesPerType ) == ( uint ) GpuQueryType::NUM, "Missing values for numQueriesPerType" );

  for ( uint i = 0u; i < NUM_QUEUED_FRAMES; ++i ) {
    for ( uint queryType = 0u; queryType < ( uint ) GpuQueryType::NUM; ++queryType )
      ourQueryHeaps[ i ][ queryType ].reset(
          ourPlatformImpl->CreateQueryHeap( ( GpuQueryType ) queryType, numQueriesPerType[ queryType ] ) );
  }

  GpuBufferProperties bufferProps;
  bufferProps.myCpuAccess = CpuMemoryAccessType::CPU_READ;
  for ( uint i = 0u; i < NUM_QUERY_BUFFERS; ++i ) {
    for ( uint queryType = 0u; queryType < ( uint ) GpuQueryType::NUM; ++queryType ) {
      bufferProps.myElementSizeBytes = GetQueryTypeDataSize( ( GpuQueryType ) queryType );
      bufferProps.myNumElements = numQueriesPerType[ queryType ];
      eastl::string name( StaticString< 64 >( "QueryHeap %s", locGetQueryTypeName( ( GpuQueryType ) queryType ) ) );

      GpuBuffer * buffer = ourPlatformImpl->CreateBuffer();
      if ( buffer )
        buffer->Create( bufferProps, name.c_str() );
      ourQueryBuffers[ i ][ queryType ].reset( buffer );
    }
  }

  ourReadbackBuffers.push_back( eastl::make_unique< GpuReadbackBuffer >( 64 * SIZE_MB ) );
}
//---------------------------------------------------------------------------//
void RenderCore::Shutdown_0_Resources() {
  UpdateAvailableRingBuffers();
  ASSERT( ourRingBufferPool.size() == ourAvailableRingBuffers.size(), "There are still some ringbuffers in flight" );
  ourAvailableRingBuffers.clear();
  ourRingBufferPool.clear();

  // Delete all pooled resources (cached pools also clear their internal caches via DeleteAll)
  ourRtShaderBindingTablePool.DeleteAll();
  ourRtAccelerationStructurePool.DeleteAll();
  ourRtPipelineStatePool.DeleteAll();
  ourTextureViewPool.DeleteAll();
  ourBufferViewPool.DeleteAll();
  ourTexturePool.DeleteAll();
  ourBufferPool.DeleteAll();
  ourShaderPipelinePool.DeleteAll();
  ourShaderPool.DeleteAll();
  ourBlendStatePool.DeleteAll();
  ourDepthStencilStatePool.DeleteAll();
  ourSamplerPool.DeleteAll();
  ourVertexInputLayoutPool.DeleteAll();
  ourRenderOutputPool.DeleteAll();

  ourLinearClampSampler = TextureSamplerHandle();
  ourMipDownsampleShader = ShaderPipelineHandle();
  ourTexturePathCache.clear();

  for ( uint i = 0u; i < NUM_QUERY_BUFFERS; ++i )
    for ( uint queryType = 0u; queryType < ( uint ) GpuQueryType::NUM; ++queryType )
      ourQueryBuffers[ i ][ queryType ].reset();

  for ( uint i = 0u; i < NUM_QUEUED_FRAMES; ++i )
    for ( uint queryType = 0u; queryType < ( uint ) GpuQueryType::NUM; ++queryType )
      ourQueryHeaps[ i ][ queryType ].reset();

  ourReadbackBuffers.clear();
}
//---------------------------------------------------------------------------//
void RenderCore::Shutdown_1_Services() {
  ourShaderFileWatcher.reset();
}
//---------------------------------------------------------------------------//
void RenderCore::Shutdown_2_Platform() {
  for ( uint i = 0u; i < ( uint ) CommandListType::NUM; ++i )
    if ( ourCommandQueues[ i ] != nullptr )
      ourCommandQueues[ i ]->WaitForIdle();

  ourPlatformImpl->Shutdown();
  ourPlatformImpl.reset();
}
//---------------------------------------------------------------------------//
void RenderCore::UpdateAvailableRingBuffers() {
  auto it = ourUsedRingBuffers.begin();
  while ( it != ourUsedRingBuffers.end() ) {
    uint64          fence = it->first;
    GpuRingBuffer * buffer = it->second;

    CommandQueue * queue = GetCommandQueue( fence );
    if ( queue->IsFenceDone( fence ) ) {
      it = ourUsedRingBuffers.erase( it );
      ourAvailableRingBuffers.push_back( buffer );
    } else
      ++it;
  }
}
//---------------------------------------------------------------------------//
void RenderCore::ResolveUsedQueryData() {
  bool hasAnyQueryData = false;
  for ( eastl::fixed_vector< eastl::pair< uint, uint >, 64 > & queryRanges : ourUsedQueryRanges )
    hasAnyQueryData |= !queryRanges.empty();

  if ( !hasAnyQueryData )
    return;

  CommandList * commandList = BeginCommandList( CommandListType::Graphics );
  for ( uint queryType = 0u; queryType < ( uint ) GpuQueryType::NUM; ++queryType ) {
    if ( ourUsedQueryRanges[ queryType ].empty() )
      continue;

    const uint                  numUsedQueryRanges = ( uint ) ourUsedQueryRanges[ queryType ].size();
    eastl::pair< uint, uint > * mergedRanges =
        ( eastl::pair< uint, uint > * ) alloca( sizeof( eastl::pair< uint, uint > ) * numUsedQueryRanges );
    uint numUsedMergedRanges = 0u;

    eastl::pair< uint, uint > currMergedRange = ourUsedQueryRanges[ queryType ][ 0 ];
    for ( uint i = 1u; i < numUsedQueryRanges; ++i ) {
      const eastl::pair< uint, uint > & range = ourUsedQueryRanges[ queryType ][ i ];
      if ( range.first == currMergedRange.second ) {
        currMergedRange.second = range.second;
      } else {
        mergedRanges[ numUsedMergedRanges++ ] = currMergedRange;
        currMergedRange = range;
      }
    }
    mergedRanges[ numUsedMergedRanges++ ] = currMergedRange;

    const GpuQueryHeap * heap = ourQueryHeaps[ ourCurrQueryHeapIdx ][ queryType ].get();
    const GpuBuffer *    readbackBuffer = ourQueryBuffers[ ourCurrQueryBufferIdx ][ queryType ].get();
    const uint           queryDataSize = GetQueryTypeDataSize( ( GpuQueryType ) queryType );
    for ( uint i = 0u; i < numUsedMergedRanges; ++i ) {
      const eastl::pair< uint, uint > & mergedRange = mergedRanges[ i ];
      const uint                        numQueries = mergedRange.second - mergedRange.first;
      const uint64                      offsetInBuffer = mergedRange.first * queryDataSize;
      commandList->CopyQueryDataToBuffer( heap, readbackBuffer, mergedRange.first, numQueries, offsetInBuffer );
    }

    ExecuteAndFreeCommandList( commandList );
  }
}
//---------------------------------------------------------------------------//
void RenderCore::UpdateChangedShaders() {
  eastl::fixed_vector< Shader *, 8 > shadersToRecompile;
  for ( const eastl::string & shaderFile : ourChangedShaderFiles ) {
    const auto & shaderCache = ourShaderPool.GetCache();
    for ( const auto & entry : shaderCache ) {
      ShaderHandle h = entry.second;
      Shader * program = ourShaderPool.Get( h );

      const ShaderDesc & desc = program->GetDescription();
      eastl::string      actualShaderPath = Path::GetAbsolutePath( desc.myPath.c_str() );

      if ( actualShaderPath == shaderFile )
        shadersToRecompile.push_back( program );
    }
  }
  ourChangedShaderFiles.clear();

  for ( uint i = 0u; i < ( uint ) shadersToRecompile.size(); ++i ) {
    Shader * shader = shadersToRecompile[ i ];

    ShaderCompilerResult compiledOutput;
    if ( ourShaderCompiler->Compile( shader->GetDescription(), &compiledOutput ) )
      shader->SetFromCompilerOutput( compiledOutput );
    else
      LOG_WARNING( "Failed compiling shader %s", shader->GetDescription().myPath.c_str() );
  }

  // Check which pipelines need to be updated...
  eastl::fixed_vector< ShaderPipeline *, 8 > changedPipelines;
  const auto & pipelineCache = ourShaderPipelinePool.GetCache();
  for ( const auto & entry : pipelineCache ) {
    ShaderPipelineHandle h = entry.second;
    ShaderPipeline * pipeline = ourShaderPipelinePool.Get( h );

    for ( uint i = 0u; i < ( uint ) shadersToRecompile.size(); ++i ) {
      Shader *   changedShader = shadersToRecompile[ i ];
      const uint stage = static_cast< uint >( changedShader->myProperties.myShaderStage );
      if ( changedShader == pipeline->GetShader( stage ) ) {
        changedPipelines.push_back( pipeline );
        break;
      }
    }
  }

  for ( uint i = 0u; i < ( uint ) changedPipelines.size(); ++i ) {
    ShaderPipeline * pipeline = changedPipelines[ i ];
    pipeline->Recreate();
    ourOnShaderPipelineRecompiled( pipeline );
  }

  // Check RTPSOs
  eastl::fixed_vector< RtPipelineState *, 8 > changedRtPsos;
  for ( const Shader * shader : shadersToRecompile ) {
    if ( !IsRaytracingStage( shader->myProperties.myShaderStage ) )
      continue;

    const auto & rtPsoCache = ourRtPipelineStatePool.GetCache();
    for ( const auto & entry : rtPsoCache ) {
      RtPipelineStateHandle h = entry.second;
      RtPipelineState * rtPso = ourRtPipelineStatePool.Get( h );
      if ( rtPso->HasShader( shader ) )
        changedRtPsos.push_back( rtPso );
    }
  }

  for ( RtPipelineState * rtPso : changedRtPsos ) {
    rtPso->Recompile();
    ourOnRtPipelineStateRecompiled( rtPso );
  }
}
//---------------------------------------------------------------------------//
RenderOutputHandle RenderCore::CreateRenderOutput( void *                   aNativeInstanceHandle,
                                                   const WindowParameters & someWindowParams ) {
  RenderOutput * output = ourPlatformImpl->CreateRenderOutput( aNativeInstanceHandle, someWindowParams );
  return ourRenderOutputPool.Add( output );
}
//---------------------------------------------------------------------------//
ShaderHandle RenderCore::CreateShader( const ShaderDesc & aDesc ) {
  ShaderHandle cached = ourShaderPool.Get( aDesc );
  if ( cached.IsValid() )
    return cached;

  ShaderCompilerResult compilerOutput;
  if ( !ourShaderCompiler->Compile( aDesc, &compilerOutput ) )
    return ShaderHandle{};

  Shader * program = ourPlatformImpl->CreateShader();
  program->SetFromCompilerOutput( compilerOutput );

  const eastl::string actualShaderPath = Path::GetAbsolutePath( aDesc.myPath.c_str() );
  ourShaderFileWatcher->AddFileWatch( actualShaderPath );

  for ( const eastl::string & includeFile : compilerOutput.myIncludedFilePaths ) {
    ourShaderFileWatcher->AddFileWatch( includeFile );
    ourShaderIncludeHeaderToShaderPaths[ includeFile ].push_back( actualShaderPath );
  }

  return ourShaderPool.Add( program, aDesc );
}
//---------------------------------------------------------------------------//
ShaderPipelineHandle RenderCore::CreateShaderPipeline( const ShaderPipelineDesc & aDesc ) {
  ShaderPipelineHandle cached = ourShaderPipelinePool.Get( aDesc );
  if ( cached.IsValid() )
    return cached;

  Shader * pipelinePrograms[ ( uint ) ShaderStage::SHADERSTAGE_NUM ] = {};
  for ( uint i = 0u; i < ( uint ) ShaderStage::SHADERSTAGE_NUM; ++i ) {
    if ( !aDesc.myShader[ i ].myPath.empty() ) {
      ShaderHandle sh = CreateShader( aDesc.myShader[ i ] );
      pipelinePrograms[ i ] = sh.IsValid() ? ourShaderPool.Get( sh ) : nullptr;
    }
  }

  ShaderPipeline * pipeline = ourPlatformImpl->CreateShaderPipeline();
  pipeline->Create( pipelinePrograms );
  return ourShaderPipelinePool.Add( pipeline, aDesc );
}
//---------------------------------------------------------------------------//
ShaderPipelineHandle RenderCore::CreateVertexPixelShaderPipeline( const char * aShaderPath,
                                                                  const char * aMainVtxFunction,
                                                                  const char * aMainFragmentFunction,
                                                                  const char * someDefines ) {
  eastl::vector< eastl::string > defines;
  if ( someDefines )
    StringUtil::Tokenize( someDefines, ",", defines );

  ShaderPipelineDesc pipelineDesc;

  ShaderDesc * shaderDesc = &pipelineDesc.myShader[ ( uint ) ShaderStage::SHADERSTAGE_VERTEX ];
  shaderDesc->myPath = aShaderPath;
  shaderDesc->myMainFunction = aMainVtxFunction;
  for ( const eastl::string & str : defines )
    shaderDesc->myDefines.push_back( str );

  shaderDesc = &pipelineDesc.myShader[ ( uint ) ShaderStage::SHADERSTAGE_FRAGMENT ];
  shaderDesc->myPath = aShaderPath;
  shaderDesc->myMainFunction = aMainFragmentFunction;
  for ( const eastl::string & str : defines )
    shaderDesc->myDefines.push_back( str );

  return CreateShaderPipeline( pipelineDesc );
}
//---------------------------------------------------------------------------//
ShaderPipelineHandle RenderCore::CreateComputeShaderPipeline( const char * aShaderPath, const char * aMainFunction,
                                                              const char * someDefines ) {
  eastl::vector< eastl::string > defines;
  if ( someDefines )
    StringUtil::Tokenize( someDefines, ",", defines );

  ShaderPipelineDesc pipelineDesc;

  ShaderDesc * shaderDesc = &pipelineDesc.myShader[ ( uint ) ShaderStage::SHADERSTAGE_COMPUTE ];
  shaderDesc->myPath = aShaderPath;
  shaderDesc->myMainFunction = aMainFunction;
  for ( const eastl::string & str : defines )
    shaderDesc->myDefines.push_back( str );

  return CreateShaderPipeline( pipelineDesc );
}
//---------------------------------------------------------------------------//
ShaderHandle RenderCore::GetShader( uint64 aDescHash ) {
  const auto & cache = ourShaderPool.GetCache();
  auto it = cache.find( aDescHash );
  if ( it != cache.end() )
    return it->second;
  return ShaderHandle{};
}
//---------------------------------------------------------------------------//
ShaderPipelineHandle RenderCore::GetShaderPipeline( uint64 aDescHash ) {
  const auto & cache = ourShaderPipelinePool.GetCache();
  auto it = cache.find( aDescHash );
  if ( it != cache.end() )
    return it->second;
  return ShaderPipelineHandle{};
}
//---------------------------------------------------------------------------//
BlendStateHandle RenderCore::CreateBlendState( const BlendStateProperties & aProperties ) {
  BlendStateHandle cached = ourBlendStatePool.Get( aProperties );
  if ( cached.IsValid() )
    return cached;
  return ourBlendStatePool.Add( new BlendState( aProperties ), aProperties );
}
//---------------------------------------------------------------------------//
DepthStencilStateHandle RenderCore::CreateDepthStencilState( const DepthStencilStateProperties & aDesc ) {
  DepthStencilStateHandle cached = ourDepthStencilStatePool.Get( aDesc );
  if ( cached.IsValid() )
    return cached;
  return ourDepthStencilStatePool.Add( new DepthStencilState( aDesc ), aDesc );
}
//---------------------------------------------------------------------------//
TextureSamplerHandle RenderCore::CreateTextureSampler( const TextureSamplerProperties & someProperties ) {
  TextureSamplerHandle cached = ourSamplerPool.Get( someProperties );
  if ( cached.IsValid() )
    return cached;
  return ourSamplerPool.Add( ourPlatformImpl->CreateTextureSampler( someProperties ), someProperties );
}
//---------------------------------------------------------------------------//
VertexInputLayoutHandle RenderCore::CreateVertexInputLayout( const VertexInputLayoutProperties & aDesc ) {
  VertexInputLayoutHandle cached = ourVertexInputLayoutPool.Get( aDesc );
  if ( cached.IsValid() )
    return cached;
  return ourVertexInputLayoutPool.Add( new VertexInputLayout( aDesc ), aDesc );
}
//---------------------------------------------------------------------------//
BlendState * RenderCore::GetDefaultBlendState() {
  return ourBlendStatePool.Get( ourDefaultBlendState );
}
//---------------------------------------------------------------------------//
DepthStencilState * RenderCore::GetDefaultDepthStencilState() {
  return ourDepthStencilStatePool.Get( ourDefaultDepthStencilState );
}
//---------------------------------------------------------------------------//
RenderPlatformType RenderCore::GetPlatformType() {
  return ourPlatformImpl->GetType();
}
//---------------------------------------------------------------------------//
const RenderPlatformCaps & RenderCore::GetPlatformCaps() {
  return ourPlatformImpl->GetCaps();
}
//---------------------------------------------------------------------------//
RenderCore_Platform * RenderCore::GetPlatform() {
  return ourPlatformImpl.get();
}
//---------------------------------------------------------------------------//
TextureHandle RenderCore::CreateTexture( const TextureProperties & someProperties, const char * aName /*= nullptr*/,
                                         TextureSubData * someUploadDatas, uint aNumUploadDatas ) {
  Texture * tex = ourPlatformImpl->CreateTexture();
  if ( !tex )
    return TextureHandle{};

  tex->Create( someProperties, aName, someUploadDatas, aNumUploadDatas );
  if ( !tex->IsValid() ) {
    delete tex;
    return TextureHandle{};
  }
  return ourTexturePool.Add( tex );
}
//---------------------------------------------------------------------------//
GpuBufferHandle RenderCore::CreateBuffer( const GpuBufferProperties & someProperties, const char * aName /*= nullptr*/,
                                          const void * someInitialData /* = nullptr */ ) {
  GpuBuffer * buffer = ourPlatformImpl->CreateBuffer();
  if ( !buffer )
    return GpuBufferHandle{};

  buffer->Create( someProperties, aName, someInitialData );
  if ( !buffer->IsValid() ) {
    delete buffer;
    return GpuBufferHandle{};
  }
  return ourBufferPool.Add( buffer );
}
//---------------------------------------------------------------------------//
TextureViewHandle RenderCore::CreateTextureView( Texture * aTexture, const TextureViewProperties & someProperties,
                                                 const char * aName /*= nullptr*/ ) {
  const TextureProperties & texProps = aTexture->GetProperties();
  TextureViewProperties     viewProps = someProperties;
  viewProps.myFormat = viewProps.myFormat != DataFormat::UNKNOWN ? viewProps.myFormat : texProps.myFormat;
  viewProps.myDimension =
      viewProps.myDimension != GpuResourceDimension::UNKONWN ? viewProps.myDimension : texProps.myDimension;
  viewProps.mySubresourceRange.myNumMipLevels =
      glm::max( 1u, glm::min( viewProps.mySubresourceRange.myNumMipLevels, texProps.myNumMipLevels ) );
  viewProps.mySubresourceRange.myNumArrayIndices =
      glm::max( 1u, glm::min( viewProps.mySubresourceRange.myNumArrayIndices,
                              texProps.GetArraySize() - viewProps.mySubresourceRange.myFirstArrayIndex ) );
  viewProps.myZSize = glm::max( 1u, glm::min( viewProps.myZSize, texProps.GetDepthSize() - viewProps.myFirstZindex ) );

  const DataFormatInfo & formatInfo = DataFormatInfo::GetFormatInfo( viewProps.myFormat );
  ASSERT( viewProps.mySubresourceRange.myFirstPlane < formatInfo.myNumPlanes );
  ASSERT( !viewProps.myIsShaderWritable || !viewProps.myIsRenderTarget, "UAV and RTV are mutually exclusive" );
  TextureView * view = ourPlatformImpl->CreateTextureView( aTexture, viewProps, aName );
  return ourTextureViewPool.Add( view );
}
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
GpuBufferViewHandle RenderCore::CreateBufferView( GpuBuffer * aBuffer, GpuBufferViewProperties someProperties,
                                                  const char * aName /*=nullptr*/ ) {
  if ( someProperties.mySize == UINT64_MAX )
    someProperties.mySize = aBuffer->GetByteSize() - someProperties.myOffset;

  const DataFormat       format = someProperties.myFormat;
  const DataFormatInfo & formatInfo = DataFormatInfo::GetFormatInfo( format );

  ASSERT( aBuffer->GetByteSize() >= someProperties.myOffset + someProperties.mySize ||
              someProperties.myIsRtAccelerationStructure,
          "Invalid buffer range" );
  ASSERT( !someProperties.myIsStructured || someProperties.myStructureSize > 0u,
          "Structured buffer views need a valid structure size" );
  ASSERT( !someProperties.myIsStructured || !someProperties.myIsRaw,
          "Raw and structured buffer views are mutually exclusive" );
  ASSERT( !someProperties.myIsShaderWritable || aBuffer->GetProperties().myIsShaderWritable,
          "A shader-writable buffer view requires a shader-writable buffer" );
  ASSERT( !someProperties.myIsStructured || format == DataFormat::UNKNOWN,
          "Structured buffer views can't have a format" );
  ASSERT( !someProperties.myIsRaw || format == DataFormat::UNKNOWN || format == DataFormat::R_32UI,
          "Raw buffer views can't have a format other than R32" );
  ASSERT( !someProperties.myIsShaderWritable || !someProperties.myIsRtAccelerationStructure,
          "Rt acceleration structures can only be SRVs" );
  ASSERT( !someProperties.myIsStructured || !someProperties.myIsRtAccelerationStructure,
          "Rt acceleration structures can't be structured" );
  ASSERT( !someProperties.myIsRtAccelerationStructure || format == DataFormat::UNKNOWN,
          "Rt acceleration structures can't have formats" );

  GpuBufferView * view = ourPlatformImpl->CreateBufferView( aBuffer, someProperties, aName );
  return ourBufferViewPool.Add( view );
}
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
RtAccelerationStructureHandle
RenderCore::CreateRtBottomLevelAccelerationStructure( const RtAccelerationStructureGeometryData * someGeometries,
                                                      uint aNumGeometries, uint aSomeFlags, const char * aName ) {
  if ( !GetPlatformCaps().mySupportsRaytracing )
    return RtAccelerationStructureHandle{};

  RtAccelerationStructure * as =
      ourPlatformImpl->CreateRtBottomLevelAccelerationStructure( someGeometries, aNumGeometries, aSomeFlags, aName );
  return ourRtAccelerationStructurePool.Add( as );
}
//---------------------------------------------------------------------------//
RtAccelerationStructureHandle
RenderCore::CreateRtTopLevelAccelerationStructure( const RtAccelerationStructureInstanceData * someInstances,
                                                   uint aNumInstances, uint someFlags, const char * aName ) {
  if ( !GetPlatformCaps().mySupportsRaytracing )
    return RtAccelerationStructureHandle{};

  RtAccelerationStructure * as =
      ourPlatformImpl->CreateRtTopLevelAccelerationStructure( someInstances, aNumInstances, someFlags, aName );
  return ourRtAccelerationStructurePool.Add( as );
}
//---------------------------------------------------------------------------//
RtPipelineStateHandle RenderCore::CreateRtPipelineState( const RtPipelineStateProperties & someProps ) {
  if ( !GetPlatformCaps().mySupportsRaytracing )
    return RtPipelineStateHandle{};

  RtPipelineStateHandle cached = ourRtPipelineStatePool.Get( someProps );
  if ( cached.IsValid() )
    return cached;
  return ourRtPipelineStatePool.Add( ourPlatformImpl->CreateRtPipelineState( someProps ), someProps );
}
//---------------------------------------------------------------------------//
RtShaderBindingTableHandle RenderCore::CreateRtShaderTable( const RtShaderBindingTableProperties & someProps ) {
  if ( !GetPlatformCaps().mySupportsRaytracing )
    return RtShaderBindingTableHandle{};

  return ourRtShaderBindingTablePool.Add( new RtShaderBindingTable( someProps ) );
}
//---------------------------------------------------------------------------//
void RenderCore::DeleteTexture( TextureHandle aHandle ) {
  ourTexturePool.Delete( aHandle );
}
void RenderCore::DeleteBuffer( GpuBufferHandle aHandle ) {
  ourBufferPool.Delete( aHandle );
}
void RenderCore::DeleteTextureView( TextureViewHandle aHandle ) {
  ourTextureViewPool.Delete( aHandle );
}
void RenderCore::DeleteBufferView( GpuBufferViewHandle aHandle ) {
  ourBufferViewPool.Delete( aHandle );
}
void RenderCore::DeleteRtAccelerationStructure( RtAccelerationStructureHandle aHandle ) {
  ourRtAccelerationStructurePool.Delete( aHandle );
}
void RenderCore::DeleteRtPipelineState( RtPipelineStateHandle aHandle ) {
  ourRtPipelineStatePool.Delete( aHandle );
}
void RenderCore::DeleteRtShaderBindingTable( RtShaderBindingTableHandle aHandle ) {
  ourRtShaderBindingTablePool.Delete( aHandle );
}
void RenderCore::DeleteRenderOutput( RenderOutputHandle aHandle ) {
  ourRenderOutputPool.Delete( aHandle );
}
//---------------------------------------------------------------------------//
Texture * RenderCore::GetTexture( TextureHandle aHandle ) {
  return ourTexturePool.Get( aHandle );
}
GpuBuffer * RenderCore::GetBuffer( GpuBufferHandle aHandle ) {
  return ourBufferPool.Get( aHandle );
}
TextureView * RenderCore::GetTextureView( TextureViewHandle aHandle ) {
  return ourTextureViewPool.Get( aHandle );
}
GpuBufferView * RenderCore::GetBufferView( GpuBufferViewHandle aHandle ) {
  return ourBufferViewPool.Get( aHandle );
}
Shader * RenderCore::GetShader( ShaderHandle aHandle ) {
  return ourShaderPool.Get( aHandle );
}
ShaderPipeline * RenderCore::GetShaderPipeline( ShaderPipelineHandle aHandle ) {
  return ourShaderPipelinePool.Get( aHandle );
}
BlendState * RenderCore::GetBlendState( BlendStateHandle aHandle ) {
  return ourBlendStatePool.Get( aHandle );
}
DepthStencilState * RenderCore::GetDepthStencilState( DepthStencilStateHandle aHandle ) {
  return ourDepthStencilStatePool.Get( aHandle );
}
TextureSampler * RenderCore::GetTextureSampler( TextureSamplerHandle aHandle ) {
  return ourSamplerPool.Get( aHandle );
}
VertexInputLayout * RenderCore::GetVertexInputLayout( VertexInputLayoutHandle aHandle ) {
  return ourVertexInputLayoutPool.Get( aHandle );
}
RtAccelerationStructure * RenderCore::GetRtAccelerationStructure( RtAccelerationStructureHandle aHandle ) {
  return ourRtAccelerationStructurePool.Get( aHandle );
}
RtPipelineState * RenderCore::GetRtPipelineState( RtPipelineStateHandle aHandle ) {
  return ourRtPipelineStatePool.Get( aHandle );
}
RtShaderBindingTable * RenderCore::GetRtShaderBindingTable( RtShaderBindingTableHandle aHandle ) {
  return ourRtShaderBindingTablePool.Get( aHandle );
}
RenderOutput * RenderCore::GetRenderOutput( RenderOutputHandle aHandle ) {
  return ourRenderOutputPool.Get( aHandle );
}
//---------------------------------------------------------------------------//
uint RenderCore::GetQueryTypeDataSize( GpuQueryType aType ) {
  return ourPlatformImpl->GetQueryTypeDataSize( aType );
}
//---------------------------------------------------------------------------//
CommandList * RenderCore::BeginCommandList( CommandListType aType ) {
  ASSERT( aType == CommandListType::Graphics || aType == CommandListType::Compute,
          "CommandList type %d not implemented", ( uint ) aType );

  CommandQueue * queue = GetCommandQueue( aType );
  return queue->BeginCommandList();
}
//---------------------------------------------------------------------------//
uint64 RenderCore::ExecuteAndFreeCommandList( class CommandList * aCommandList, SyncMode aSyncMode ) {
  CommandListType type = aCommandList->GetType();
  ASSERT( type == CommandListType::Graphics || type == CommandListType::Compute, "CommandList type %d not implemented",
          ( uint ) type );

  ASSERT( aCommandList->IsOpen(), "CommandList is not open (already executed?)" );

  CommandQueue * queue = GetCommandQueue( aCommandList->GetType() );
  return queue->ExecuteAndFreeCommandList( aCommandList, aSyncMode );
}
//---------------------------------------------------------------------------//
uint64 RenderCore::ExecuteAndResetCommandList( CommandList * aCommandList, SyncMode aSyncMode ) {
  CommandListType type = aCommandList->GetType();
  ASSERT( type == CommandListType::Graphics || type == CommandListType::Compute, "CommandList type %d not implemented",
          ( uint ) type );

  ASSERT( aCommandList->IsOpen(), "CommandList is not open (already executed?)" );

  CommandQueue * queue = GetCommandQueue( aCommandList->GetType() );
  return queue->ExecuteAndResetCommandList( aCommandList, aSyncMode );
}
//---------------------------------------------------------------------------//
TempTextureResource RenderCore::AllocateTempTexture( const TextureResourceProperties & someProps, uint someFlags,
                                                     const char * aName ) {
  return ourTempResourcePool->AllocateTexture( someProps, someFlags, aName );
}
//---------------------------------------------------------------------------//
TempBufferResource RenderCore::AllocateTempBuffer( const GpuBufferResourceProperties & someProps, uint someFlags,
                                                   const char * aName ) {
  return ourTempResourcePool->AllocateBuffer( someProps, someFlags, aName );
}
//---------------------------------------------------------------------------//
uint RenderCore::AllocateQueryRange( GpuQueryType aType, uint aNumQueries ) {
  GpuQueryHeap * heap = ourQueryHeaps[ ourCurrQueryHeapIdx ][ ( uint ) aType ].get();
  return heap->Allocate( aNumQueries );
}
//---------------------------------------------------------------------------//
void RenderCore::FreeQueryRange( GpuQueryType aType, uint aFirstQuery, uint aNumQueries, uint aNumUsedQueries ) {
  GpuQueryHeap * heap = ourQueryHeaps[ ourCurrQueryHeapIdx ][ ( uint ) aType ].get();

  // Is this the last query-range that was allocated? Then deallocate the range in the storage again so it can be used
  // again
  if ( heap->myNextFreeQuery == aFirstQuery + aNumQueries )
    heap->myNextFreeQuery = aFirstQuery + aNumUsedQueries;

  if ( aNumUsedQueries > 0u )
    ourUsedQueryRanges[ ( uint ) aType ].push_back( { aFirstQuery, aFirstQuery + aNumUsedQueries } );
}
//---------------------------------------------------------------------------//
bool RenderCore::BeginQueryDataReadback( GpuQueryType aType, uint64 aFrameIdx,
                                         const uint8 ** aDataPtrOut /*= nullptr*/ ) {
  if ( !IsFrameDone( aFrameIdx ) )
    return false;

  const uint type = ( uint ) aType;
  if ( ourMappedQueryBufferData[ type ] != nullptr &&
       ourQueryBufferFrames[ ourMappedQueryBufferIdx[ type ] ] == aFrameIdx ) {
    if ( aDataPtrOut != nullptr )
      *aDataPtrOut = ourMappedQueryBufferData[ type ];

    return true;
  }

  int bufferIdx = -1;
  for ( uint i = 0u; bufferIdx < 0 && i < NUM_QUERY_BUFFERS; ++i ) {
    if ( ourQueryBufferFrames[ i ] == aFrameIdx )
      bufferIdx = i;
  }

  if ( bufferIdx < 0 )
    return false;

  GpuBuffer * buffer = ourQueryBuffers[ bufferIdx ][ type ].get();
  ourMappedQueryBufferData[ type ] = ( const uint8 * ) buffer->Map( GpuResourceMapMode::READ_UNSYNCHRONIZED );
  ASSERT( ourMappedQueryBufferData[ type ] != nullptr );
  ourMappedQueryBufferIdx[ type ] = ( uint ) bufferIdx;

  if ( aDataPtrOut != nullptr )
    *aDataPtrOut = ourMappedQueryBufferData[ type ];

  return true;
}
//---------------------------------------------------------------------------//
bool RenderCore::ReadQueryData( const GpuQuery & aQuery, uint8 * aData ) {
  const uint    type = ( uint ) aQuery.myType;
  const uint8 * mappedData = ourMappedQueryBufferData[ type ];
  if ( mappedData == nullptr )
    return false;

  const uint bufferIdx = ourMappedQueryBufferIdx[ type ];
  if ( aQuery.myFrame != ourQueryBufferFrames[ bufferIdx ] )
    return false;

  const uint  queryTypeDataSize = GetQueryTypeDataSize( ( GpuQueryType ) type );
  const uint8 byteOffset = aQuery.myIndexInHeap * queryTypeDataSize;
  ASSERT( ourQueryBuffers[ bufferIdx ][ type ]->GetByteSize() >= byteOffset + queryTypeDataSize );

  memcpy( aData, mappedData + byteOffset, ( size_t ) queryTypeDataSize );
  return true;
}
//---------------------------------------------------------------------------//
void RenderCore::EndQueryDataReadback( GpuQueryType aType ) {
  const uint type = ( uint ) aType;
  if ( ourMappedQueryBufferData[ type ] == nullptr )
    return;

  const uint bufferIdx = ourMappedQueryBufferIdx[ type ];
  ourQueryBuffers[ bufferIdx ][ type ]->Unmap( GpuResourceMapMode::READ_UNSYNCHRONIZED );

  ourMappedQueryBufferData[ type ] = nullptr;
  ourMappedQueryBufferIdx[ type ] = 0u;
}
//---------------------------------------------------------------------------//
float64 RenderCore::GetGpuTicksToMsFactor( CommandListType aCommandListType ) {
  return ourPlatformImpl->GetGpuTicksToMsFactor( aCommandListType );
}
//---------------------------------------------------------------------------//
bool RenderCore::IsFenceDone( uint64 aFenceVal ) {
  CommandQueue * queue = GetCommandQueue( aFenceVal );
  ASSERT( queue );

  return queue->IsFenceDone( aFenceVal );
}
//---------------------------------------------------------------------------//
bool RenderCore::IsFrameDone( uint64 aFrameIdx ) {
  if ( ourLastFrameDoneFences.IsEmpty() ||
       ourLastFrameDoneFences[ ourLastFrameDoneFences.Size() - 1 ].first < aFrameIdx )
    return false;

  CommandQueue * queue = GetCommandQueue( CommandListType::Graphics );
  for ( uint i = 0u, e = ourLastFrameDoneFences.Size(); i < e; ++i )
    if ( ourLastFrameDoneFences[ i ].first == aFrameIdx )
      return queue->IsFenceDone( ourLastFrameDoneFences[ i ].second );

  ASSERT( false );  // It should never reach this point
  return false;
}
//---------------------------------------------------------------------------//
void RenderCore::WaitForFrame( uint64 aFrameIdx ) {
  if ( ourLastFrameDoneFences.IsEmpty() ||
       ourLastFrameDoneFences[ ourLastFrameDoneFences.Size() - 1 ].first < aFrameIdx ) {
    WaitForIdle( CommandListType::Graphics );
    return;
  }

  if ( ourLastFrameDoneFences[ 0 ].first > aFrameIdx )
    return;

  CommandQueue * queue = GetCommandQueue( CommandListType::Graphics );
  for ( uint i = 0u, e = ourLastFrameDoneFences.Size(); i < e; ++i ) {
    if ( ourLastFrameDoneFences[ i ].first == aFrameIdx ) {
      WaitForFence( ourLastFrameDoneFences[ i ].second );
      break;
    }
  }
}
//---------------------------------------------------------------------------//
void RenderCore::WaitForFence( uint64 aFenceVal ) {
  CommandQueue * queue = GetCommandQueue( aFenceVal );
  ASSERT( queue );

  queue->WaitForFence( aFenceVal );
}
//---------------------------------------------------------------------------//
void RenderCore::WaitForIdle( CommandListType aType ) {
  CommandQueue * queue = GetCommandQueue( aType );
  if ( queue != nullptr )
    queue->WaitForIdle();
}
//---------------------------------------------------------------------------//
CommandQueue * RenderCore::GetCommandQueue( CommandListType aType ) {
  const CommandListType commandListType = ResolveSupportedCommandListType( aType );
  return ourCommandQueues[ ( uint ) commandListType ].get();
}
//---------------------------------------------------------------------------//
CommandQueue * RenderCore::GetCommandQueue( uint64 aFenceVal ) {
  CommandListType type = CommandQueue::GetCommandListType( aFenceVal );
  return GetCommandQueue( type );
}
//---------------------------------------------------------------------------//
void RenderCore::OnShaderFileUpdated( const eastl::string & aShaderFile ) {
  // Is this file an include-header? Then mark all the shader files its used in for recompilation
  auto it = ourShaderIncludeHeaderToShaderPaths.find( aShaderFile );
  if ( it != ourShaderIncludeHeaderToShaderPaths.end() ) {
    const eastl::vector< eastl::string > & shaderPaths = it->second;
    for ( const eastl::string & path : shaderPaths )
      ourChangedShaderFiles.push_back( path );
  } else {
    ourChangedShaderFiles.push_back( aShaderFile );
  }
}
//---------------------------------------------------------------------------//
void RenderCore::OnShaderFileDeletedMoved( const eastl::string & aShaderFile ) {}
//---------------------------------------------------------------------------//