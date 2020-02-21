#pragma once

#include "CommandList.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"

#include "GpuResourceDataDX12.h"
#include "RenderEnums.h"
#include "GpuResourceStateTracking.h"
#include <glm/detail/type_mat.hpp>
#include "StaticArray.h"
#include "ShaderResourceInfoDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class DescriptorDX12;
  class DynamicDescriptorHeapDX12;
  class GpuResourceDX12;
  class GpuResource;
  struct GpuResourceDataDX12;
  class Shader;
  class GpuBuffer;
//---------------------------------------------------------------------------//
  class CommandListDX12 final : public CommandList
  {
    friend class RenderCore_PlatformDX12;
    friend class CommandQueueDX12;

  public:
    CommandListDX12(CommandListType aType);
    ~CommandListDX12() override;

    void UpdateSubresources(ID3D12Resource* aDstResource, ID3D12Resource* aStagingResource, uint aFirstSubresourceIndex, uint aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas);
    
    void ClearRenderTarget(TextureView* aTextureView, const float* aColor) override;
    void ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags = (uint)DepthStencilClearFlags::CLEAR_ALL) override;
    void CopyResource(GpuResource* aDstResource, GpuResource* aSrcResource) override;
    void CopyBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize) override;
    void CopyTextureToBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) override;
    void CopyTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) override;
    void CopyBufferToTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset) override;
    void UpdateTextureData(const Texture* aDstTexture, const SubresourceRange& aSubresourceRange, const TextureSubData* someDatas, uint aNumDatas /*, const TextureRegion* someRegions = nullptr */) override; // TODO: Support regions

    void PostExecute(uint64 aFenceVal) override;
    void PreBegin() override;
    void FlushBarriers() override;
    void SetShaderPipeline(const SharedPtr<ShaderPipeline>& aShaderPipeline) override;
    void BindVertexBuffer(const GpuBuffer* aBuffer, uint aVertexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) override;
    void BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) override;
    void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance) override;
    void RenderGeometry(const GeometryData* pGeometry) override;

    void BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint64 aNameHash) override;
    void BindResourceView(const GpuResourceView* aView, uint64 aNameHash) override;
    void BindSampler(const TextureSampler* aSampler, uint64 aNameHash) override;

    GpuQuery BeginQuery(GpuQueryType aType) override;
    void EndQuery(const GpuQuery& aQuery) override;
    GpuQuery InsertTimestamp() override;
    void CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset) override;

    void ResourceUAVbarrier(const GpuResource** someResources = nullptr, uint aNumResources = 0u) override;

    void Close() override;

    void SetComputeProgram(const Shader* aProgram) override;
    void Dispatch(const glm::int3& aNumThreads) override;

  protected:
    struct ResourceState
    {
      struct RootDescriptor
      {
        ShaderResourceTypeDX12 myType;
        uint64 myGpuVirtualAddress;
      };

      struct DescriptorTable
      {
        DescriptorDX12* myDescriptors;
        uint myNumDescriptors;
      };

      struct RootParameter
      {
        bool myIsDescriptorTable;
        RootDescriptor myRootDescriptor;
        DescriptorTable myDescriptorTable;
      };

      StaticArray<DescriptorDX12, 32> myTempAllocatedDescriptors;
      StaticArray<DescriptorDX12, 256> myBoundDescriptorPool;
      RootParameter myRootParameters[256];
      uint myNumBoundRootParameters;
    };

    static D3D12_DESCRIPTOR_HEAP_TYPE ResolveDescriptorHeapTypeFromMask(uint aDescriptorTypeMask);
    static D3D12_GRAPHICS_PIPELINE_STATE_DESC GetNativePSOdesc(const GraphicsPipelineState& aState);
    static D3D12_COMPUTE_PIPELINE_STATE_DESC GetNativePSOdesc(const ComputePipelineState& aState);

    bool FindShaderResourceInfo(uint64 aNameHash, ShaderResourceInfoDX12& aResourceInfoOut) const;
    void BindInternal(const ShaderResourceInfoDX12& aResourceInfo, const DescriptorDX12& aDescriptor, uint64 aGpuVirtualAddress);
    void ClearResourceBindings();

    bool SubresourceBarrierInternal(
      const GpuResource* aResource,
      const SubresourceRange& aSubresourceRange,
      GpuResourceState aSrcState,
      GpuResourceState aDstState,
      CommandListType aSrcQueue,
      CommandListType aDstQueue) override;

    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DynamicDescriptorHeapDX12* aDescriptorHeap);
    void ApplyDescriptorHeaps();
    void ApplyViewportAndClipRect();
    void ApplyGraphicsPipelineState();
    void ApplyComputePipelineState();
    void ApplyRenderTargets();
    void ApplyTopologyType();
    void ApplyResourceBindings();

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    void SetTrackResourceTransitionBarrier(const GpuResource* aResource, D3D12_RESOURCE_STATES aNewState) const;
    void SetTrackResourceTransitionBarriers(const GpuResource** someResources, const D3D12_RESOURCE_STATES* someNewStates, uint aNumStates) const;
    void SetTrackSubresourceTransitionBarrier(const GpuResource* aResource, uint16 aSubresourceIndex, D3D12_RESOURCE_STATES aNewState) const;
    void SetTrackSubresourceTransitionBarriers(const GpuResource** someResources, const D3D12_RESOURCE_STATES* someNewStates, const uint16** someSubresourceLists, const uint* someNumSubresources, uint aNumStates) const;
#endif 

    DescriptorDX12 CopyDescriptorsToDynamicHeapRange(const DescriptorDX12* someResources, uint aResourceCount);

    static std::unordered_map<uint64, ID3D12PipelineState*> ourPSOcache;

    ResourceState myResourceState;
    
    ID3D12RootSignature* myRootSignature;  // The rootSignature that is set on myCommandList
    ID3D12RootSignature* myComputeRootSignature;
    ID3D12GraphicsCommandList* myCommandList;
    ID3D12CommandAllocator* myCommandAllocator;
    D3D12_RESOURCE_BARRIER myPendingBarriers[kNumCachedBarriers];
    uint myNumPendingBarriers;

    DynamicDescriptorHeapDX12* myDynamicShaderVisibleHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    std::vector<DynamicDescriptorHeapDX12*> myRetiredDescriptorHeaps; // TODO: replace vector with a smallObjectPool
  };
//---------------------------------------------------------------------------//
}
