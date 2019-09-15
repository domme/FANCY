#pragma once

#include "RendererPrerequisites.h"
#include "RenderEnums.h"
#include "Slot.h"
#include "RenderCore_Platform.h"
#include "DynamicArray.h"
#include "Ptr.h"
#include "TempResources.h"
#include "GpuQuery.h"
#include "CircularArray.h"
#include "FixedArray.h"

#include <map>
#include <list>

namespace Fancy {
//---------------------------------------------------------------------------//
  class Mesh;
  struct MeshDesc;
  struct MeshData;
  struct ShaderPipelineDesc;
  struct DepthStencilStateDesc;
  struct BlendStateDesc;
  class RenderCore_PlatformDX12;
  class RenderCore_PlatformVk;
  class ShaderPipeline;
  class BlendState;
  class DepthStencilState;
  class FileWatcher;
  class GpuRingBuffer;
  class Shader;
  struct ShaderDesc;
  class TempResourcePool;
  struct TextureResourceProperties;
  class GpuResource;
  struct GpuBufferResourceProperties;
  struct TextureSubLocation;
  struct GpuBufferProperties;
  struct TextureProperties;
  struct TextureSubData;
  class GpuQueryHeap;
  struct GpuQueryStorage;
//---------------------------------------------------------------------------//
  class RenderCore
  {
  public:
    enum Constants
    {
      NUM_QUEUED_FRAMES = 3u,
      QUERY_BUFFER_LIFETIME = 3u,

      NUM_QUERY_BUFFERS = NUM_QUEUED_FRAMES + QUERY_BUFFER_LIFETIME,
    };

    /// Init platform-independent stuff
    static void Init(RenderPlatformType aRenderingApi);
    static void BeginFrame();
    static void EndFrame();
    static void Shutdown();

    static const char* ResourceUsageStateToString(GpuResourceState aState);
    static const char* CommandListTypeToString(CommandListType aType);

    static bool IsInitialized();

    static const Texture* GetDefaultDiffuseTexture();
    static const Texture* GetDefaultNormalTexture();
    static const Texture* GetDefaultMaterialTexture();
    static const ShaderCompiler* GetShaderCompiler();
    
    static SharedPtr<Shader> GetShader(uint64 aDescHash);
    static SharedPtr<ShaderPipeline> GetShaderPipeline(uint64 aDescHash);
    static SharedPtr<Mesh> CreateMesh(const MeshDesc& aDesc, const MeshData* someMeshDatas, uint aNumMeshDatas);

