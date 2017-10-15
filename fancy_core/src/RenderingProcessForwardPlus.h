#pragma once

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

class DLLEXPORT RenderingProcessForwardPlus : public RenderingProcess
{
public:
  RenderingProcessForwardPlus();
  virtual ~RenderingProcessForwardPlus();

  void Startup() override;
  void Tick(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock) override;

protected:
  void PopulateRenderQueues(const GraphicsWorld* aWorld);

  void DepthPrepass(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock);
  void BuildLightTiles(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock);

  void UpdatePerFrameData(const Time& aClock) const;
  void UpdatePerCameraData(const Scene::CameraComponent* aCamera) const;
  void UpdatePerLightData(const Scene::LightComponent* aLight, const Scene::CameraComponent* aCamera) const;
  void UpdatePerDrawData(const Scene::CameraComponent* aCamera, const glm::float4x4& aWorldMat) const;

  void StartupDebug();
  void RenderDebug(const RenderOutput* anOutput);
  void UpdateDebug(const RenderOutput* anOutput);

  RenderQueue myRenderQueueFromCamera;

  SharedPtr<GpuBuffer> myPerFrameData;
  SharedPtr<GpuBuffer> myPerMaterialData;
  SharedPtr<GpuBuffer> myPerCameraData;
  SharedPtr<GpuBuffer> myPerLightData;
  SharedPtr<GpuBuffer> myPerViewportData;
  SharedPtr<GpuBuffer> myPerDrawData;

  SharedPtr<Geometry::GeometryData> myFullscreenQuad;
    
  SharedPtr<BlendState> myBlendStateAdd;
  SharedPtr<BlendState> myBlendStateNoColors;
  
  SharedPtr<GpuProgramPipeline> myDepthPrepassObjectShader;
  SharedPtr<GpuProgramPipeline> myBuildLightTilesShader;

  SharedPtr<Texture> myDepthBufferDebugTex;
  SharedPtr<GpuBuffer> myDebugTextureParams;
  SharedPtr<GpuProgramPipeline> myDefaultTextureDebugShader;
  SharedPtr<GpuProgramPipeline> myDepthBufferDebugShader;
  SharedPtr<DepthStencilState> myDepthStencil_NoDepthTest;
};
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(RenderingProcessForwardPlus)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering