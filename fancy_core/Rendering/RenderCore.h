#pragma once

#include "RendererPrerequisites.h"
#include "RenderEnums.h"
#include "Common/Slot.h"
#include "RenderCore_Platform.h"
#include "Common/Ptr.h"
#include "TempResources.h"
#include "GpuQuery.h"
#include "RtPipelineState.h"
#include "Common/CircularArray.h"
#include "TextureSampler.h"
#include "TextureReadbackTask.h"
#include "ResourceHandle.h"
#include "ResourcePool.h"
#include "GpuResource.h"
#include "GpuResourceView.h"

#include "EASTL/fixed_vector.h"
#include "EASTL/fixed_list.h"
#include "EASTL/hash_map.h"
#include "EASTL/string.h"

namespace Fancy {
  class Time;
  struct RtAccelerationStructureInstanceData;
  //---------------------------------------------------------------------------//
  class RtShaderBindingTable;
  struct RtShaderBindingTableProperties;
  class RtPipelineState;
  struct MaterialDesc;
  struct Material;
  struct MeshDesc;
  struct MeshPartData;
  struct VertexInputLayoutProperties;
  struct Mesh;
  struct ShaderPipelineDesc;
  struct DepthStencilStateProperties;
  struct BlendStateProperties;
  class RenderCore_PlatformDX12;
  class ShaderPipeline;
  class BlendState;
  class DepthStencilState;
  class FileWatcher;
  class GpuRingBuffer;
  class GpuReadbackBuffer;
  class Shader;
  struct ShaderDesc;
  class TempResourcePool;
  struct TextureResourceProperties;
  class GpuResource;
  struct GpuBufferResourceProperties;
  struct SubresourceLocation;
  struct GpuBufferProperties;
  struct TextureProperties;
  struct TextureSubData;
  class GpuQueryHeap;
  struct GpuQueryStorage;
  struct ReadbackBufferAllocation;
  class ReadbackTask;
  class TextureReadbackTask;
  struct VertexInputLayout;
  class Texture;
  struct SubResourceRange;
  class GpuResourceViewSet;
  class GpuResourceView;
  struct GpuResourceViewRange;
  struct RtAccelerationStructureProps;
  struct RtAccelerationStructureGeometryData;
  class RtAccelerationStructure;
  class RenderOutput;
  //---------------------------------------------------------------------------//
  class RenderCore {
  public:
    // Utility functions
    static uint GetNumDescriptors( GlobalResourceType aType, const RenderPlatformProperties & someProperties );

    enum Constants {
      NUM_QUEUED_FRAMES = 3u,
      QUERY_BUFFER_LIFETIME = 3u,

      NUM_QUERY_BUFFERS = NUM_QUEUED_FRAMES + QUERY_BUFFER_LIFETIME,
    };

    /// Init platform-independent stuff
    static void Init( const RenderPlatformProperties & someProperties, const SharedPtr< Time > & aTimeClock );
    static void BeginFrame();
    static void EndFrame();
    static void Shutdown();

    static const char * CommandListTypeToString( CommandListType aType );
    static CommandListType ResolveSupportedCommandListType( CommandListType aType );

    static bool IsInitialized();

    static TextureHandle GetDefaultDiffuseTexture();
    static TextureHandle GetDefaultNormalTexture();
    static TextureHandle GetDefaultMaterialTexture();
    static const ShaderCompiler * GetShaderCompiler();

    static ShaderHandle GetShader( uint64 aDescHash );
    static ShaderPipelineHandle GetShaderPipeline( uint64 aDescHash );

