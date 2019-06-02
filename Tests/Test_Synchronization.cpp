#include "Test_Synchronization.h"

#include "fancy_core/MathUtil.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_imgui/imgui.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/CommandList.h"
#include "fancy_core/CommandQueue.h"
#include "fancy_imgui/imgui_internal.h"
#include "fancy_core/GrowingList.h"
#include "fancy_core/TimeManager.h"
#include "fancy_core/CommandContext.h"

using namespace Fancy;

static uint kNumBufferElements = 1000;

Test_Synchronization::Test_Synchronization(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Synchronization")
{
  GpuBufferProperties props;
  props.myElementSizeBytes = sizeof(uint);
  props.myNumElements = kNumBufferElements;
  props.myIsShaderWritable = false;
  props.myUsage = GpuBufferUsage::VERTEX_BUFFER;

  std::vector<uint> initialData;
  initialData.resize(props.myNumElements);
  for (uint& data : initialData)
    data = 0;
  
  props.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
  myUploadBuffer = RenderCore::CreateBuffer(props, "Sync-tests upload buffer", initialData.data());

  props.myCpuAccess = CpuMemoryAccessType::CPU_READ;
  myReadbackBuffer = RenderCore::CreateBuffer(props, "Sync-tests readback buffer", initialData.data());
}

Test_Synchronization::~Test_Synchronization()
{
  RenderCore::WaitForIdle(CommandListType::Graphics);
  RenderCore::WaitForIdle(CommandListType::Compute);
}

void Test_Synchronization::OnUpdate(bool aDrawProperties)
{
  ImGui::Checkbox("Wait for results", &myWaitForResults);

  switch (myStage) 
  { 
    case Stage::IDLE: 
    {
      myExpectedBufferValue = Time::ourFrameIdx;
      uint* bufferData = (uint*)myUploadBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
      for (uint i = 0; i < kNumBufferElements; ++i)
        bufferData[i] = myExpectedBufferValue;
      myUploadBuffer->Unmap(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);

      CommandContext ctx(CommandListType::Graphics);
      ctx->CopyBufferRegion(myReadbackBuffer.get(), 0ull, myUploadBuffer.get(), 0ull, myUploadBuffer->GetByteSize());
      ctx.Execute();
    }
    break;
    case Stage::WAITING_FOR_COPY: break;
    case Stage::COPY_DONE: break;
    default: ;
  }

    // Expected behavior:
  // 1) Write to A on CPU
  // 2) Copy A->B on GPU
  // 3) Read B on CPU -> Verify that its the content that was written into A before

  


}

void Test_Synchronization::OnRender()
{
  // LongGpuCopy(myDummyGpuBuffer1.get(), myDummyGpuBuffer2.get());
}
