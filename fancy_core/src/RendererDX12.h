#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined (RENDERER_DX12)

#include "DepthStencilState.h"
#include "BlendState.h"
#include "FenceDX12.h"
#include <unordered_map>

namespace Fancy { namespace Rendering {
  class GeometryVertexLayout;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  struct InputLayout
  {
    std::vector<D3D12_INPUT_ELEMENT_DESC> myElements;

    // Combined hash of ShaderVertexLayout, GeometryVertexLayout as well as any instancing data
    // should be used to quickly determine if an inputlayout can be re-used for changing shader- geometry- or instance data 
    // TODO: Important: Implement InputLayout in DX12
    uint myHash;
  };
//---------------------------------------------------------------------------//
  struct PipelineState
  {
    PipelineState();
    uint getHash();
    D3D12_GRAPHICS_PIPELINE_STATE_DESC toNativePSOdesc();
    
    FillMode myFillMode;
    CullMode myCullMode;
    WindingOrder myWindingOrder;
    DepthStencilState myDepthStencilState;
    BlendState myBlendState;
    const GpuProgram* myShaderStages[static_cast<uint>(ShaderStage::NUM)];
    uint8 myNumRenderTargets;
    DataFormat myRTVformats[Constants::kMaxNumRenderTargets];
    DataFormat myDSVformat;
    InputLayout myInputLayout;

    bool myIsDirty : 1;
  };
//---------------------------------------------------------------------------//
  struct ResourceState
  {
    Texture* myDSV; 
    



    bool myIsDirty : 1;
  };
//---------------------------------------------------------------------------//
	class RendererDX12
	{
	public:
		virtual ~RendererDX12();

    void init(void* aNativeWindowHandle);
		/// Sets the render-system to a valid state. Should be called just before the first frame
		void postInit();

		void beginFrame();
		void endFrame();

    // TODO: Implement multithreaded rendering...
    uint32 getCurrentRenderThreadIdx() { return 0u; }

		/// x, y, width, height
		void setViewport(const glm::uvec4& uViewportParams);
		/// x, y, width, height
		const glm::uvec4 getViewport() const { return myViewportParams; }

		void setBlendState(const BlendState& clBlendState);
		void setDepthStencilState(const DepthStencilState& clDepthStencilState);
		void setFillMode(const FillMode eFillMode);
		void setCullMode(const CullMode eCullMode);
		void setWindingOrder(const WindingOrder eWindingOrder);

		void setDepthStencilRenderTarget(Texture* pDStexture);
		void setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex);
		void removeAllRenderTargets();

    void SetGraphicsRootSignature(ID3D12RootSignature* aRootSignature);

		void setReadTexture(const Texture* pTexture, const uint8 u8RegisterIndex);
		void setWriteTexture(const Texture* pTexture, const uint8 u8RegisterIndex);
		void setReadBuffer(const GpuBuffer* pBuffer, const uint8 u8RegisterIndex);
		void setConstantBuffer(const GpuBuffer* pConstantBuffer, const uint8 u8RegisterIndex);
		void setTextureSampler(const TextureSampler* pSampler, const uint8 u8RegisterIndex);
		void setGpuProgram(const GpuProgram* pProgram, const ShaderStage eShaderStage);

		void renderGeometry(const Geometry::GeometryData* pGeometry);
    /// Returns the pipeline state in the current thread
    PipelineState& getState();
    /// Returns the graphics command list of the current thread
    ComPtr<ID3D12GraphicsCommandList>& getGraphicsCmdList();

    ComPtr<ID3D12Device>& GetDevice() { return myDevice; }

	protected:
    void applyViewport();
	  void applyPipelineState();

	  RendererDX12();

    PipelineState myState[Constants::kNumRenderThreads];

    uint myPSOhash;
    ID3D12PipelineState* myPSO;
    std::unordered_map<uint, ID3D12PipelineState*> myPSOcache;

    glm::uvec4 myViewportParams;
    bool myViewportDirty;

    // Synchronization objects.
    uint myFrameIndex;
    FenceDX12 myFrameDone;

    static const uint kBackbufferCount = 2u;
    ComPtr<ID3D12Device> myDevice;
    ComPtr<IDXGISwapChain3> mySwapChain;
    ComPtr<ID3D12Resource> myBackbuffers[kBackbufferCount];
    ComPtr<ID3D12CommandAllocator> myCommandAllocator;
    ComPtr<ID3D12CommandQueue> myCommandQueue;
    ComPtr<ID3D12PipelineState> myPipelineState;

    ComPtr<ID3D12GraphicsCommandList> myCommandList[Constants::kNumRenderThreads];

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myRtvHeap;
    uint myRtvDescriptorSize;
	};
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  class RenderingSubsystemDX12
  {
  public:
    /// Initializes platform-dependent rendering stuff
    static void InitPlatform();
    /// Shutdown of platform-dependent rendering stuff
    static void ShutdownPlatform();
  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Renderer::DX12

#endif