    static RenderOutputHandle CreateRenderOutput( void * aNativeInstanceHandle,
                                                  const WindowParameters & someWindowParams );
    static ShaderHandle CreateShader( const ShaderDesc & aDesc );
    static ShaderPipelineHandle CreateShaderPipeline( const ShaderPipelineDesc & aDesc );
    static ShaderPipelineHandle CreateVertexPixelShaderPipeline( const char * aShaderPath,
                                                                 const char * aMainVtxFunction = "main",
                                                                 const char * aMainFragmentFunction = "main",
                                                                 const char * someDefines = nullptr );
    static ShaderPipelineHandle CreateComputeShaderPipeline( const char * aShaderPath,
                                                             const char * aMainFunction = "main",
                                                             const char * someDefines = nullptr );
    static TextureHandle CreateTexture( const TextureProperties & someProperties, const char * aName = nullptr,
                                        TextureSubData * someUploadDatas = nullptr, uint aNumUploadDatas = 0u );
    static GpuBufferHandle CreateBuffer( const GpuBufferProperties & someProperties, const char * aName = nullptr,
                                         const void * someInitialData = nullptr );
    static TextureViewHandle CreateTextureView( Texture * aTexture, const TextureViewProperties & someProperties,
                                                const char * aName = nullptr );
    static GpuBufferViewHandle CreateBufferView( GpuBuffer * aBuffer, GpuBufferViewProperties someProperties,
                                                 const char * aName = nullptr );
    static RtAccelerationStructureHandle
    CreateRtBottomLevelAccelerationStructure( const RtAccelerationStructureGeometryData * someGeometries,
                                              uint aNumGeometries, uint aSomeFlags = 0, const char * aName = nullptr );
    static RtAccelerationStructureHandle
    CreateRtTopLevelAccelerationStructure( const RtAccelerationStructureInstanceData * someInstances,
                                           uint aNumInstances, uint someFlags = 0, const char * aName = nullptr );
    static RtPipelineStateHandle CreateRtPipelineState( const RtPipelineStateProperties & someProps );
    static RtShaderBindingTableHandle CreateRtShaderTable( const RtShaderBindingTableProperties & someProps );
    static uint GetQueryTypeDataSize( GpuQueryType aType );

    static BlendStateHandle CreateBlendState( const BlendStateProperties & aProperties );
    static DepthStencilStateHandle CreateDepthStencilState( const DepthStencilStateProperties & aDesc );
    static TextureSamplerHandle CreateTextureSampler( const TextureSamplerProperties & someProperties );
    static VertexInputLayoutHandle CreateVertexInputLayout( const VertexInputLayoutProperties & aDesc );
    static BlendState * GetDefaultBlendState();
    static DepthStencilState * GetDefaultDepthStencilState();

    // Delete methods - release ownership back to pool
    static void DeleteTexture( TextureHandle aHandle );
    static void DeleteBuffer( GpuBufferHandle aHandle );
    static void DeleteTextureView( TextureViewHandle aHandle );
    static void DeleteBufferView( GpuBufferViewHandle aHandle );
    static void DeleteRtAccelerationStructure( RtAccelerationStructureHandle aHandle );
    static void DeleteRtPipelineState( RtPipelineStateHandle aHandle );
    static void DeleteRtShaderBindingTable( RtShaderBindingTableHandle aHandle );
    static void DeleteRenderOutput( RenderOutputHandle aHandle );

    // Get methods - resolve handle to raw pointer (valid until deleted)
    static Texture * GetTexture( TextureHandle aHandle );
    static GpuBuffer * GetBuffer( GpuBufferHandle aHandle );
    static TextureView * GetTextureView( TextureViewHandle aHandle );
    static GpuBufferView * GetBufferView( GpuBufferViewHandle aHandle );
    static Shader * GetShader( ShaderHandle aHandle );
    static ShaderPipeline * GetShaderPipeline( ShaderPipelineHandle aHandle );
    static BlendState * GetBlendState( BlendStateHandle aHandle );
    static DepthStencilState * GetDepthStencilState( DepthStencilStateHandle aHandle );
    static TextureSampler * GetTextureSampler( TextureSamplerHandle aHandle );
    static VertexInputLayout * GetVertexInputLayout( VertexInputLayoutHandle aHandle );
    static RtAccelerationStructure * GetRtAccelerationStructure( RtAccelerationStructureHandle aHandle );
    static RtPipelineState * GetRtPipelineState( RtPipelineStateHandle aHandle );
    static RtShaderBindingTable * GetRtShaderBindingTable( RtShaderBindingTableHandle aHandle );
    static RenderOutput * GetRenderOutput( RenderOutputHandle aHandle );

    static RenderPlatformType GetPlatformType();
    static const RenderPlatformCaps & GetPlatformCaps();
    static RenderCore_Platform * GetPlatform();
    static RenderCore_PlatformDX12 * GetPlatformDX12();

