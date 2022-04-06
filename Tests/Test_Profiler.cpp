#include "Test_Profiler.h"

#include "Common/MathUtil.h"
#include "Debug/Profiler.h"
#include "Debug/Annotations.h"
#include "Rendering/GpuBuffer.h"
#include "imgui.h"
#include "Rendering/RenderCore.h"
#include "Rendering/CommandList.h"
#include "imgui_internal.h"
#include "Common/GrowingList.h"

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
  CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
  GPU_BEGIN_PROFILE_FUNCTION_TAG(ctx, ANNTAG_PROFILER_TEST);
  ctx->CopyBuffer(aDstBuffer, 0u, aSrcBuffer, 0u, aSrcBuffer->GetByteSize());
  GPU_END_PROFILE(ctx);
  RenderCore::ExecuteAndFreeCommandList(ctx);
}

struct TestStruct
{
  uint64 myDataA;
  uint64 myDataB;
  uint64 myDataC;
};

Test_Profiler::Test_Profiler(Fancy::AssetManager* anAssetManager, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(anAssetManager, aWindow, aRenderOutput, anInputState, "Profiler")
{
  GpuBufferProperties props;
  props.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
  props.myElementSizeBytes = 50 * SIZE_MB;
  props.myNumElements = 1u;
  props.myIsShaderWritable = false;
  props.myBindFlags = (uint) GpuBufferBindFlags::SHADER_BUFFER;
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
