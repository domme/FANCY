#pragma once

#include "RendererPrerequisites.h"
#include "RenderingStartupParameters.h"
#include <list>
#include "CommandQueue.h"
#include "CommandListType.h"
#include "Slot.h"
#include "TempResourcePool.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class CommandListType;
  struct MeshData;
  struct GpuProgramDesc;
  struct GpuProgramPipelineDesc;
  struct DepthStencilStateDesc;
  struct BlendStateDesc;
  struct WindowParameters;
  class BlendState;
  class DepthStencilState;
  class RenderCore_Platform;
  class CommandContext;
  class RenderOutput;
  class RenderCore_PlatformDX12;
  class Mesh;
  struct MeshDesc;
  class FileWatcher;
  class Texture;
  struct TextureViewProperties;
  class TextureView;
  struct RenderPlatformCaps;
  class GpuRingBuffer;
  class GpuBuffer;
  class GpuBufferView;
  struct GpuBufferViewProperties;
//---------------------------------------------------------------------------//
  class RenderCore
  {
  public:
    /// Init platform-independent stuff
    static void Init(RenderingApi aRenderingApi);
    static void EndFrame();
    static void Shutdown();

    static bool IsInitialized();

    static const Texture* GetDefaultDiffuseTexture();
    static const Texture* GetDefaultNormalTexture();
    static const Texture* GetDefaultMaterialTexture();
    static const GpuProgramCompiler* GetGpuProgramCompiler();
    static DataFormat ResolveFormat(DataFormat aFormat);
    
    static SharedPtr<GpuProgram> GetGpuProgram(uint64 aDescHash);
    static SharedPtr<GpuProgramPipeline> GetGpuProgramPipeline(uint64 aDescHash);
    static SharedPtr<Mesh> CreateMesh(const MeshDesc& aDesc, const MeshData* someMeshDatas, uint aNumMeshDatas);

    static SharedPtr<RenderOutput> CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams);
    static SharedPtr<GpuProgram> CreateGpuProgram(const GpuProgramDesc& aDesc);
    static SharedPtr<GpuProgramPipeline> CreateGpuProgramPipeline(const GpuProgramPipelineDesc& aDesc);
    static SharedPtr<Texture> CreateTexture(const TextureProperties& someProperties, const char* aName = nullptr, TextureSubData* someUploadDatas = nullptr, uint aNumUploadDatas = 0u);
    static SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferProperties& someProperties, const char* aName = nullptr, const void* someInitialData = nullptr);
    static SharedPtr<TextureView> CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties);
    static SharedPtr<TextureView> CreateTextureView(const TextureProperties& someProperties, const TextureViewProperties& someViewProperties, const char* aName = nullptr, TextureSubData* someUploadDatas = nullptr, uint aNumUploadDatas = 0u);
    static SharedPtr<GpuBufferView> CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties);
    static SharedPtr<GpuBufferView> CreateBufferView(const GpuBufferProperties& someProperties, const GpuBufferViewProperties& someViewProperties, const char* aName = nullptr, const void* someInitialData = nullptr);

    static void UpdateBufferData(GpuBuffer* aDestBuffer, uint64 aDestOffset, const void* aDataPtr, uint64 aByteSize);
    static void UpdateTextureData(Texture* aDestTexture, const TextureSubLocation& aStartSubresource, const TextureSubData* someDatas, uint aNumDatas);
    
    static SharedPtr<BlendState> CreateBlendState(const BlendStateDesc& aDesc);
    static SharedPtr<DepthStencilState> CreateDepthStencilState(const DepthStencilStateDesc& aDesc);
    static const SharedPtr<BlendState>& GetDefaultBlendState();
    static const SharedPtr<DepthStencilState>& GetDefaultDepthStencilState();

    static const RenderPlatformCaps& GetPlatformCaps();
    static RenderCore_Platform* GetPlatform();
    static RenderCore_PlatformDX12* GetPlatformDX12();

    static GpuRingBuffer* AllocateRingBuffer(GpuBufferUsage aUsage, uint64 aSize, const char* aName = nullptr);
    static void ReleaseRingBuffer(GpuRingBuffer* aBuffer, uint64 aFenceVal);

    static GpuBuffer* AllocateReadbackBuffer(uint64 aSize, const char* aName = nullptr);
    static void ReleaseReadbackBuffer(GpuBuffer* aBuffer);

    static CommandContext* AllocateContext(CommandListType aType);
    static void FreeContext(CommandContext* aContext);

    static TempTextureResource AllocateTempTexture(const TextureResourceProperties& someProps, uint someFlags, const char* aName);

    static void WaitForFence(uint64 aFenceVal);
    static void WaitForIdle(CommandListType aType);
    static void WaitForResourceIdle(const GpuResource* aResource, uint aSubresourceOffset = 0, uint aNumSubresources = UINT_MAX);

    static CommandQueue* GetCommandQueue(CommandListType aType);
    static CommandQueue* GetCommandQueue(uint64 aFenceVal);

    static Slot<void(const GpuProgram*)> ourOnShaderRecompiled;

  protected:
    RenderCore() = default;

    static void Init_0_Platform(RenderingApi aRenderingApi);
    static void Init_1_Services();
    static void Init_2_Resources();

    static void Shutdown_0_Resources();
    static void Shutdown_1_Services();
    static void Shutdown_2_Platform();

    static void UpdateAvailableRingBuffers();

    static std::unique_ptr<RenderCore_Platform> ourPlatformImpl;
    
    static std::map<uint64, SharedPtr<GpuProgram>> ourShaderCache;
    static std::map<uint64, SharedPtr<GpuProgramPipeline>> ourGpuProgramPipelineCache;
    static std::map<uint64, SharedPtr<BlendState>> ourBlendStateCache;
    static std::map<uint64, SharedPtr<DepthStencilState>> ourDepthStencilStateCache;
    
    static std::vector<std::unique_ptr<CommandContext>> ourRenderContextPool;
    static std::vector<std::unique_ptr<CommandContext>> ourComputeContextPool;
    static std::list<CommandContext*> ourAvailableRenderContexts;
    static std::list<CommandContext*> ourAvailableComputeContexts;

    static SharedPtr<DepthStencilState> ourDefaultDepthStencilState;
    static SharedPtr<BlendState> ourDefaultBlendState;
    static SharedPtr<Texture> ourDefaultDiffuseTexture;
    static SharedPtr<Texture> ourDefaultNormalTexture;
    static SharedPtr<Texture> ourDefaultSpecularTexture;
        
    static UniquePtr<GpuProgramCompiler> ourShaderCompiler;
    static UniquePtr<FileWatcher> ourShaderFileWatcher;
    
    static std::vector<std::unique_ptr<GpuRingBuffer>> ourRingBufferPool;
    static std::list<GpuRingBuffer*> ourAvailableRingBuffers;
    static std::list<std::pair<uint64, GpuRingBuffer*>> ourUsedRingBuffers;

    static std::vector<std::unique_ptr<GpuBuffer>> ourReadbackBufferPool;
    static std::list<GpuBuffer*> ourAvailableReadbackBuffers;

    static TempResourcePool ourTempResourcePool;

    static void OnShaderFileUpdated(const String& aShaderFile);
    static void OnShaderFileDeletedMoved(const String& aShaderFile);
  };
//---------------------------------------------------------------------------//
}
