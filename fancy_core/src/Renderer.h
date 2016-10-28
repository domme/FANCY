#ifndef INCLUDE_RENDERER_H
#define INCLUDE_RENDERER_H

#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_RENDERER

namespace Fancy { namespace Rendering {
struct GpuProgramDesc;

//---------------------------------------------------------------------------//
  class DLLEXPORT RenderOutput : public PLATFORM_DEPENDENT_NAME(RenderOutput)
  {
    public:
      RenderOutput() {}
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
    static GpuProgram* GetGpuProgram(const GpuProgramDesc& aDesc);

    static SharedPtr<Texture> CreateTexture(const TextureParams& someParams, TextureUploadData* someUploadDatas = nullptr, uint32 aNumUploadDatas = 0u);
    static SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferCreationParams& someParams, void* someInitialData = nullptr);
    static void UpdateBufferData(GpuBuffer* aBuffer, void* aData, uint32 aDataSizeBytes, uint32 aByteOffsetFromBuffer = 0u);

  protected:
    RenderCore() {}

    static ScopedPtr<GpuProgramCompiler> ourShaderCompiler;
    static std::shared_ptr<Texture> ourDefaultDiffuseTexture;
    static std::shared_ptr<Texture> ourDefaultNormalTexture;
    static std::shared_ptr<Texture> ourDefaultSpecularTexture;
  };
//---------------------------------------------------------------------------//
} // end of namespace Rendering
} // end of namespace Fancy

#endif  // INCLUDE_RENDERER_H