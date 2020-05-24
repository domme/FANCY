#include "fancy_core_precompile.h"
#include "GpuQueryHeapVk.h"
#include "RenderCore_PlatformVk.h"
#include "RenderCore.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuQueryHeapVk::GpuQueryHeapVk(GpuQueryType aQueryType, uint aNumQueries)
    : GpuQueryHeap(aQueryType, aNumQueries)
    , myQueryPool(nullptr)
  {
    VkQueryPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    createInfo.flags = 0u;
    createInfo.pNext = nullptr;
    createInfo.queryType = RenderCore_PlatformVk::ResolveQueryType(aQueryType);
    createInfo.queryCount = aNumQueries;

    ASSERT(createInfo.queryType != VK_QUERY_TYPE_PIPELINE_STATISTICS);  // If a pipeline statistics query is requested, the pipelineStatistics flags below need to be set.
    createInfo.pipelineStatistics = 0u;

    ASSERT_VK_RESULT(vkCreateQueryPool(RenderCore::GetPlatformVk()->GetDevice(), &createInfo, nullptr, &myQueryPool));
    vkResetQueryPool(RenderCore::GetPlatformVk()->GetDevice(), myQueryPool, 0u, aNumQueries);
  }
//---------------------------------------------------------------------------//
  GpuQueryHeapVk::~GpuQueryHeapVk()
  {
    vkDestroyQueryPool(RenderCore::GetPlatformVk()->GetDevice(), myQueryPool, nullptr);
  }
//---------------------------------------------------------------------------//
  void GpuQueryHeapVk::Reset(uint64 aFrame)
  {
    if (myNextFreeQuery > 0)
      vkResetQueryPool(RenderCore::GetPlatformVk()->GetDevice(), myQueryPool, 0u, myNextFreeQuery);

    GpuQueryHeap::Reset(aFrame);
  }
//---------------------------------------------------------------------------//
}

#endif