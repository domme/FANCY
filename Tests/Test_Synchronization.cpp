#include "Test_Synchronization.h"

#include "fancy_core/MathUtil.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_imgui/imgui.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/CommandContext.h"
#include "fancy_core/CommandQueue.h"
#include "fancy_imgui/imgui_internal.h"
#include "fancy_core/GrowingList.h"

using namespace Fancy;

void LongGpuCopy(GpuBuffer* aSrcBuffer, GpuBuffer* aDstBuffer)
{
  CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
  CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);

  ctx->CopyBufferRegion(aDstBuffer, 0u, aSrcBuffer, 0u, aSrcBuffer->GetByteSize());

  queue->ExecuteContext(ctx);
  RenderCore::FreeContext(ctx);
}

Test_Synchronization::Test_Synchronization(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Synchronization")
{
  GpuBufferProperties props;
  props.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
  props.myElementSizeBytes = sizeof(uint);
  props.myNumElements = 1000;
  props.myIsShaderWritable = false;
  props.myUsage = GpuBufferUsage::VERTEX_BUFFER;

  std::vector<uint> initialData;
  initialData.resize(props.myNumElements);
  uint i = 0;
  for (uint& data : initialData)
    data = i++;

  myBufferCpuSync = RenderCore::CreateBuffer(props, "CPU Sync buffer", initialData.data());
  ASSERT(myBufferCpuSync);
  myBufferAsyncGpu = RenderCore::CreateBuffer(props, "Async GPU Sync buffer", initialData.data());
  ASSERT(myBufferAsyncGpu);
}

Test_Synchronization::~Test_Synchronization()
{
  RenderCore::WaitForIdle(CommandListType::Graphics);
}

void Test_Synchronization::OnUpdate(bool aDrawProperties)
{
  if (aDrawProperties)
  {
    ImGui::Checkbox("Perfom CPU sync test", &myTestCpuGpu);
    ImGui::Checkbox("Perfom Async GPU sync test", &myTestAsnycGpuGpu);
  }

  if (myTestCpuGpu)
  {
    
  }
}

void Test_Synchronization::OnRender()
{
  // LongGpuCopy(myDummyGpuBuffer1.get(), myDummyGpuBuffer2.get());
}
