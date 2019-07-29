#pragma once

#include "Test.h"
#include "fancy_core/Ptr.h"
#include "fancy_assets/ModelLoader.h"
#include "app_framework/Camera.h"
#include "app_framework/CameraController.h"

namespace Fancy
{
  class CommandList;
  class AssetManager;
  class GpuProgramPipeline;
}

class Test_ModelViewer : public Test
{
public:
  Test_ModelViewer(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_ModelViewer() override;
  void OnWindowResized(uint aWidth, uint aHeight) override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

private:
  void RenderGrid(Fancy::CommandList* ctx);
  void RenderScene(Fancy::CommandList* ctx);

  Fancy::SharedPtr<Fancy::AssetManager> myAssetManager;
  Fancy::ModelLoader::Scene myScene;

  Camera myCamera;
  Fancy::SharedPtr<CameraController> myCameraController;

  Fancy::SharedPtr<Fancy::GpuProgramPipeline> myUnlitTexturedShader;
  Fancy::SharedPtr<Fancy::GpuProgramPipeline> myUnlitVertexColorShader;
  Fancy::SharedPtr<Fancy::GpuProgramPipeline> myDebugGeoShader;  
};

