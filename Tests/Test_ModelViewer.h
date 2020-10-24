#pragma once

#include "Test.h"
#include "fancy_core/Ptr.h"
#include "fancy_core/Camera.h"
#include "fancy_core/CameraController.h"
#include "fancy_core/MeshImporter.h"
#include "fancy_core/MathIncludes.h"

namespace Fancy
{
  class CommandList;
  class ShaderPipeline;
  class TextureSampler;
  struct VertexInputLayout;
  class GpuBuffer;
  struct Material;
  struct Scene;
  class GpuResourceViewSet;

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

  Fancy::Camera myCamera;
  Fancy::CameraController myCameraController;
  Fancy::SharedPtr<Fancy::Scene> myScene;

  Fancy::SharedPtr<Fancy::ShaderPipeline> myUnlitTexturedShader;
  Fancy::SharedPtr<Fancy::ShaderPipeline> myInstancedUnlitTexturedShader;
  Fancy::SharedPtr<Fancy::ShaderPipeline> myUnlitVertexColorShader;
  Fancy::SharedPtr<Fancy::ShaderPipeline> myDebugGeoShader;
  Fancy::SharedPtr<Fancy::TextureSampler> mySampler;
  Fancy::SharedPtr<Fancy::VertexInputLayout> myInstancedVertexLayout;
  Fancy::SharedPtr<Fancy::GpuBuffer> myInstancePositions;
  Fancy::SharedPtr<Fancy::TextureView> myDefaultFloorTexture;
  Fancy::SharedPtr<Fancy::TextureView> myMarmorTexture;
  Fancy::SharedPtr<Fancy::GpuResourceViewSet> myResourceViewSet;
  int myNumInstances;
};
