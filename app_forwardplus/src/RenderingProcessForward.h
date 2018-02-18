#ifndef INCLUDE_RENDERINGPROCESSFORWARD_H
#define INCLUDE_RENDERINGPROCESSFORWARD_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "RenderingProcess.h"
#include "RenderQueues.h"

namespace Fancy{ namespace Scene{
  class LightComponent;
  class CameraComponent;
} }

//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {
class Material;
class BlendState;
class DepthStencilState;
class CommandContext;

class RenderingProcessForward : public RenderingProcess
{
public:
  RenderingProcessForward();
  virtual ~RenderingProcessForward();

  void Startup() override;
  void Tick(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock) override;

protected:
  void PopulateRenderQueues(const GraphicsWorld* aWorld);
  void FlushRenderQueues(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock) const;

  static void BindResources_ForwardColorPass(CommandContext* aRenderContext, const Material* aMaterial);

  void UpdatePerFrameData(const Time& aClock) const;
  void UpdatePerCameraData(const Scene::CameraComponent* aCamera) const;
  void UpdatePerLightData(const Scene::LightComponent* aLight, const Scene::CameraComponent* aCamera) const;
  void UpdatePerDrawData(const Scene::CameraComponent* aCamera, const glm::float4x4& aWorldMat) const;

  RenderQueue myRenderQueueFromCamera;

  SharedPtr<GpuBuffer> myPerFrameData;
  SharedPtr<GpuBuffer> myPerMaterialData;
  SharedPtr<GpuBuffer> myPerCameraData;
  SharedPtr<GpuBuffer> myPerLightData;
  SharedPtr<GpuBuffer> myPerViewportData;
  SharedPtr<GpuBuffer> myPerDrawData;

  SharedPtr<Geometry::GeometryData> myFullscreenQuad;
  SharedPtr<GpuProgramPipeline> myFsTextureShaderState;
    
  SharedPtr<BlendState> myBlendStateAdd;
  SharedPtr<GpuProgramPipeline> myDefaultObjectShaderState;
    
  // Tests:
  void _DebugLoadComputeShader();
  void _DebugExecuteComputeShader();
  SharedPtr<Texture> myTestTexture;
  SharedPtr<GpuProgramPipeline> myComputeProgram;
};
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(RenderingProcessForward)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_RENDERINGPROCESS_H