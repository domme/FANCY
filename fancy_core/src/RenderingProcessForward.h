#ifndef INCLUDE_RENDERINGPROCESSFORWARD_H
#define INCLUDE_RENDERINGPROCESSFORWARD_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "RenderingProcess.h"

namespace Fancy{ namespace Scene{
  class LightComponent;
  class CameraComponent;
} }

//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {
class MaterialPassInstance;

class DLLEXPORT RenderingProcessForward : public RenderingProcess
  {
  public:
    RenderingProcessForward();
    virtual ~RenderingProcessForward();

    virtual void Startup() override;
    virtual void Tick(float _dt) override;

  protected:
    void BindResources_ForwardColorPass(RenderContext* aRenderContext, const MaterialPassInstance* aMaterial) const;

    void UpdatePerFrameData() const;
    void UpdatePerCameraData(const Scene::CameraComponent* aCamera) const;
    void UpdatePerLightData(const Scene::LightComponent* aLight, const Scene::CameraComponent* aCamera) const;
    void UpdatePerDrawData(const Scene::CameraComponent* aCamera, const glm::float4x4& aWorldMat) const;

    SharedPtr<GpuBuffer> myPerFrameData;
    SharedPtr<GpuBuffer> myPerMaterialData;
    SharedPtr<GpuBuffer> myPerCameraData;
    SharedPtr<GpuBuffer> myPerLightData;
    SharedPtr<GpuBuffer> myPerViewportData;
    SharedPtr<GpuBuffer> myPerDrawData;

    SharedPtr<Geometry::GeometryData> myFullscreenQuad;
    SharedPtr<GpuProgramPipeline> myFsTextureShaderState;

    // Tests:
    void _DebugLoadComputeShader();
    void _DebugExecuteComputeShader();
    SharedPtr<Texture> myTestTexture;
    SharedPtr<GpuProgram> myComputeProgram;

  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(RenderingProcessForward)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_RENDERINGPROCESS_H