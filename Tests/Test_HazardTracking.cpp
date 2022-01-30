#include "Test_HazardTracking.h"
#include "TextureProperties.h"
#include "Texture.h"
#include "RenderCore.h"
#include "CommandList.h"

using namespace Fancy;

Test_HazardTracking::Test_HazardTracking(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow,
  Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Hazard Tracking")
{
  TextureProperties texProps;
  texProps.myWidth = 32;
  texProps.myHeight = 32;
  texProps.myNumMipLevels = 3;
  texProps.myFormat = DataFormat::R_32UI;
  texProps.myIsShaderWritable = true;
  myTex = RenderCore::CreateTexture(texProps, "Hazard tracking test texture");

  for (uint i = 0u; i < 3; ++i)
  {
    TextureViewProperties viewProps;
    viewProps.mySubresourceRange = SubresourceRange(i, 1, 0, 1, 0, 1);

    myTexMipRead[i] = RenderCore::CreateTextureView(myTex, viewProps, "Hazard tracking test texture read");
    
    viewProps.myIsShaderWritable = true;
    myTexMipWrite[i] = RenderCore::CreateTextureView(myTex, viewProps, "Hazard tracking test texture write");
  }

  GpuBufferProperties bufProps;
  bufProps.myBindFlags = (uint) GpuBufferBindFlags::SHADER_BUFFER;
  bufProps.myIsShaderWritable = true;
  bufProps.myNumElements = (32 * 32) + (16 * 16) + (8 * 8);
  bufProps.myElementSizeBytes = 4u;

  eastl::vector<unsigned int>initialBufferData(bufProps.myNumElements, 0u);
  myBuffer = RenderCore::CreateBuffer(bufProps, "Hazard tracking test buffer", initialBufferData.data());
  
  uint viewOffsets[3] = {
    0,
    32 * 32 * sizeof(unsigned int),
    32 * 32 * sizeof(unsigned int) + 16 * 16 * sizeof(unsigned int),
  };

  uint viewSizes[3] = {
    32 * 32 * sizeof(unsigned int),
    16 * 16 * sizeof(unsigned int),
    8 * 8 * sizeof(unsigned int)
  };

  for (uint i = 0u; i < 3; ++i)
  {
    GpuBufferViewProperties bufViewProps;
    bufViewProps.myOffset = viewOffsets[i];
    bufViewProps.mySize = viewSizes[i];
    bufViewProps.myFormat = DataFormat::R_32UI;
    
    myBufferRead[i] = RenderCore::CreateBufferView(myBuffer, bufViewProps, "Hazard tracking test buffer read");

    bufViewProps.myIsShaderWritable = true;
    myBufferWrite[i] = RenderCore::CreateBufferView(myBuffer, bufViewProps, "Hazard tracking test buffer write");
  }

  ShaderPipelineDesc pipelineDesc;
  ShaderDesc* shaderDesc = &pipelineDesc.myShader[(uint)ShaderStage::SHADERSTAGE_COMPUTE];
  shaderDesc->myShaderStage = (uint) ShaderStage::SHADERSTAGE_COMPUTE;
  shaderDesc->myMainFunction = "main";
  shaderDesc->myPath = "fancy/resources/shaders/Tests/HazardTracking.hlsl";

  shaderDesc->myDefines.push_back("BUFFER_TO_TEXTURE_MIP");
  myBufferToMipShader = RenderCore::CreateShaderPipeline(pipelineDesc);

  shaderDesc->myDefines.clear();
  shaderDesc->myDefines.push_back("TEXUTRE_MIP_TO_BUFFER");
  myMipToBufferShader = RenderCore::CreateShaderPipeline(pipelineDesc);
}

Test_HazardTracking::~Test_HazardTracking()
{
}

void Test_HazardTracking::OnUpdate(bool aDrawProperties)
{
  CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);

  glm::int3 dispatchSizes[3] =
  {
    glm::int3(32, 32, 1),
    glm::int3(16, 16, 1),
    glm::int3(8, 8, 1)
  };

  struct cBuffer
  {
    uint myBufferIndex;
    uint myTextureIndex;
  };
  
  for (uint i = 0u; i < 3; ++i)
  {
    ctx->SetShaderPipeline(myBufferToMipShader.get());

    cBuffer cbuf =
    {
      myBufferRead[i]->GetGlobalDescriptorIndex(),
      myTexMipWrite[i]->GetGlobalDescriptorIndex()
    };
    ctx->BindConstantBuffer(&cbuf, sizeof(cbuf), 0);

    ctx->PrepareResourceShaderAccess(myBufferRead[i].get());
    ctx->PrepareResourceShaderAccess(myTexMipWrite[i].get());
    ctx->Dispatch(dispatchSizes[i]);
  }

  RenderCore::ExecuteAndResetCommandList(ctx, SyncMode::BLOCKING);

  for (uint i = 0u; i < 3; ++i)
  {
    ctx->SetShaderPipeline(myMipToBufferShader.get());

    cBuffer cbuf =
    {
      myBufferWrite[i]->GetGlobalDescriptorIndex(),
      myTexMipRead[i]->GetGlobalDescriptorIndex()
    };
    ctx->BindConstantBuffer(&cbuf, sizeof(cbuf), 0);
    ctx->PrepareResourceShaderAccess(myTexMipRead[i].get());
    ctx->PrepareResourceShaderAccess(myBufferWrite[i].get());
    ctx->Dispatch(dispatchSizes[i]);
  }

  RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
}

void Test_HazardTracking::OnRender()
{
}
