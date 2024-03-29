#pragma once

#include "Rendering/CommandList.h"
#include "Rendering/RenderEnums.h"

#include "DX12Prerequisites.h"
#include "eastl/fixed_hash_map.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  class DescriptorDX12;
  class ShaderVisibleDescriptorHeapDX12;
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
    void ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags) override;
    void CopyResource(GpuResource* aDstResource, GpuResource* aSrcResource) override;
    void CopyBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize) override;
    void CopyTextureToBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) override;
    void CopyTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) override;
    void CopyBufferToTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset) override;
    void UpdateTextureData(const Texture* aDstTexture, const SubresourceRange& aSubresourceRange, const TextureSubData* someDatas, uint aNumDatas /*, const TextureRegion* someRegions = nullptr */) override; // TODO: Support regions

    void PostExecute(uint64 aFenceVal) override;
    void ResetAndOpen() override;
    void FlushBarriers() override;

    void BindVertexBuffers(const GpuBuffer** someBuffers, uint64* someOffsets, uint64* someSizes, uint aNumBuffers, const VertexInputLayout* anInputLayout = nullptr) override;
    void BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) override;
    void BindLocalBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint aRegisterIndex) override;

    GpuQuery BeginQuery(GpuQueryType aType) override;
    void EndQuery(const GpuQuery& aQuery) override;
    GpuQuery InsertTimestamp() override;
    void CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset) override;
    void BeginMarkerRegion(const char* aName, uint aColor) override;
    void EndMarkerRegion() override;

    void TransitionResource(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, ResourceTransition aTransition, uint someUsageFlags = 0u) override;
    void PrepareResourceShaderAccess(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, ShaderResourceAccess aTransition) override;
    void ResourceUAVbarrier(const GpuResource** someResources = nullptr, uint aNumResources = 0u) override;

    void Close() override;

    void DrawInstanced(uint aNumVerticesPerInstance, uint aNumInstances, uint aBaseVertex, uint aStartInstance) override;
    void DrawIndexedInstanced(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance) override;
    void Dispatch(const glm::int3& aNumThreads) override;
    void DispatchRays(const DispatchRaysDesc& aDesc) override;

    void TrackResourceTransition(const GpuResource* aResource, D3D12_RESOURCE_STATES aNewState, bool aIsSharedReadState = false);
    void TrackSubresourceTransition(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, D3D12_RESOURCE_STATES aNewState, bool aToSharedReadState = false);
    void AddBarrier(const D3D12_RESOURCE_BARRIER& aBarrier);

    ID3D12GraphicsCommandList6* GetDX12CommandList() const { return myCommandList; }

  protected:
    static D3D12_DESCRIPTOR_HEAP_TYPE ResolveDescriptorHeapTypeFromMask(uint aDescriptorTypeMask);

    void PrepareForRecord(bool aResetCommandList);

    void ApplyViewportAndClipRect();
    void ApplyGraphicsPipelineState();
    void ApplyComputePipelineState();
    void ApplyRaytracingPipelineState();
    void ApplyRenderTargets();
    void ApplyTopologyType();
    void ApplyResourceBindings();
    
    bool GetLocalSubresourceStates(const GpuResource* aResource, SubresourceLocation aSubresource, D3D12_RESOURCE_STATES& aStatesOut);
    D3D12_RESOURCE_STATES ResolveValidateDstStates(const GpuResource* aResource, D3D12_RESOURCE_STATES aDstStates);
    bool ValidateSubresourceTransition(const GpuResource* aResource, uint aSubresourceIndex, D3D12_RESOURCE_STATES aDstStates);

    ID3D12GraphicsCommandList6* myCommandList;
    ID3D12CommandAllocator* myCommandAllocator;
    eastl::fixed_vector<D3D12_RESOURCE_BARRIER, kNumCachedBarriers, false> myPendingBarriers;
    eastl::fixed_vector<uint64, 16> myLocalBuffersToBind;
    eastl::fixed_vector<uint64, 16> myLocalRWBuffersToBind;
    eastl::fixed_vector<uint64, 16> myLocalCBuffersToBind;

    D3D12_RESOURCE_STATES myResourceStateMask;

    struct SubresourceHazardData
    {
      SubresourceHazardData()
        : myWasWritten(false)
        , myWasUsed(false)
        , myIsSharedReadState(false)
      { }

      // TODO: Those two could be reduced to only 24 bits
      D3D12_RESOURCE_STATES myFirstDstStates = (D3D12_RESOURCE_STATES) 0;
      D3D12_RESOURCE_STATES myStates = (D3D12_RESOURCE_STATES) 0;
      uint myWasWritten : 1;
      uint myWasUsed : 1;
      uint myIsSharedReadState : 1;
    };
    struct LocalHazardData
    {
      eastl::fixed_vector<SubresourceHazardData, 16> mySubresources;
    };

    // TODO: Make this a simple vector and compare perf. Most likely a vector will be much faster in almost all cases
    eastl::fixed_hash_map<const GpuResource*, LocalHazardData, kNumExpectedResourcesPerDispatch> myLocalHazardData;
  };
//---------------------------------------------------------------------------//
}

#endif