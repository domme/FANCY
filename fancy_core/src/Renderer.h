#ifndef INCLUDE_RENDERER_H
#define INCLUDE_RENDERER_H

#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_RENDERER


namespace Fancy {
  class FileWatcher;
}

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  class Mesh;
  struct MeshDesc;
//---------------------------------------------------------------------------//
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc;
  struct GpuProgramPipelineDesc;
  struct DepthStencilStateDesc;
  struct BlendStateDesc;
  class BlendState;
  class DepthStencilState;
//---------------------------------------------------------------------------//
  class DLLEXPORT RenderOutput : public PLATFORM_DEPENDENT_NAME(RenderOutput)
  {
    public:
      RenderOutput(void* aNativeInstanceHandle) : PLATFORM_DEPENDENT_NAME(RenderOutput)(aNativeInstanceHandle) {}
      virtual ~RenderOutput() {}
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class Texture;
//---------------------------------------------------------------------------//
  class RenderCore : public PLATFORM_DEPENDENT_NAME(RenderCore)
  {
  public:
    /// Init platform-independent stuff
    static void Init();
    /// Called after the rendering system has been fully initialzed (Rendering-resources can be created here)
    static void PostInit();

    /// Shutdown platform-independent stuff
    static void Shutdown();

    static const Texture* GetDefaultDiffuseTexture() { return ourDefaultDiffuseTexture.get(); }
    static const Texture* GetDefaultNormalTexture() { return ourDefaultNormalTexture.get(); }
    static const Texture* GetDefaultSpecularTexture() { return ourDefaultSpecularTexture.get(); }
    static const GpuProgramCompiler* GetGpuProgramCompiler() { ASSERT(ourShaderCompiler != nullptr); return ourShaderCompiler; }
    
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

    static SharedPtr<Rendering::BlendState> CreateBlendState(const Rendering::BlendStateDesc& aDesc);
    static SharedPtr<Rendering::DepthStencilState> CreateDepthStencilState(const Rendering::DepthStencilStateDesc& aDesc);

  protected:
    RenderCore() {}

    static FileWatcher* ourShaderFileWatcher;

    static std::map<uint64, SharedPtr<GpuProgram>> ourShaderCache;
    static std::map<uint64, SharedPtr<GpuProgramPipeline>> ourGpuProgramPipelineCache;
    static std::map<uint64, SharedPtr<Texture>> ourTextureCache;
    static std::map<uint64, SharedPtr<Geometry::Mesh>> ourMeshCache;
    static std::map<uint64, SharedPtr<Rendering::BlendState>> ourBlendStateCache;
    static std::map<uint64, SharedPtr<Rendering::DepthStencilState>> ourDepthStencilStateCache;
    
    static ScopedPtr<GpuProgramCompiler> ourShaderCompiler;
    static SharedPtr<Texture> ourDefaultDiffuseTexture;
    static SharedPtr<Texture> ourDefaultNormalTexture;
    static SharedPtr<Texture> ourDefaultSpecularTexture;

    static void OnShaderFileUpdated(const String& aShaderFile);
    static void OnShaderFileDeletedMoved(const String& aShaderFile);
  };
//---------------------------------------------------------------------------//
} // end of namespace Rendering
} // end of namespace Fancy

#endif  // INCLUDE_RENDERER_H