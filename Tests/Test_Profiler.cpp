#include "Test_Profiler.h"

#include "fancy_core/MathUtil.h"
#include "fancy_core/Profiler.h"
#include "fancy_core/Annotations.h"
#include "app_framework/ProfilerWindow.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_imgui/imgui.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/CommandList.h"
#include "fancy_core/CommandQueue.h"
#include "fancy_imgui/imgui_internal.h"
#include "fancy_core/GrowingList.h"
#include "fancy_core/CommandContext.h"

using namespace Fancy;

ANNOTATION_CREATE_TAG(ANNTAG_PROFILER_TEST, "ProfilerTest", 0xFF00FF00);

void ShortFunc()
{
  PROFILE_FUNCTION();

  int i = 0;
  while (i < 9000)
    ++i;
}

void MediumFunc()
{
  PROFILE_FUNCTION();

  uint64 hash = 0u;
  int i = 0;
  while (i < 999)
    MathUtil::hash_combine(hash, i++);

  ShortFunc();
}

void LongFunc()
{
  PROFILE_FUNCTION_TAG(ANNTAG_PROFILER_TEST);

  uint64 hash = 0u;
  int i = 0;
  while (i < 9999)
    MathUtil::hash_combine(hash, i++);

  MediumFunc();
}

void LongGpuCopy(GpuBuffer* aSrcBuffer, GpuBuffer* aDstBuffer)
{
  CommandContext ctx(CommandListType::Graphics);
  GPU_BEGIN_PROFILE_FUNCTION_TAG(ctx.GetCommandList(), ANNTAG_PROFILER_TEST);
  ctx->CopyBufferRegion(aDstBuffer, 0u, aSrcBuffer, 0u, aSrcBuffer->GetByteSize());
  GPU_END_PROFILE(ctx.GetCommandList());
  ctx.Execute();
}

struct TestStruct
{
  uint64 myDataA;
  uint64 myDataB;
  uint64 myDataC;
};

Test_Profiler::Test_Profiler(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Profiler")
{
  GpuBufferProperties props;
  props.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
  props.myElementSizeBytes = 50 * SIZE_MB;
  props.myNumElements = 1u;
  props.myIsShaderWritable = false;
  props.myUsage = GpuBufferUsage::SHADER_BUFFER;
  myDummyGpuBuffer1 = RenderCore::CreateBuffer(props, "TestItem_Profiler_DummyBuffer1");
  ASSERT(myDummyGpuBuffer1 != nullptr, "Test Profiler failed: Unable to create gpu dummy buffer");
  myDummyGpuBuffer2 = RenderCore::CreateBuffer(props, "TestItem_Profiler_DummyBuffer1");
  ASSERT(myDummyGpuBuffer2 != nullptr, "Test Profiler failed: Unable to create gpu dummy buffer");
}

Test_Profiler::~Test_Profiler()
{
  RenderCore::WaitForIdle(CommandListType::Graphics);
}

void Test_Profiler::OnUpdate(bool aDrawProperties)
{
  LongFunc();
  LongFunc();

  if (aDrawProperties && ImGui::Button("Toggle Profiler Window"))
    myShowProfilerWindow ^= 1;

  if (myShowProfilerWindow)
    myProfilerWindow.Render();
}

void Test_Profiler::OnRender()
{
  LongGpuCopy(myDummyGpuBuffer1.get(), myDummyGpuBuffer2.get());
}
