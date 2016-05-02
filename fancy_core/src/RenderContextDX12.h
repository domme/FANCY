#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include <unordered_map>
#include "DepthStencilState.h"
#include "BlendState.h"
#include "GpuDynamicAllocatorDX12.h"
#include "VertexInputLayout.h"
#include "SmallObjectAllocator.h"
#include "ObjectPool.h"

namespace Fancy{ namespace Rendering{
class Descriptor;
class GpuResource;
class Renderer;
  class GpuProgramPipeline;
}}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorHeapDX12;
  class GpuResourceDX12;
  class CommandAllocatorPoolDX12;
//---------------------------------------------------------------------------//
  struct PipelineState
  {
    PipelineState();
    uint getHash();
    D3D12_GRAPHICS_PIPELINE_STATE_DESC GetNativePSOdesc();

    FillMode myFillMode;
    CullMode myCullMode;
    WindingOrder myWindingOrder;
    DepthStencilState myDepthStencilState;
    BlendState myBlendState;
    const GpuProgramPipeline* myGpuProgramPipeline;
    uint8 myNumRenderTargets;
    DataFormat myRTVformats[Constants::kMaxNumRenderTargets];
    DataFormat myDSVformat;
    
    bool myIsDirty : 1;
  };
//---------------------------------------------------------------------------//
  class RenderContextDX12
  {
  public:
    enum Constants
    {
      kMaxNumCachedResourceBarriers = 16
    };

    RenderContextDX12();
    explicit RenderContextDX12(Renderer& aRenderer);
    ~RenderContextDX12();
    
    // Root arguments:
    void SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const;
    void SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const;
    void SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const;
    void SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const;
    
    // Descriptor tables:
    void SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aDescriptorTypeMask, uint32 aRegisterIndex);
        
    void SetGpuProgramPipeline(const GpuProgramPipeline* pProgramPipeline);

    void ClearRenderTarget(Texture* aTexture, const float* aColor);
    void ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags = (uint32) DepthStencilClearFlags::CLEAR_ALL);
    
    // It might be ok to keep these state-modifiers the way they are for a more modern approach
    void setViewport(const glm::uvec4& uViewportParams); /// x, y, width, height
    const glm::uvec4 getViewport() const { return myViewportParams; } /// x, y, width, height
    void setBlendState(const BlendState& clBlendState);
    void setDepthStencilState(const DepthStencilState& clDepthStencilState);
    void setFillMode(const FillMode eFillMode);
    void setCullMode(const CullMode eCullMode);
    void setWindingOrder(const WindingOrder eWindingOrder);

    void setDepthStencilRenderTarget(Texture* pDStexture);
    void setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex);
    void removeAllRenderTargets();

    void renderGeometry(const Geometry::GeometryData* pGeometry);

    uint64 ExecuteAndReset(bool aWaitForCompletion = false);

    void Destroy();
    void Reset();

    // DX12-Specific stuff - TODO: Check if we need to find platform-independent ways to express these
    void UpdateSubresources(ID3D12Resource* aDestResource, ID3D12Resource* aStagingResource, 
      uint32 aFirstSubresourceIndex, uint32 aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas);
    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DescriptorHeapDX12* aDescriptorHeap);
    void TransitionResource(GpuResourceDX12* aResource, D3D12_RESOURCE_STATES aDestState, bool aExecuteNow = false);
    //void CopySubresources(ID3D12Resource* aDestResource, ID3D12Resource* aSrcResource, uint aFirstSubresource, uint aSubResourceCount);

    void CopyResource(GpuResourceDX12* aDestResource, GpuResourceDX12* aSrcResource);

    static void InitBufferData(GpuBufferDX12* aBuffer, void* aDataPtr);
    static void UpdateBufferData(GpuBufferDX12* aBuffer, void* aDataPtr, uint32 aByteOffset, uint32 aByteSize);
    static void InitTextureData(TextureDX12* aTexture, const TextureUploadData* someUploadDatas, uint32 aNumUploadDatas);

  protected:
    void ApplyViewport();
    void ApplyPipelineState();
    void ApplyDescriptorHeaps();
    void ApplyRenderTargets();
    void KickoffResourceBarriers();
    void ReleaseAllocator(uint64 aFenceVal);
    void ReleaseDynamicHeaps(uint64 aFenceVal);
    void ResetInternalStates();

    static std::unordered_map<uint, ID3D12PipelineState*> ourPSOcache;
    
    Renderer& myRenderer;
    CommandAllocatorPoolDX12& myCommandAllocatorPool;

    PipelineState myPipelineState;

    glm::uvec4 myViewportParams;
    bool myViewportDirty;

    Texture* myRenderTargets[Rendering::Constants::kMaxNumRenderTargets];
    Texture* myDepthStencilTarget;
    bool myRenderTargetsDirty;

    ID3D12RootSignature* myRootSignature;  // The rootSignature that is set on myCommandList
    ID3D12GraphicsCommandList* myCommandList;

    ID3D12CommandAllocator* myCommandAllocator;
    FixedArray<D3D12_RESOURCE_BARRIER, kMaxNumCachedResourceBarriers> myWaitingResourceBarriers;
    
    DescriptorHeapDX12* myDynamicShaderVisibleHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    std::vector<DescriptorHeapDX12*> myRetiredDescriptorHeaps; // TODO: replace vector with a smallObjectPool

    bool myIsInRecordState;

    GpuDynamicAllocatorDX12 myCpuVisibleAllocator;
    GpuDynamicAllocatorDX12 myGpuOnlyAllocator;
  };
//---------------------------------------------------------------------------//
} } }
