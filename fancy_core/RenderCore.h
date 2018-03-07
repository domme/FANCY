#pragma once

#include "RendererPrerequisites.h"
#include "RenderingStartupParameters.h"
#include <list>
#include "RenderPlatformCaps.h"

namespace Fancy {
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
//---------------------------------------------------------------------------//
  class RenderCore
  {
  public:
    /// Init platform-independent stuff
    static void Init(RenderingApi aRenderingApi);
    static void Shutdown();

    static bool IsInitialized();

    static const Texture* GetDefaultDiffuseTexture();
    static const Texture* GetDefaultNormalTexture();
    static const Texture* GetDefaultMaterialTexture();
    static const GpuProgramCompiler* GetGpuProgramCompiler();
    static DataFormat ResolveFormat(DataFormat aFormat);

    static SharedPtr<GpuProgram> GetGpuProgram(uint64 aDescHash);
    static SharedPtr<GpuProgramPipeline> GetGpuProgramPipeline(uint64 aDescHash);
    static SharedPtr<Mesh> GetInternalMesh(const MeshDesc& aDesc);
    static SharedPtr<Mesh> CreateMesh(const MeshDesc& aDesc,
      const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas,
      const std::vector<uint>& someNumVertices, const std::vector<uint>& someNumIndices);

    static SharedPtr<RenderOutput> CreateRenderOutput(void* aNativeInstanceHandle);
    static SharedPtr<GpuProgram> CreateGpuProgram(const GpuProgramDesc& aDesc);
    static SharedPtr<GpuProgramPipeline> CreateGpuProgramPipeline(const GpuProgramPipelineDesc& aDesc);
    static SharedPtr<Texture> GetInternalTexture(const TextureDesc& aDesc);
    static SharedPtr<Texture> CreateTexture(const TextureParams& someParams, TextureUploadData* someUploadDatas = nullptr, uint aNumUploadDatas = 0u);
    static SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferCreationParams& someParams, void* someInitialData = nullptr);

    static void InitBufferData(GpuBuffer* aBuffer, void* aDataPtr);
    static void UpdateBufferData(GpuBuffer* aBuffer, void* aDataPtr, uint aByteSize, uint aByteOffsetFromBuffer = 0u);
    static void InitTextureData(Texture* aTexture, const TextureUploadData* someUploadDatas, uint aNumUploadDatas);

    static SharedPtr<BlendState> CreateBlendState(const BlendStateDesc& aDesc);
    static SharedPtr<DepthStencilState> CreateDepthStencilState(const DepthStencilStateDesc& aDesc);
    static const SharedPtr<BlendState>& GetDefaultBlendState();
    static const SharedPtr<DepthStencilState>& GetDefaultDepthStencilState();

    static const RenderPlatformCaps& GetPlatformCaps();
    static RenderCore_Platform* GetPlatform();
    static RenderCore_PlatformDX12* GetPlatformDX12();

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
    
    static std::map<uint64, SharedPtr<Texture>> ourInternalTextures;
    static std::map<uint64, SharedPtr<Mesh>> ourInternalMeshes;
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

    static void OnShaderFileUpdated(const String& aShaderFile);
    static void OnShaderFileDeletedMoved(const String& aShaderFile);
  };
//---------------------------------------------------------------------------//
}

