#include "Test_GpuMemoryAllocator.h"

#include "fancy_core/MathUtil.h"
#include "fancy_core/Profiler.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_imgui/imgui.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/RenderCore_PlatformDX12.h"
#include "fancy_core/StringUtil.h"
#include "fancy_core/PagedLinearAllocator.h"
#include "fancy_core/StaticString.h"

using namespace Fancy;

Test_GpuMemoryAllocator::Test_GpuMemoryAllocator(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Profiler")
  , myBufferToAllocSizeMb(64)
  , myScale(10.0f)
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

  ImGui::NewLine();

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

const char* locGetMemoryLabel(uint64 aMemorySize, StringPool& aStringPool)
{
  if (aMemorySize >= SIZE_MB)
    return aStringPool.Format("%d MiB", aMemorySize / SIZE_MB);
  if (aMemorySize >= SIZE_KB)
    return aStringPool.Format("%d KiB", aMemorySize / SIZE_KB);

  return aStringPool.Format("%d Byte", aMemorySize);
}

void locDebugPrintMemoryAllocatorDx12(GpuMemoryAllocatorDX12* anAllocatorDx12, float aMemoryToPixelScale)
{
  auto& allocator = anAllocatorDx12->myAllocator;

  const float elementHeight = 20.0f;

  const ImVec2 startPos = ImGui::GetCursorPos();
  ImVec2 pos = startPos;
  for (uint i = 0u; i < allocator.myPages.size(); ++i)
  {
    const auto& page = allocator.myPages[i];
    const float pixelWidth = aMemoryToPixelScale * (float)(page.myEnd - page.myStart);
    if (pixelWidth >= 2)
    {
      StringPool stringPool;
      ImGui::SetCursorPos(pos);
      ImGui::Button(stringPool.Format("Heap %d", i), ImVec2(pixelWidth - 1, elementHeight));
      if (ImGui::IsItemHovered())
      {
        StaticString<128> str("Start: %s\nEnd:%s\nSize:%s", 
          locGetMemoryLabel(page.myStart, stringPool), locGetMemoryLabel(page.myEnd, stringPool), locGetMemoryLabel(page.myEnd - page.myStart, stringPool));
        ImGui::SetTooltip(str);
      }
    }
    pos.x += pixelWidth;
  }

  pos = startPos;
  pos.y += elementHeight + 2.0f;
  ImGui::PushStyleColor(ImGuiCol_Button, 0xFFAAAAAA);
  for (auto freeBlockIt = allocator.myFreeList.Begin(); freeBlockIt != allocator.myFreeList.Invalid(); ++freeBlockIt)
  {
    const uint64 freeMemory = freeBlockIt->myEnd - freeBlockIt->myStart;
    const float pixelWidth = aMemoryToPixelScale * (float)freeMemory;
    if (pixelWidth < 2)
      continue;

    StringPool stringPool;
    ImVec2 currPos = pos;
    currPos.x = startPos.x + (float)(freeBlockIt->myStart * aMemoryToPixelScale);
    ImGui::SetCursorPos(currPos);
    ImGui::Button(stringPool.Format("%s", locGetMemoryLabel(freeMemory, stringPool)), ImVec2(pixelWidth-1, elementHeight));
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip(stringPool.Format("Free Block\nStart: %s\nEnd: %s\n Size: %s",
        locGetMemoryLabel(freeBlockIt->myStart, stringPool), locGetMemoryLabel(freeBlockIt->myEnd, stringPool), locGetMemoryLabel(freeMemory, stringPool)));
  }
  ImGui::PopStyleColor(1);
  
  ImGui::PushStyleColor(ImGuiCol_Button, 0xFFFFAAAA);
  for (const auto& debugInfo : anAllocatorDx12->myAllocDebugInfos)
  {
    const uint64 allocatedMemory = debugInfo.myEnd - debugInfo.myStart;
    const float pixelWidth = aMemoryToPixelScale * (float)allocatedMemory;
    if (pixelWidth < 2)
      continue;

    StringPool stringPool;
    ImVec2 currPos = pos;
    currPos.x = startPos.x + (float)(debugInfo.myStart * aMemoryToPixelScale);
    ImGui::SetCursorPos(currPos);
    ImGui::Button(stringPool.Format("%s", locGetMemoryLabel(allocatedMemory, stringPool)), ImVec2(pixelWidth-1, elementHeight));
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip(stringPool.Format("Allocated Block %s\nStart: %s\nEnd: %s\nSize: %s",
        debugInfo.myName.c_str(), locGetMemoryLabel(debugInfo.myStart, stringPool), locGetMemoryLabel(debugInfo.myEnd, stringPool), locGetMemoryLabel(allocatedMemory, stringPool)));
  }
  ImGui::PopStyleColor(1);
}

void Test_GpuMemoryAllocator::RenderMemoryAllocatorLayouts()
{
  RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();
  if (platformDx12 == nullptr)
    return;

  ImGui::SliderFloat("Scale", &myScale, 0.1f, 2000.0f);

  ImGui::BeginChildFrame(1, ImVec2(ImGui::GetWindowWidth(), 512), ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground);
  for (uint memType = 0; memType < (uint)Fancy::GpuMemoryType::NUM; ++memType)
  {
    if (ImGui::TreeNode(locMemoryTypeToString((GpuMemoryType) memType)))
    {
      for (uint accessType = 0; accessType < (uint)Fancy::CpuMemoryAccessType::NUM; ++accessType)
      {
        ImGui::Text(locCpuAccessTypeToString((CpuMemoryAccessType)accessType));
        GpuMemoryAllocatorDX12* allocatorDx12 = platformDx12->myGpuMemoryAllocators[memType][accessType].get();
        locDebugPrintMemoryAllocatorDx12(allocatorDx12, myScale / SIZE_MB);
      }
      ImGui::TreePop();
    }
  }
  ImGui::EndChildFrame();
}
