#pragma once

#include "RendererPrerequisites.h"
#include "RenderingStartupParameters.h"
#include <list>
#include <deque>
#include <mutex>

namespace Fancy {
  struct MeshData;
  //---------------------------------------------------------------------------//
  enum class CommandListType;
  struct TextureDesc;
  struct GpuProgramDesc;
  struct GpuProgramPipelineDesc;
  struct DepthStencilStateDesc;
  struct BlendStateDesc;
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
  struct RenderPlatformCaps;
//---------------------------------------------------------------------------//
  struct GpuDynamicBuffer
  {
    GpuDynamicBuffer() : myData(nullptr), myOffset(0u) {}
    SharedPtr<GpuBuffer> myBuffer;
    uint8* myData;
    uint64 myOffset;
  };
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

    static SharedPtr<RenderOutput> CreateRenderOutput(void* aNativeInstanceHandle);
    static SharedPtr<GpuProgram> CreateGpuProgram(const GpuProgramDesc& aDesc);
    static SharedPtr<GpuProgramPipeline> CreateGpuProgramPipeline(const GpuProgramPipelineDesc& aDesc);
    static SharedPtr<Texture> CreateTexture(const TextureParams& someParams, TextureUploadData* someUploadDatas = nullptr, uint aNumUploadDatas = 0u);
    static SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferCreationParams& someParams, void* someInitialData = nullptr);

    static void InitBufferData(GpuBuffer* aBuffer, const void* aDataPtr);
    static void UpdateBufferData(GpuBuffer* aBuffer, void* aDataPtr, uint aByteSize, uint aByteOffsetFromBuffer = 0u);
    static void InitTextureData(Texture* aTexture, const TextureUploadData* someUploadDatas, uint aNumUploadDatas);

    static SharedPtr<BlendState> CreateBlendState(const BlendStateDesc& aDesc);
    static SharedPtr<DepthStencilState> CreateDepthStencilState(const DepthStencilStateDesc& aDesc);
    static const SharedPtr<BlendState>& GetDefaultBlendState();
    static const SharedPtr<DepthStencilState>& GetDefaultDepthStencilState();

    static const RenderPlatformCaps& GetPlatformCaps();
    static RenderCore_Platform* GetPlatform();
    static RenderCore_PlatformDX12* GetPlatformDX12();

    static GpuDynamicBuffer* AllocateDynamicBuffer(uint64 aNeededByteSize);
    static void ReleaseDynamicBuffer(GpuDynamicBuffer* aBuffer, uint64 aFenceVal);

    static CommandContext* AllocateContext(CommandListType aType);
    static void FreeContext(CommandContext* aContext);

  protected:
    RenderCore() = default;

    static void Init_0_Platform(RenderingApi aRenderingApi);
    static void Init_1_Services();
    static void Init_2_Resources();

    static void Shutdown_0_Resources();
    static void Shutdown_1_Services();
    static void Shutdown_2_Platform();

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
    
    static std::unique_ptr<GpuProgramCompiler> ourShaderCompiler;
    static std::unique_ptr<FileWatcher> ourShaderFileWatcher;

    static std::mutex ourDynamicBufferMutex;
    static std::vector<std::unique_ptr<GpuDynamicBuffer>> ourDynamicBufferPool;
    static std::deque<GpuDynamicBuffer*> ourAvailableDynamicBuffers;
    static std::deque<std::pair<GpuDynamicBuffer*, uint64>> ourUsedDynamicBuffers;

    static void OnShaderFileUpdated(const String& aShaderFile);
    static void OnShaderFileDeletedMoved(const String& aShaderFile);
  };
//---------------------------------------------------------------------------//
}