    static GpuRingBuffer * AllocateRingBuffer( CpuMemoryAccessType aCpuAccess, uint someBindFlags,
                                               uint64 aNeededByteSize, const char * aName = nullptr );
    static void ReleaseRingBuffer( GpuRingBuffer * aBuffer, uint64 aFenceVal );

    static GpuBuffer * AllocateReadbackBuffer( uint64 aBlockSize, uint anOffsetAlignment, uint64 & anOffsetToBlockOut );
    static void FreeReadbackBuffer( GpuBuffer * aBuffer, uint64 aBlockSize, uint64 anOffsetToBlock );

    static TextureReadbackTask ReadbackTexture( Texture * aTexture, const SubresourceRange & aSubresourceRange,
                                                CommandListType aCommandListType = CommandListType::Graphics );
    static ReadbackTask ReadbackBuffer( GpuBuffer * aBuffer, uint64 anOffset, uint64 aSize,
                                        CommandListType aCommandListType = CommandListType::Graphics );

    static CommandList * BeginCommandList( CommandListType aType );
    static uint64 ExecuteAndFreeCommandList( CommandList * aCommandList, SyncMode aSyncMode = SyncMode::ASYNC );
    static uint64 ExecuteAndResetCommandList( CommandList * aCommandList, SyncMode aSyncMode = SyncMode::ASYNC );

    static TempTextureResource AllocateTempTexture( const TextureResourceProperties & someProps, uint someFlags,
                                                    const char * aName );
    static TempBufferResource AllocateTempBuffer( const GpuBufferResourceProperties & someProps, uint someFlags,
                                                  const char * aName );

    static GpuQueryHeap * GetQueryHeap( GpuQueryType aType ) {
      return ourQueryHeaps[ ourCurrQueryHeapIdx ][ ( uint ) aType ].get();
    }
    static uint AllocateQueryRange( GpuQueryType aType, uint aNumQueries );
    static void FreeQueryRange( GpuQueryType aType, uint aFirstQuery, uint aNumQueries, uint aNumUsedQueries );
    static bool BeginQueryDataReadback( GpuQueryType aType, uint64 aFrameIdx, const uint8 ** aDataPtrOut = nullptr );
    static bool ReadQueryData( const GpuQuery & aQuery, uint8 * aData );
    static void EndQueryDataReadback( GpuQueryType aType );
    static float64 GetGpuTicksToMsFactor( CommandListType aCommandListType );

    static bool IsFenceDone( uint64 aFenceVal );
    static bool IsFrameDone( uint64 aFrameIdx );
    static void WaitForFrame( uint64 aFrameIdx );
    static void WaitForFence( uint64 aFenceVal );
    static void WaitForIdle( CommandListType aType );

    static CommandQueue * GetCommandQueue( CommandListType aType );
    static CommandQueue * GetCommandQueue( uint64 aFenceVal );

    static Slot< void( const ShaderPipeline * ) > ourOnShaderPipelineRecompiled;
    static Slot< void( const RtPipelineState * ) > ourOnRtPipelineStateRecompiled;
    static bool ourDebugLogResourceBarriers;
    static bool ourDebugWaitAfterEachSubmit;

    static TextureSamplerHandle ourLinearClampSampler;

  protected:
    RenderCore() = default;

    static void Init_0_Platform( const RenderPlatformProperties & someProperties );
    static void Init_1_Services( const SharedPtr< Time > & aTimeClock );
    static void Init_2_Resources();

    static void Shutdown_0_Resources();
    static void Shutdown_1_Services();
    static void Shutdown_2_Platform();

    static void UpdateAvailableRingBuffers();
    static void ResolveUsedQueryData();
    static void UpdateChangedShaders();

    static void OnShaderFileUpdated( const eastl::string & aShaderFile );
    static void OnShaderFileDeletedMoved( const eastl::string & aShaderFile );

    static UniquePtr< RenderCore_Platform > ourPlatformImpl;
    static UniquePtr< TempResourcePool > ourTempResourcePool;
    static UniquePtr< ShaderCompiler > ourShaderCompiler;
    static UniquePtr< FileWatcher > ourShaderFileWatcher;

