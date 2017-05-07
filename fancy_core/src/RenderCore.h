#pragma once

#include "RendererPrerequisites.h"
#include "ScopedPtr.h"

namespace Fancy {
  class FileWatcher;
}

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  class Mesh;
  struct MeshDesc;
//---------------------------------------------------------------------------//
} }

namespace Fancy { namespace Rendering { namespace DX12 {
  class RenderCore_PlatformDX12;
} } }

namespace Fancy { namespace Rendering {
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
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class Texture;
//---------------------------------------------------------------------------//
  class RenderCore
  {
  public:
    /// Init platform-independent stuff
    static void Init(RenderingApi aRenderingApi);
    static void Shutdown();

    static bool IsInitialized();

    static const Texture* GetDefaultDiffuseTexture() { return ourDefaultDiffuseTexture.get(); }
    static const Texture* GetDefaultNormalTexture() { return ourDefaultNormalTexture.get(); }
    static const Texture* GetDefaultMaterialTexture() { return ourDefaultSpecularTexture.get(); }
    static const GpuProgramCompiler* GetGpuProgramCompiler() { return ourShaderCompiler.get(); }

    static SharedPtr<Texture> GetTexture(uint64 aDescHash);
    static SharedPtr<GpuProgram> GetGpuProgram(uint64 aDescHash);
    static SharedPtr<GpuProgramPipeline> GetGpuProgramPipeline(uint64 aDescHash);
    static SharedPtr<Geometry::Mesh> GetMesh(uint64 aVertexIndexHash);
    static SharedPtr<Geometry::Mesh> CreateMesh(const Geometry::MeshDesc& aDesc,
      const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas,
      const std::vector<uint>& someNumVertices, const std::vector<uint>& someNumIndices);

    static SharedPtr<GpuProgram> CreateGpuProgram(const GpuProgramDesc& aDesc);
    static SharedPtr<GpuProgramPipeline> CreateGpuProgramPipeline(const GpuProgramPipelineDesc& aDesc);
    static SharedPtr<Texture> CreateTexture(const TextureDesc &aTextureDesc);
    static SharedPtr<Texture> CreateTexture(const String& aTexturePath);
    static SharedPtr<Texture> CreateTexture(const TextureParams& someParams, TextureUploadData* someUploadDatas = nullptr, uint32 aNumUploadDatas = 0u);
    static SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferCreationParams& someParams, void* someInitialData = nullptr);
    static void UpdateBufferData(GpuBuffer* aBuffer, void* aData, uint32 aDataSizeBytes, uint32 aByteOffsetFromBuffer = 0u);

    static SharedPtr<BlendState> CreateBlendState(const Rendering::BlendStateDesc& aDesc);
    static SharedPtr<DepthStencilState> CreateDepthStencilState(const Rendering::DepthStencilStateDesc& aDesc);
    static const SharedPtr<BlendState>& GetDefaultBlendState() { return ourDefaultBlendState; }
    static const SharedPtr<DepthStencilState>& GetDefaultDepthStencilState() { return ourDefaultDepthStencilState; }

    static RenderCore_Platform* GetPlatform() { return ourPlatformImpl; }
    static DX12::RenderCore_PlatformDX12* GetPlatformDX12();

    static CommandContext* AllocateContext(CommandListType aType);
    static void FreeContext(CommandContext* aContext);

  protected:
    RenderCore() {}

    static void Init_0_Platform(RenderingApi aRenderingApi);
    static void Init_1_Services();
    static void Init_2_Resources();

    static void Shutdown_0_Resources();
    static void Shutdown_1_Services();
    static void Shutdown_2_Platform();

    static ScopedPtr<RenderCore_Platform> ourPlatformImpl;

    static std::map<uint64, SharedPtr<GpuProgram>> ourShaderCache;
    static std::map<uint64, SharedPtr<GpuProgramPipeline>> ourGpuProgramPipelineCache;
    static std::map<uint64, SharedPtr<Texture>> ourTextureCache;
    static std::map<uint64, SharedPtr<Geometry::Mesh>> ourMeshCache;
    static std::map<uint64, SharedPtr<Rendering::BlendState>> ourBlendStateCache;
    static std::map<uint64, SharedPtr<Rendering::DepthStencilState>> ourDepthStencilStateCache;

    static SharedPtr<DepthStencilState> ourDefaultDepthStencilState;
    static SharedPtr<BlendState> ourDefaultBlendState;
    static SharedPtr<GpuProgramCompiler> ourShaderCompiler;
    static SharedPtr<Texture> ourDefaultDiffuseTexture;
    static SharedPtr<Texture> ourDefaultNormalTexture;
    static SharedPtr<Texture> ourDefaultSpecularTexture;
    static ScopedPtr<FileWatcher> ourShaderFileWatcher;

    static void OnShaderFileUpdated(const String& aShaderFile);
    static void OnShaderFileDeletedMoved(const String& aShaderFile);
  };
//---------------------------------------------------------------------------//
  } } // end of namespace Fancy::Rendering

