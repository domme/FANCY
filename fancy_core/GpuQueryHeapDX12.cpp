#include "fancy_core_precompile.h"
#include "GpuQueryHeapDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy
{
  GpuQueryHeapDX12::GpuQueryHeapDX12(GpuQueryType aQueryType, uint aNumQueries)
    : GpuQueryHeap(aQueryType, aNumQueries)
  {
    D3D12_QUERY_HEAP_DESC heapDesc;
    memset(&heapDesc, 0, sizeof(heapDesc));
    heapDesc.Count = aNumQueries;
    heapDesc.Type = Adapter::ResolveQueryHeapType(aQueryType);

    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();
    CheckD3Dcall(platformDx12->GetDevice()->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&myHeap)));
  }

  GpuQueryHeapDX12::~GpuQueryHeapDX12()
  {
  }
}
