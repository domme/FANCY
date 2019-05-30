#include "Test_Profiler.h"

#include "fancy_core/MathUtil.h"
#include "fancy_core/Profiler.h"
#include "fancy_core/Annotations.h"
#include "app_framework/ProfilerWindow.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_imgui/imgui.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/CommandContext.h"
#include "fancy_core/CommandQueue.h"
#include "fancy_imgui/imgui_internal.h"
#include "fancy_core/GrowingList.h"

using namespace Fancy;

ANNOTATION_CREATE_TAG(ANNTAG_PROFILER_TEST, "ProfilerTest", 0xFF00FF00);

ANNOTATION_CREATE_TAG(ANNTAG_GROWINGLIST, "GrowingList", 0xFFf4a442);
ANNOTATION_CREATE_TAG(ANNTAG_LIST, "List", 0xFF48f442);

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
  CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
  CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
  
  GPU_BEGIN_PROFILE_FUNCTION_TAG(ctx, ANNTAG_PROFILER_TEST);
  ctx->CopyBufferRegion(aDstBuffer, 0u, aSrcBuffer, 0u, aSrcBuffer->GetByteSize());
  GPU_END_PROFILE(ctx);

  queue->ExecuteContext(ctx);
  RenderCore::FreeContext(ctx);
}

struct TestStruct
{
  uint64 myDataA;
  uint64 myDataB;
  uint64 myDataC;
};

Fancy::GrowingList<TestStruct, 64> myGrowingList;
std::list<TestStruct> myStlList;

void GrowingList_Iteration()
{
  PROFILE_FUNCTION_TAG(ANNTAG_GROWINGLIST);

  uint64 dataSum = 0;
  for (auto it = myGrowingList.Begin(); it != myGrowingList.Invalid(); ++it)
    dataSum += it->myDataA + it->myDataB + it->myDataC;
}

void StdList_Iteration()
{
  PROFILE_FUNCTION_TAG(ANNTAG_LIST);

  uint64 dataSum = 0;
  for (auto it = myStlList.begin(); it != myStlList.end(); ++it)
    dataSum += it->myDataA + it->myDataB + it->myDataC;
}

void GrowingList_DeleteInsert()
{
  PROFILE_FUNCTION_TAG(ANNTAG_GROWINGLIST);

  const uint numElements = myGrowingList.Size();
  {
    Profiler::ScopedMarker marker("GrowingList Delete", ANNTAG_GROWINGLIST);
    while (!myGrowingList.IsEmpty())
    {
      uint idx = std::rand() % myGrowingList.Size();
      auto it = myGrowingList.FindAtIndex(idx);
      ASSERT(it);
      myGrowingList.Remove(it);
    }
  }

  {
    Profiler::ScopedMarker marker("GrowingList Fill", ANNTAG_GROWINGLIST);
    for (uint i = 0; i < numElements; ++i)
    {
      myGrowingList.Add(TestStruct());
    }
  }
}

void StdList_DeleteInsert()
{
  PROFILE_FUNCTION_TAG(ANNTAG_LIST);

  auto GetIterator = [&](uint idx)
  {
    auto it = myStlList.begin();
    while (idx != 0)
    {
      ++it;
      --idx;
    }
    return it;
  };

  const uint numElements = myStlList.size();
  {
    Profiler::ScopedMarker marker("StdList Delete", ANNTAG_LIST);
    while (!myStlList.empty())
    {
      const uint idx = std::rand() % myStlList.size();
      myStlList.erase(GetIterator(idx));
    }
  }

  {
    Profiler::ScopedMarker marker("StdList Fill", ANNTAG_LIST);
    for (uint i = 0; i < numElements; ++i)
    {
      myStlList.push_back(TestStruct());
    }
  }
}

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

  for (uint i = 0; i < 1000; ++i)
  {
    TestStruct testData{};
    myGrowingList.Add(testData);
    myStlList.push_back(testData);
  }
}

Test_Profiler::~Test_Profiler()
{
  RenderCore::WaitForIdle(CommandListType::Graphics);
}

void Test_Profiler::OnUpdate(bool aDrawProperties)
{
  //LongFunc();
  //LongFunc();

  GrowingList_Iteration();
  StdList_Iteration();

  GrowingList_DeleteInsert();
  StdList_DeleteInsert();

  if (aDrawProperties && ImGui::Button("Toggle Profiler Window"))
    myShowProfilerWindow ^= 1;

  if (myShowProfilerWindow)
    myProfilerWindow.Render();
}

void Test_Profiler::OnRender()
{
 // LongGpuCopy(myDummyGpuBuffer1.get(), myDummyGpuBuffer2.get());
}
