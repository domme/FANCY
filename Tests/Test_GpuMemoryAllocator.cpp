#include "Test_GpuMemoryAllocator.h"

#include "fancy_core/MathUtil.h"
#include "fancy_core/Profiler.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_imgui/imgui.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/RenderCore_PlatformDX12.h"

using namespace Fancy;

Test_GpuMemoryAllocator::Test_GpuMemoryAllocator(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Profiler")
  , myBufferToAllocSizeMb(64)
{
  for (uint memType = 0; memType < (uint)Fancy::GpuMemoryType::NUM; ++memType)
    for (uint accessType = 0; accessType < (uint)Fancy::CpuMemoryAccessType::NUM; ++accessType)
      myAllocatorTypeVisible[memType][accessType] = false;
}

Test_GpuMemoryAllocator::~Test_GpuMemoryAllocator()
{
  RenderCore::WaitForIdle(CommandListType::Graphics);
}

void Test_GpuMemoryAllocator::OnUpdate(bool aDrawProperties)
{
  if (!aDrawProperties)
    return;

  ImGui::InputInt("Buffer Size (MiB)", &myBufferToAllocSizeMb);
  if (ImGui::Button("Allocate Gpu Buffer"))
  {
    GpuBufferProperties props;
    props.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
    props.myNumElements = 1u;
    props.myIsShaderWritable = false;
    props.myUsage = GpuBufferUsage::SHADER_BUFFER;
    props.myElementSizeBytes = myBufferToAllocSizeMb * SIZE_MB;

    String name = StringFormat("Gpu-buffer % MiB", myBufferToAllocSizeMb);
    myBuffers.push_back(RenderCore::CreateBuffer(props, name.c_str()));
  }

  if (!myBuffers.empty() && ImGui::Button("Remove last GpuBuffer"))
  {
    myBuffers.erase(myBuffers.begin() + myBuffers.size() - 1);
  }

  if (!myBuffers.empty() && ImGui::Button("Remove all GpuBuffers"))
    myBuffers.clear();

  ImGui::Text("%d Buffers allocated:", myBuffers.size());

  for (const SharedPtr<GpuBuffer>& buffer : myBuffers)
    ImGui::Text("\t%s", buffer->myName.c_str());

  RenderMemoryAllocatorLayouts();
}

void Test_GpuMemoryAllocator::OnRender()
{
}

const char* locMemoryTypeToString(GpuMemoryType aType)
{
  switch (aType)
  {
  case GpuMemoryType::BUFFER: return "buffer";
  case GpuMemoryType::TEXTURE: return "texture";
  case GpuMemoryType::RENDERTARGET: return "rendertarget";
  default: return "";
  }
}

const char* locCpuAccessTypeToString(CpuMemoryAccessType aType)
{
  switch (aType)
  {
  case CpuMemoryAccessType::NO_CPU_ACCESS: return "default";
  case CpuMemoryAccessType::CPU_WRITE: return "write";
  case CpuMemoryAccessType::CPU_READ: return "read";
  default: return "";
  }
}

void locDebugPrintMemoryAllocatorDx12(GpuMemoryAllocatorDX12* anAllocatorDx12)
{
  std::stringstream debugStr;
  debugStr << "Num Pages: " << anAllocatorDx12->myAllocator.myPages.size() << std::endl;
  debugStr << "Free list: " << std::endl;
  int oldPageIdx = -2;
  uint64 lastElementEnd = 0;
  for (auto it = anAllocatorDx12->myAllocator.myFreeList.Begin(); it != anAllocatorDx12->myAllocator.myFreeList.Invalid(); ++it)
  {
    int pageIdx = -1;
    for (int i = 0; i < anAllocatorDx12->myAllocator.myPages.size(); ++i)
    {
      if (anAllocatorDx12->myAllocator.IsBlockInPage(*it, anAllocatorDx12->myAllocator.myPages[i]))
      {
        pageIdx = i;
        break;
      }
    }

    if (oldPageIdx != pageIdx)
      debugStr << "|| Page " << pageIdx << ": ";
    oldPageIdx = pageIdx;

    if (lastElementEnd != it->myStart)
      debugStr << "[X: " << (it->myStart - lastElementEnd) << "]";
    lastElementEnd = it->myEnd;

    debugStr << "[" << it->myStart << ".." << it->myEnd << "]";
  }

  ImGui::Text(debugStr.str().c_str());
}

void Test_GpuMemoryAllocator::RenderMemoryAllocatorLayouts()
{
  RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();
  if (platformDx12 == nullptr)
    return;

  for (uint memType = 0; memType < (uint)Fancy::GpuMemoryType::NUM; ++memType)
  {
    if (ImGui::TreeNode(locMemoryTypeToString((GpuMemoryType) memType)))
    {
      for (uint accessType = 0; accessType < (uint)Fancy::CpuMemoryAccessType::NUM; ++accessType)
      {
        if(ImGui::TreeNode(locCpuAccessTypeToString((CpuMemoryAccessType) accessType)))
        {
          GpuMemoryAllocatorDX12* allocatorDx12 = platformDx12->myGpuMemoryAllocators[memType][accessType].get();
          locDebugPrintMemoryAllocatorDx12(allocatorDx12);
          ImGui::TreePop();
        }
      }
      ImGui::TreePop();
    }
  }
}
