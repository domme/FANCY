#include "Test_SharedQueueResourceUsage.h"

#include "fancy_imgui/imgui.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/CommandList.h"
#include "fancy_core/CommandQueue.h"
#include "fancy_core/GrowingList.h"
#include "fancy_core/TimeManager.h"
#include "fancy_core/StaticString.h"
#include "fancy_core/ShaderPipelineDesc.h"
#include "fancy_core/TempResourcePool.h"

using namespace Fancy;

static uint kNumBufferElements = 1024;

Test_SharedQueueResourceUsage::Test_SharedQueueResourceUsage(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Shared Queue Resources")
{
  GpuBufferProperties props;
  props.myElementSizeBytes = sizeof(uint);
  props.myNumElements = kNumBufferElements;
  props.myIsShaderWritable = true;
  props.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
  props.myBindFlags = (uint)GpuBufferBindFlags::SHADER_BUFFER;

  std::vector<uint> initialData;
  initialData.resize(props.myNumElements, 0u);

  myBuffer = RenderCore::CreateBuffer(props, "Shared Queue Test Buffer", initialData.data());

  GpuBufferViewProperties viewProps;
  viewProps.myFormat = DataFormat::R_32UI;
  viewProps.myIsShaderWritable = true;
  myBufferWrite = RenderCore::CreateBufferView(myBuffer, viewProps, "Shared Queue Test Buffer UAV");

  viewProps.myIsShaderWritable = false;
  myBufferRead = RenderCore::CreateBufferView(myBuffer, viewProps, "Shared Queue Test Buffer SRV");

  ShaderPipelineDesc pipelineDesc;
  ShaderDesc& shaderDesc = pipelineDesc.myShader[(uint)ShaderStage::COMPUTE];
  shaderDesc.myShaderStage = (uint)ShaderStage::COMPUTE;
  shaderDesc.myPath = "Tests/ModifyBuffer.hlsl";
  shaderDesc.myMainFunction = "main_increment";
  myWriteBufferShader = RenderCore::CreateShaderPipeline(pipelineDesc);

  shaderDesc.myMainFunction = "main_copy";
  myCopyBufferShader = RenderCore::CreateShaderPipeline(pipelineDesc);
}

Test_SharedQueueResourceUsage::~Test_SharedQueueResourceUsage()
{
  RenderCore::WaitForIdle(CommandListType::Graphics);
  RenderCore::WaitForIdle(CommandListType::Compute);
}

void Test_SharedQueueResourceUsage::OnUpdate(bool aDrawProperties)
{
  CommandList* graphicsContext = RenderCore::BeginCommandList(CommandListType::Graphics);
  graphicsContext->SetShaderPipeline(myWriteBufferShader.get());

  graphicsContext->BindResourceView(myBufferWrite.get(), "DstBuffer");
  graphicsContext->Dispatch(glm::int3(kNumBufferElements, 1, 1));
  graphicsContext->TransitionResource(myBuffer.get(), ResourceTransition::TO_SHARED_CONTEXT_READ);
  const uint64 setValueFence = RenderCore::ExecuteAndFreeCommandList(graphicsContext);

  GpuBufferResourceProperties props;
  props.myBufferProperties = myBuffer->GetProperties();
  props.myFormat = DataFormat::R_32UI;
  props.myIsShaderWritable = true;
  TempBufferResource tempBuffer = RenderCore::AllocateTempBuffer(props, 0u, "Temp buffer");
  
  // Use the resource on the compute context in multiple readonly operations
  RenderCore::GetCommandQueue(CommandListType::Compute)->StallForFence(setValueFence);
  CommandList* computeContext = RenderCore::BeginCommandList(CommandListType::Compute);
  computeContext->SetShaderPipeline(myCopyBufferShader.get());
  computeContext->BindResourceView(myBufferRead.get(), "SrcBuffer");
  computeContext->BindResourceView(tempBuffer.myWriteView, "DstBuffer");
  computeContext->Dispatch(glm::int3(kNumBufferElements, 1, 1));
  
  computeContext->ResourceUAVbarrier();

  computeContext->CopyBuffer(tempBuffer.myBuffer, 0u, myBuffer.get(), 0u, sizeof(uint) * kNumBufferElements);
  
  const uint64 copyBufferFence = RenderCore::ExecuteAndFreeCommandList(computeContext);
}

void Test_SharedQueueResourceUsage::OnRender()
{
}