    static SharedPtr<RenderOutput> CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams);
    static SharedPtr<Shader> CreateShader(const ShaderDesc& aDesc);
    static SharedPtr<ShaderPipeline> CreateShaderPipeline(const ShaderPipelineDesc& aDesc);
    static SharedPtr<Texture> CreateTexture(const TextureProperties& someProperties, const char* aName = nullptr, TextureSubData* someUploadDatas = nullptr, uint aNumUploadDatas = 0u);
    static SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferProperties& someProperties, const char* aName = nullptr, const void* someInitialData = nullptr);
    static SharedPtr<TextureView> CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aName = nullptr);
    static SharedPtr<TextureView> CreateTextureView(const TextureProperties& someProperties, const TextureViewProperties& someViewProperties, const char* aName = nullptr, TextureSubData* someUploadDatas = nullptr, uint aNumUploadDatas = 0u);
    static SharedPtr<GpuBufferView> CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, GpuBufferViewProperties someProperties, const char* aName = nullptr);
    static SharedPtr<GpuBufferView> CreateBufferView(const GpuBufferProperties& someProperties, GpuBufferViewProperties someViewProperties, const char* aName = nullptr, const void* someInitialData = nullptr);
    static uint GetQueryTypeDataSize(GpuQueryType aType);
    
    static MappedTempBuffer ReadbackBufferData(const GpuBuffer* aBuffer, uint64 anOffset, uint64 aByteSize);
    static MappedTempTextureBuffer ReadbackTextureData(const Texture* aTexture, const TextureSubLocation& aStartSubLocation, uint aNumSublocations);
    static bool ReadbackTextureData(const Texture* aTexture, const TextureSubLocation& aStartSubLocation, uint aNumSublocations, TextureData& aTextureDataOut);
    
    static SharedPtr<BlendState> CreateBlendState(const BlendStateDesc& aDesc);
    static SharedPtr<DepthStencilState> CreateDepthStencilState(const DepthStencilStateDesc& aDesc);
    static const SharedPtr<BlendState>& GetDefaultBlendState();
    static const SharedPtr<DepthStencilState>& GetDefaultDepthStencilState();

    static RenderPlatformType GetPlatformType();
    static const RenderPlatformCaps& GetPlatformCaps();
    static RenderCore_Platform* GetPlatform();
    static RenderCore_PlatformDX12* GetPlatformDX12();
    static RenderCore_PlatformVk* GetPlatformVk();

    static GpuRingBuffer* AllocateRingBuffer(CpuMemoryAccessType aCpuAccess, uint someBindFlags, uint64 aNeededByteSize, const char* aName = nullptr);
    static void ReleaseRingBuffer(GpuRingBuffer* aBuffer, uint64 aFenceVal);

    static CommandList* BeginCommandList(CommandListType aType);
    static uint64 ExecuteAndFreeCommandList(CommandList* aCommandList, SyncMode aSyncMode = SyncMode::ASYNC);
    static uint64 ExecuteAndResetCommandList(CommandList* aCommandList, SyncMode aSyncMode = SyncMode::ASYNC);

    static TempTextureResource AllocateTempTexture(const TextureResourceProperties& someProps, uint someFlags, const char* aName);
    static TempBufferResource AllocateTempBuffer(const GpuBufferResourceProperties& someProps, uint someFlags, const char* aName);

    static GpuQueryHeap* GetQueryHeap(GpuQueryType aType) { return ourQueryHeaps[ourCurrQueryHeapIdx][(uint)aType].get(); }
    static uint AllocateQueryRange(GpuQueryType aType, uint aNumQueries);
    static void FreeQueryRange(GpuQueryType aType, uint aFirstQuery, uint aNumQueries, uint aNumUsedQueries);
    static bool BeginQueryDataReadback(GpuQueryType aType, uint64 aFrameIdx, const uint8** aDataPtrOut = nullptr);
    static bool ReadQueryData(const GpuQuery& aQuery, uint8* aData);
    static void EndQueryDataReadback(GpuQueryType aType);
    static float64 GetGpuTicksToMsFactor(CommandListType aCommandListType);
    
    static bool IsFenceDone(uint64 aFenceVal);
    static bool IsFrameDone(uint64 aFrameIdx);
    static void WaitForFrame(uint64 aFrameIdx);
    static void WaitForFence(uint64 aFenceVal);
    static void WaitForIdle(CommandListType aType);
    static void WaitForResourceIdle(const GpuResource* aResource, uint aSubresourceOffset = 0, uint aNumSubresources = UINT_MAX);

    static CommandQueue* GetCommandQueue(CommandListType aType);
    static CommandQueue* GetCommandQueue(uint64 aFenceVal);

    static Slot<void(const Shader*)> ourOnShaderRecompiled;

  protected:
    RenderCore() = default;

    static void Init_0_Platform(RenderPlatformType aRenderingApi);
    static void Init_1_Services();
    static void Init_2_Resources();

    static void Shutdown_0_Resources();
    static void Shutdown_1_Services();
    static void Shutdown_2_Platform();

    static void UpdateAvailableRingBuffers();
    static void ResolveUsedQueryData();

    static void OnShaderFileUpdated(const String& aShaderFile);
    static void OnShaderFileDeletedMoved(const String& aShaderFile);

    static UniquePtr<RenderCore_Platform> ourPlatformImpl;
    static UniquePtr<TempResourcePool> ourTempResourcePool;
    static UniquePtr<ShaderCompiler> ourShaderCompiler;
    static UniquePtr<FileWatcher> ourShaderFileWatcher;

    static SharedPtr<DepthStencilState> ourDefaultDepthStencilState;
    static SharedPtr<BlendState> ourDefaultBlendState;
    static SharedPtr<Texture> ourDefaultDiffuseTexture;
    static SharedPtr<Texture> ourDefaultNormalTexture;
    static SharedPtr<Texture> ourDefaultSpecularTexture;

    static std::map<uint64, SharedPtr<Shader>> ourShaderCache;  
    static std::map<uint64, SharedPtr<ShaderPipeline>> ourShaderPipelineCache;
    static std::map<uint64, SharedPtr<BlendState>> ourBlendStateCache;
    static std::map<uint64, SharedPtr<DepthStencilState>> ourDepthStencilStateCache;
    
    static DynamicArray<UniquePtr<GpuRingBuffer>> ourRingBufferPool;
    static std::list<GpuRingBuffer*> ourAvailableRingBuffers;
    static std::list<std::pair<uint64, GpuRingBuffer*>> ourUsedRingBuffers;

    static UniquePtr<CommandQueue> ourCommandQueues[(uint)CommandListType::NUM];
    
    static StaticCircularArray<uint64, NUM_QUEUED_FRAMES> ourQueuedFrameDoneFences;
    static StaticCircularArray<std::pair<uint64, uint64>, 256> ourLastFrameDoneFences;
    
    static UniquePtr<GpuQueryHeap> ourQueryHeaps[NUM_QUEUED_FRAMES][(uint)GpuQueryType::NUM];
    static uint ourCurrQueryHeapIdx;

    static FixedArray<std::pair<uint, uint>, 512> ourUsedQueryRanges[(uint)GpuQueryType::NUM];
    static uint ourNumUsedQueryRanges[(uint)GpuQueryType::NUM];

    static UniquePtr<GpuBuffer> ourQueryBuffers[NUM_QUERY_BUFFERS][(uint)GpuQueryType::NUM];
    static uint64 ourQueryBufferFrames[NUM_QUERY_BUFFERS];
    static uint ourCurrQueryBufferIdx;

    static const uint8* ourMappedQueryBufferData[(uint)GpuQueryType::NUM];
    static uint ourMappedQueryBufferIdx[(uint)GpuQueryType::NUM];
  };
//---------------------------------------------------------------------------//
}
