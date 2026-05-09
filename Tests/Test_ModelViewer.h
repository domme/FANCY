#pragma once

#include "Test.h"
#include "Rendering/ResourceHandle.h"
#include "Common/Camera.h"
#include "Common/CameraController.h"
#include "IO/MeshImporter.h"

namespace Fancy {
  class CommandList;
  class ShaderPipeline;
  class TextureSampler;
  struct VertexInputLayout;
  class GpuBuffer;
  struct Material;
  struct Scene;
}  // namespace Fancy

class Test_ModelViewer : public Test {
public:
  Test_ModelViewer( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow, Fancy::RenderOutput * aRenderOutput, Fancy::InputState * anInputState );
  ~Test_ModelViewer() override;
  void OnWindowResized( uint aWidth, uint aHeight ) override;
  void OnUpdate( bool aDrawProperties ) override;
  void OnRender() override;

private:
  void UpdateDepthbuffer();
  void RenderGrid( Fancy::CommandList * ctx );
  void RenderScene( Fancy::CommandList * ctx );

  Fancy::Camera                    myCamera;
  Fancy::CameraController          myCameraController;
  Fancy::ShaderPipelineHandle    myUnlitTexturedShader;
  Fancy::ShaderPipelineHandle    myInstancedUnlitTexturedShader;
  Fancy::ShaderPipelineHandle    myUnlitVertexColorShader;
  Fancy::ShaderPipelineHandle    myDebugGeoShader;
  Fancy::TextureSamplerHandle    mySampler;
  Fancy::VertexInputLayoutHandle myInstancedVertexLayout;
  Fancy::GpuBufferHandle         myInstancePositions;
  Fancy::TextureViewHandle       myDepthStencilDsv;
  Fancy::SharedPtr< Fancy::Scene > myScene;
  int                            myNumInstances;
};
