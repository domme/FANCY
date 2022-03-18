#include "Test_GpuMemoryAllocator.h"

#include "Common/MathUtil.h"
#include "Debug/Profiler.h"
#include "Rendering/GpuBuffer.h"
#include "imgui.h"
#include "Rendering/RenderCore.h"
#include "Rendering/DX12/RenderCore_PlatformDX12.h"
#include "Common/StringUtil.h"
#include "Common/PagedLinearAllocator.h"
#include "Common/StaticString.h"
#include "Rendering/RenderCore.h"
#include "Rendering/RenderEnums.h"

using namespace Fancy;

Test_GpuMemoryAllocator::Test_GpuMemoryAllocator(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Application(aRuntime, aWindow, aRenderOutput, anInputState, "GPU Memory Allocations")
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
    props.myElementSizeBytes = myBufferToAllocSizeMb * SIZE_MB;

    eastl::string name(StaticString<64>("Gpu-buffer %d MiB", myBufferToAllocSizeMb));
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

const char* locGetMemoryLabel(uint64 aMemorySize, CircularStringBuffer& aStringPool)
{
  if (aMemorySize >= SIZE_MB)
    return aStringPool.Format("%d MiB", aMemorySize / SIZE_MB);
  if (aMemorySize >= SIZE_KB)
    return aStringPool.Format("%d KiB", aMemorySize / SIZE_KB);

  return aStringPool.Format("%d Byte", aMemorySize);
}

#if FANCY_ENABLE_DX12
void locDebugPrintMemoryAllocatorDx12(GpuMemoryAllocatorDX12* anAllocatorDx12, float aMemoryToPixelScale)
{
  PagedLinearAllocator& allocator = *anAllocatorDx12;
  
  const float elementHeight = 20.0f;
  CircularStringBuffer stringBuffer;

  const ImVec2 startPos = ImGui::GetCursorPos();
  ImVec2 pos = startPos;
  for (uint i = 0u; i < allocator.myPages.size(); ++i)
  {
    const auto& page = allocator.myPages[i];
    const float pixelWidth = aMemoryToPixelScale * (float)(page.myEnd - page.myStart);
    if (pixelWidth >= 2)
    {
      ImGui::SetCursorPos(pos);
      ImGui::Button(stringBuffer.Format("Heap %d", i), ImVec2(pixelWidth - 1, elementHeight));
      if (ImGui::IsItemHovered())
      {
        StaticString<128> str("Start: %s\nEnd:%s\nSize:%s", 
          locGetMemoryLabel(page.myStart, stringBuffer), locGetMemoryLabel(page.myEnd, stringBuffer), locGetMemoryLabel(page.myEnd - page.myStart, stringBuffer));
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

    ImVec2 currPos = pos;
    currPos.x = startPos.x + (float)(freeBlockIt->myStart * aMemoryToPixelScale);
    ImGui::SetCursorPos(currPos);
    ImGui::Button(stringBuffer.Format("%s", locGetMemoryLabel(freeMemory, stringBuffer)), ImVec2(pixelWidth-1, elementHeight));
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip(stringBuffer.Format("Free Block\nStart: %s\nEnd: %s\n Size: %s",
        locGetMemoryLabel(freeBlockIt->myStart, stringBuffer), locGetMemoryLabel(freeBlockIt->myEnd, stringBuffer), locGetMemoryLabel(freeMemory, stringBuffer)));
  }
  ImGui::PopStyleColor(1);
  
  ImGui::PushStyleColor(ImGuiCol_Button, 0xFFFFAAAA);
  for (const auto& debugInfo : allocator.myAllocDebugInfos)
  {
    const uint64 allocatedMemory = debugInfo.myEnd - debugInfo.myStart;
    const float pixelWidth = aMemoryToPixelScale * (float)allocatedMemory;
    if (pixelWidth < 2)
      continue;

    ImVec2 currPos = pos;
    currPos.x = startPos.x + (float)(debugInfo.myStart * aMemoryToPixelScale);
    ImGui::SetCursorPos(currPos);
    ImGui::Button(stringBuffer.Format("%s", locGetMemoryLabel(allocatedMemory, stringBuffer)), ImVec2(pixelWidth-1, elementHeight));
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip(stringBuffer.Format("Allocated Block %s\nStart: %s\nEnd: %s\nSize: %s",
        debugInfo.myName.c_str(), locGetMemoryLabel(debugInfo.myStart, stringBuffer), locGetMemoryLabel(debugInfo.myEnd, stringBuffer), locGetMemoryLabel(allocatedMemory, stringBuffer)));
  }
  ImGui::PopStyleColor(1);
}

#endif


void Test_GpuMemoryAllocator::RenderMemoryAllocatorLayouts()
{
#if FANCY_ENABLE_DX12
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
#endif
}


