#include "Test_AsyncCompute.h"

#include "fancy_core/MathUtil.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_imgui/imgui.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/CommandList.h"
#include "fancy_core/CommandQueue.h"
#include "fancy_imgui/imgui_internal.h"
#include "fancy_core/GrowingList.h"
#include "fancy_core/TimeManager.h"
#include "fancy_core/StaticString.h"

using namespace Fancy;

static uint kNumBufferElements = 1024;

Test_AsyncCompute::Test_AsyncCompute(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Async Compute")
{
  GpuBufferProperties props;
  props.myElementSizeBytes = sizeof(uint);
  props.myNumElements = kNumBufferElements;
  props.myIsShaderWritable = true;
  props.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
  props.myBindFlags = (uint)GpuBufferBindFlags::SHADER_BUFFER;
  
  std::vector<uint> initialData;
  initialData.resize(props.myNumElements);
  for (uint& data : initialData)
    data = 0;

  myBuffer = RenderCore::CreateBuffer(props, "Async compute test buffer", initialData.data());
  ASSERT(myBuffer);
  
  GpuBufferViewProperties viewProps;
  viewProps.myFormat = DataFormat::R_32UI;
  viewProps.myIsShaderWritable = true;
  myBufferUAV = RenderCore::CreateBufferView(myBuffer, viewProps, "Async compute text buffer UAV");
  ASSERT(myBufferUAV);

  props.myIsShaderWritable = false;
  props.myCpuAccess = CpuMemoryAccessType::CPU_READ;
  myReadbackBuffer = RenderCore::CreateBuffer(props, "Async compute test readback buffer", initialData.data());
  ASSERT(myReadbackBuffer);

  ShaderDesc shaderDesc;
  shaderDesc.myShaderStage = (uint) ShaderStage::COMPUTE;
  shaderDesc.myShaderFileName = "Tests/ModifyBuffer";
  shaderDesc.myMainFunction = "main_increment";
  myIncrementBufferShader = RenderCore::CreateShader(shaderDesc);
  ASSERT(myIncrementBufferShader);

  shaderDesc.myMainFunction = "main_set";
  mySetBufferValueShader = RenderCore::CreateShader(shaderDesc);
  ASSERT(mySetBufferValueShader);
}

Test_AsyncCompute::~Test_AsyncCompute()
{
  RenderCore::WaitForIdle(CommandListType::Graphics);
  RenderCore::WaitForIdle(CommandListType::Compute);
}

void Test_AsyncCompute::OnUpdate(bool aDrawProperties)
{
  // Async compute test:
  // 1) Write to buffer A on graphics queue
  // -- Compute: Wait on graphics
  // 2) Add buffer-values on compute queue
  // -- Graphics: Wait on compute
  // 3) Copy buffer to readback on graphics queue
  // -- Wait for copy on CPU
  // 4) Readback buffer on CPU and verify buffer-contents

  switch(myStage) 
  { 
    case Stage::IDLE: 
    {
      CommandList* graphicsContext = RenderCore::BeginCommandList(CommandListType::Graphics);
      graphicsContext->SetComputeProgram(mySetBufferValueShader.get());

      struct CBuffer
      {
        uint myValue;
        uint _unused;
        uint _unused1;
        uint _unused2;
      };
      myExpectedBufferValue = (uint)Time::ourFrameIdx;
      CBuffer cbuf = { myExpectedBufferValue };
      graphicsContext->BindConstantBuffer(&cbuf, sizeof(cbuf), "CB0");
      
      graphicsContext->BindResourceView(myBufferUAV.get(), "DstBuffer");
      graphicsContext->Dispatch(glm::int3(kNumBufferElements, 1, 1));
      const uint64 setValueFence = RenderCore::ExecuteAndResetCommandList(graphicsContext);

      CommandList* computeContext = RenderCore::BeginCommandList(CommandListType::Compute);
      RenderCore::GetCommandQueue(CommandListType::Compute)->StallForFence(setValueFence);
      computeContext->SetComputeProgram(myIncrementBufferShader.get());
      computeContext->BindResourceView(myBufferUAV.get(), "DstBuffer");
      computeContext->Dispatch(glm::int3(kNumBufferElements, 1, 1));
      const uint64 incrementValueFence = RenderCore::ExecuteAndFreeCommandList(computeContext);

      RenderCore::GetCommandQueue(CommandListType::Graphics)->StallForFence(incrementValueFence);
      graphicsContext->CopyBuffer(myReadbackBuffer.get(), 0ull, myBuffer.get(), 0ull, myBuffer->GetByteSize());
      myBufferCopyFence = RenderCore::ExecuteAndFreeCommandList(graphicsContext);

      myStage = Stage::WAITING_FOR_READBACK_COPY;
    }
    break;
    case Stage::WAITING_FOR_READBACK_COPY: 
    {
      if (RenderCore::IsFenceDone(myBufferCopyFence))
        myStage = Stage::COPY_DONE;
    }
    break;
    case Stage::COPY_DONE: 
    {
      const uint expectedValue = myExpectedBufferValue + 1;

      uint* bufferData = (uint*)myReadbackBuffer->Map(GpuResourceMapMode::READ_UNSYNCHRONIZED);
      bool hasExpectedData = true;
      uint bufferValue = 0;
      for (uint i = 0; hasExpectedData && i < kNumBufferElements; ++i)
      {
        bufferValue = bufferData[i];
        hasExpectedData &= bufferData[i] == expectedValue;
      }
      myReadbackBuffer->Unmap(GpuResourceMapMode::READ_UNSYNCHRONIZED);

      if (hasExpectedData)
      {
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF20a300);
        ImGui::Text("Async compute test passed!");
      }
      else
      {
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFFb22900);
        ImGui::Text("Async compute test failed! Expected: %d - Actual %d", expectedValue, bufferValue);
      }

      ImGui::PopStyleColor();

      myStage = Stage::IDLE;
    }
    break;
  }
}

void Test_AsyncCompute::OnRender()
{
  // LongGpuCopy(myDummyGpuBuffer1.get(), myDummyGpuBuffer2.get());
}