    // Resource pools
    static ResourcePool< Texture, 2048 > ourTexturePool;
    static ResourcePool< GpuBuffer, 2048 > ourBufferPool;
    static ResourcePool< TextureView, 2048 > ourTextureViewPool;
    static ResourcePool< GpuBufferView, 2048 > ourBufferViewPool;
    static ResourcePool< Shader, 1024 > ourShaderPool;
    static ResourcePool< ShaderPipeline, 1024 > ourShaderPipelinePool;
    static ResourcePool< BlendState, 1024 > ourBlendStatePool;
    static ResourcePool< DepthStencilState, 1024 > ourDepthStencilStatePool;
    static ResourcePool< TextureSampler, 1024 > ourSamplerPool;
    static ResourcePool< VertexInputLayout, 1024 > ourVertexInputLayoutPool;
    static ResourcePool< RtAccelerationStructure, 1024 > ourRtAccelerationStructurePool;
    static ResourcePool< RtPipelineState, 1024 > ourRtPipelineStatePool;
    static ResourcePool< RtShaderBindingTable, 1024 > ourRtShaderBindingTablePool;
    static ResourcePool< RenderOutput, 1024 > ourRenderOutputPool;

    // Handles to default/cached resources
    static DepthStencilStateHandle ourDefaultDepthStencilState;
    static BlendStateHandle ourDefaultBlendState;
    static TextureHandle ourDefaultDiffuseTexture;
    static TextureHandle ourDefaultNormalTexture;
    static TextureHandle ourDefaultSpecularTexture;
    static TextureHandle ourMissingTexture;

    static eastl::hash_map< eastl::string, eastl::vector< eastl::string > > ourShaderIncludeHeaderToShaderPaths;
    static eastl::hash_map< uint64, ShaderHandle > ourShaderCache;
    static eastl::hash_map< uint64, ShaderPipelineHandle > ourShaderPipelineCache;
    static eastl::hash_map< uint64, BlendStateHandle > ourBlendStateCache;
    static eastl::hash_map< uint64, DepthStencilStateHandle > ourDepthStencilStateCache;
    static eastl::hash_map< uint64, TextureSamplerHandle > ourSamplerCache;
    static eastl::hash_map< uint64, VertexInputLayoutHandle > ourVertexInputLayoutCache;
    static eastl::hash_map< uint64, RtPipelineStateHandle > ourRtPipelineStateCache;

    static eastl::vector< UniquePtr< GpuRingBuffer > > ourRingBufferPool;
    static eastl::fixed_list< GpuRingBuffer *, 128 > ourAvailableRingBuffers;
    static eastl::fixed_list< eastl::pair< uint64, GpuRingBuffer * >, 128 > ourUsedRingBuffers;

    static eastl::fixed_vector< UniquePtr< GpuReadbackBuffer >, 64 > ourReadbackBuffers;

    static UniquePtr< CommandQueue >
        ourCommandQueues[ ( uint ) CommandListType::NUM ];  // TODO: Move into RenderCore_Platform

    static StaticCircularArray< uint64, NUM_QUEUED_FRAMES > ourQueuedFrameDoneFences;
    static StaticCircularArray< eastl::pair< uint64, uint64 >, 256 > ourLastFrameDoneFences;

    static UniquePtr< GpuQueryHeap > ourQueryHeaps[ NUM_QUEUED_FRAMES ][ ( uint ) GpuQueryType::NUM ];
    static uint ourCurrQueryHeapIdx;

    static eastl::fixed_vector< eastl::pair< uint, uint >, 64 > ourUsedQueryRanges[ ( uint ) GpuQueryType::NUM ];

    static UniquePtr< GpuBuffer > ourQueryBuffers[ NUM_QUERY_BUFFERS ][ ( uint ) GpuQueryType::NUM ];
    static uint64 ourQueryBufferFrames[ NUM_QUERY_BUFFERS ];
    static uint ourCurrQueryBufferIdx;

    static const uint8 * ourMappedQueryBufferData[ ( uint ) GpuQueryType::NUM ];
    static uint ourMappedQueryBufferIdx[ ( uint ) GpuQueryType::NUM ];

    static eastl::vector< eastl::string > ourChangedShaderFiles;
    static eastl::vector< eastl::string > ourChangedShaderIncludeFiles;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy