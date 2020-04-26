#pragma once

#include "CommandList.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"

#include "RenderEnums.h"
#include "StaticArray.h"
#include "ShaderResourceInfoDX12.h"
#include "RootSignatureDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  class DescriptorDX12;
  class DynamicDescriptorHeapDX12;
  class GpuResourceDX12;
  class GpuResource;
  struct GpuResourceDataDX12;
  class Shader;
  class GpuBuffer;
  struct RootSignatureLayoutDX12;
  struct RootSignatureBindingsDX12;
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
    void BindVertexBuffer(const GpuBuffer* aBuffer, uint aVertexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) override;
    void BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) override;
    void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance) override;
    
    void BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint64 aNameHash, uint anArrayIndex = 0u) override;
    void BindResourceView(const GpuResourceView* aView, uint64 aNameHash, uint anArrayIndex = 0u) override;
    void BindSampler(const TextureSampler* aSampler, uint64 aNameHash, uint anArrayIndex = 0u) override;

    GpuQuery BeginQuery(GpuQueryType aType) override;
    void EndQuery(const GpuQuery& aQuery) override;
    GpuQuery InsertTimestamp() override;
    void CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset) override;

    void TransitionResource(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, ResourceTransition aTransition, uint someUsageFlags = 0u) override;
    void ResourceUAVbarrier(const GpuResource** someResources = nullptr, uint aNumResources = 0u) override;

    void Close() override;

    void Dispatch(const glm::int3& aNumThreads) override;

    void TrackResourceTransition(const GpuResource* aResource, D3D12_RESOURCE_STATES aNewState, bool aIsSharedReadState = false);
    void TrackSubresourceTransition(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, D3D12_RESOURCE_STATES aNewState, bool aToSharedReadState = false);
    void AddBarrier(const D3D12_RESOURCE_BARRIER& aBarrier);

  protected:
    void SetShaderPipelineInternal(const ShaderPipeline* aPipeline, bool& aHasPipelineChangedOut) override;

    static D3D12_DESCRIPTOR_HEAP_TYPE ResolveDescriptorHeapTypeFromMask(uint aDescriptorTypeMask);
    static D3D12_GRAPHICS_PIPELINE_STATE_DESC GetNativePSOdesc(const GraphicsPipelineState& aState);
    static D3D12_COMPUTE_PIPELINE_STATE_DESC GetNativePSOdesc(const ComputePipelineState& aState);

    bool FindShaderResourceInfo(uint64 aNameHash, ShaderResourceInfoDX12& aResourceInfoOut) const;
    void BindInternal(const ShaderResourceInfoDX12& aResourceInfo, const DescriptorDX12& aDescriptor, uint64 aGpuVirtualAddress, uint anArrayIndex);
    void ClearResourceBindings();

    /*
    bool SubresourceBarrierInternal(
      const GpuResource* aResource,
      const SubresourceRange& aSubresourceRange,
      GpuResourceState aSrcState,
      GpuResourceState aDstState,
      CommandListType aSrcQueue,
      CommandListType aDstQueue) override;
      */

    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DynamicDescriptorHeapDX12* aDescriptorHeap);
    void ApplyDescriptorHeaps();
    void ApplyViewportAndClipRect();
    void ApplyGraphicsPipelineState();
    void ApplyComputePipelineState();
    void ApplyRenderTargets();
    void ApplyTopologyType();
    void ApplyResourceBindings();

    bool GetLocalSubresourceStates(const GpuResource* aResource, SubresourceLocation aSubresource, D3D12_RESOURCE_STATES& aStatesOut);
    D3D12_RESOURCE_STATES ResolveValidateDstStates(const GpuResource* aResource, D3D12_RESOURCE_STATES aDstStates);
    bool ValidateSubresourceTransition(const GpuResource* aResource, uint aSubresourceIndex, D3D12_RESOURCE_STATES aDstStates);

    // void SetTrackSubresourceTransitionBarrier(const GpuResource* aResource, uint16 aSubresourceIndex, D3D12_RESOURCE_STATES aNewState) const;
    // void SetTrackSubresourceTransitionBarriers(const GpuResource** someResources, const D3D12_RESOURCE_STATES* someNewStates, const uint16** someSubresourceLists, const uint* someNumSubresources, uint aNumStates) const;

    bool CreateDescriptorTable(const RootSignatureBindingsDX12::DescriptorTable& aTable, DescriptorDX12& aStartDescriptorOut);

    static std::unordered_map<uint64, ID3D12PipelineState*> ourPSOcache;

    const RootSignatureLayoutDX12* myRootSignatureLayout;
    RootSignatureBindingsDX12 myRootSignatureBindings;
    StaticArray<DescriptorDX12, 32> myTempAllocatedDescriptors;
    
    ID3D12RootSignature* myRootSignature;  // The rootSignature that is set on myCommandList
    ID3D12GraphicsCommandList* myCommandList;
    ID3D12CommandAllocator* myCommandAllocator;
    StaticArray<D3D12_RESOURCE_BARRIER, kNumCachedBarriers> myPendingBarriers;
    D3D12_RESOURCE_STATES myResourceStateMask;

    DynamicDescriptorHeapDX12* myDynamicShaderVisibleHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    std::vector<DynamicDescriptorHeapDX12*> myRetiredDescriptorHeaps; // TODO: replace vector with a smallObjectPool

    struct SubresourceHazardData
    {
      D3D12_RESOURCE_STATES myFirstDstStates = (D3D12_RESOURCE_STATES) 0;
      D3D12_RESOURCE_STATES myStates = (D3D12_RESOURCE_STATES) 0;
      bool myWasWritten = false;
      bool myWasUsed = false;
      bool myIsSharedReadState = false;
    };
    struct LocalHazardData
    {
      DynamicArray<SubresourceHazardData> mySubresources;
    };
    std::map<const GpuResource*, LocalHazardData> myLocalHazardData;
  };
//---------------------------------------------------------------------------//
}

#endif