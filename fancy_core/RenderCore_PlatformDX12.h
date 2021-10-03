#pragma once

#include "RenderCore_Platform.h"
#include "DX12Prerequisites.h"

#include "DataFormat.h"
#include "Ptr.h"
#include "ShaderVisibleDescriptorHeapDX12.h"
#include "StaticDescriptorAllocatorDX12.h"
#include "GpuMemoryAllocatorDX12.h"
#include "CommandAllocatorPoolDX12.h"
#include "CommandQueueDX12.h"
#include "PipelineStateCacheDX12.h"
#include "RootSignatureDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class GpuResourceViewType;
  struct WindowParameters;
  class Shader;
  class Texture;
  class TextureView;
  struct TextureViewProperties;
  class GpuBuffer;
  class GpuBufferView;
  struct GpuBufferViewProperties;
//---------------------------------------------------------------------------//  
  class RenderCore_PlatformDX12 final : public RenderCore_Platform
  {
  public:
    static D3D12_LOGIC_OP ResolveLogicOp(LogicOp aLogicOp);
    static D3D12_COMPARISON_FUNC ResolveCompFunc(const CompFunc& aCompFunc);
    static D3D12_STENCIL_OP ResolveStencilOp(const StencilOp& aStencilOp);
    static DXGI_FORMAT ResolveFormat(DataFormat aFormat);
    static DXGI_FORMAT GetDepthStencilTextureFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetDepthStencilViewFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetDepthViewFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetStencilViewFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetTypelessFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetCopyableFormat(DXGI_FORMAT aFormat, uint aPlaneIndex);
    static D3D12_COMMAND_LIST_TYPE GetCommandListType(CommandListType aType);
    static D3D12_HEAP_TYPE ResolveHeapType(CpuMemoryAccessType anAccessType);
    static D3D12_DESCRIPTOR_HEAP_TYPE GetDescriptorHeapType(const GpuResourceViewType& aViewType);
    static D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(const GpuResourceViewType& aViewType);
    static D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE GetRaytracingBVHType(RaytracingBVHType aType);
    static D3D12_RAYTRACING_GEOMETRY_TYPE GetRaytracingBVHGeometryType(RaytracingBVHGeometryType aGeoType);
    static D3D12_HIT_GROUP_TYPE GetRaytracingHitGroupType(RaytracingHitGroupType aType);
    static D3D12_RAYTRACING_PIPELINE_FLAGS GetRaytracingPipelineFlags(RaytracingPipelineFlags someFlags);

    RenderCore_PlatformDX12(const RenderPlatformProperties& someProperties);
    ~RenderCore_PlatformDX12() override;
    // Disallow copy and assignment (class contains a list of unique_ptrs)
    RenderCore_PlatformDX12(const RenderCore_PlatformDX12&) = delete;
    RenderCore_PlatformDX12& operator=(const RenderCore_PlatformDX12&) = delete;

    void BeginFrame() override;

    bool IsInitialized() override { return ourDevice.Get() != nullptr; }
    bool InitInternalResources() override;
    void Shutdown() override;

    void InitNullDescriptors();

    ID3D12Device8* GetDevice() const { return ourDevice.Get(); }

    ID3D12CommandAllocator* GetCommandAllocator(CommandListType aCmdListType);
    void ReleaseCommandAllocator(ID3D12CommandAllocator* anAllocator, uint64 aFenceVal);

    DescriptorDX12 AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, const char* aDebugName = nullptr);
    DescriptorDX12 AllocateShaderVisibleDescriptorForGlobalResource(GlobalResourceType aGlobalResourceType, const char* aDebugName = nullptr);
    void FreeDescriptor(const DescriptorDX12& aDescriptor);

    GpuMemoryAllocationDX12 AllocateGpuMemory(GpuMemoryType aType, CpuMemoryAccessType anAccessType, uint64 aSize, uint anAlignment, const char* aDebugName = nullptr);
    void ReleaseGpuMemory(GpuMemoryAllocationDX12& anAllocation);

    RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams) override;
    ShaderCompiler* CreateShaderCompiler() override;
    Shader* CreateShader() override;
    ShaderPipeline* CreateShaderPipeline() override;
    Texture* CreateTexture() override;
    GpuBuffer* CreateBuffer() override;
    TextureSampler* CreateTextureSampler(const TextureSamplerProperties& someProperties) override;
    CommandList* CreateCommandList(CommandListType aType) override;
    CommandQueue* CreateCommandQueue(CommandListType aType) override;
    TextureView* CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName = nullptr) override;
    GpuBufferView* CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName = nullptr) override;
    RaytracingAS* CreateRtAccelerationStructure(const RaytracingAsProps& someProps, const eastl::span<RaytracingAsGeometryInfo>& someGeometries, const char* aName = nullptr) override;
    RaytracingPipelineState* CreateRtPipelineState(const RaytracingPipelineStateProperties& someProps) override;

    GpuQueryHeap* CreateQueryHeap(GpuQueryType aType, uint aNumQueries) override;
    uint GetQueryTypeDataSize(GpuQueryType aType) override;
    float64 GetGpuTicksToMsFactor(CommandListType aCommandListType) override;
    Microsoft::WRL::ComPtr<IDXGISwapChain> CreateSwapChain(const DXGI_SWAP_CHAIN_DESC& aSwapChainDesc);

    const DescriptorDX12& GetNullDescriptor(D3D12_DESCRIPTOR_RANGE_TYPE aType, GpuResourceDimension aResouceDimension) const;
    PipelineStateCacheDX12& GetPipelineStateCache() { return myPipelineStateCache; }
    ShaderVisibleDescriptorHeapDX12* GetShaderVisibleDescriptorHeap() { return myShaderVisibleDescriptorHeap.get(); }
    const RootSignatureDX12* GetRootSignature() const { return myRootSignature.get(); }

    CommandQueueDX12* GetCommandQueueDX12(CommandListType aCommandListType);

    Microsoft::WRL::ComPtr<ID3D12Device8> ourDevice;

    UniquePtr<StaticDescriptorAllocatorDX12> myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    UniquePtr<ShaderVisibleDescriptorHeapDX12> myShaderVisibleDescriptorHeap;
    UniquePtr<GpuMemoryAllocatorDX12> myGpuMemoryAllocators[(uint)GpuMemoryType::NUM][(uint)CpuMemoryAccessType::NUM];
    UniquePtr<CommandAllocatorPoolDX12> ourCommandAllocatorPools[(uint)CommandListType::NUM];
    UniquePtr<RootSignatureDX12> myRootSignature;
    float64 myGpuTicksToMsFactor[(uint)CommandListType::NUM];

    DescriptorDX12 mySRVNullDescriptors[(uint)GpuResourceDimension::NUM];
    DescriptorDX12 myUAVNullDescriptors[(uint)GpuResourceDimension::NUM];
    DescriptorDX12 mySamplerNullDescriptor;
    DescriptorDX12 myCBVNullDescriptor;

    PipelineStateCacheDX12 myPipelineStateCache;
  };
//---------------------------------------------------------------------------//
}

#endif