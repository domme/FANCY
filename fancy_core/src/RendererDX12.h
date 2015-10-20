#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined (RENDERER_DX12)

#include "DepthStencilState.h"
#include "BlendState.h"
#include "FenceDX12.h"
#include <unordered_map>

namespace Fancy { namespace Rendering { namespace DX12 {

  struct PipelineState
  {
    PipelineState();

    FillMode myFillMode;
    CullMode myCullMode;
    WindingOrder myWindingOrder;
    DepthStencilState myDepthStencilState;
    BlendState myBlendState;
    GpuProgram* myShaderStages[static_cast<uint>(ShaderStage::NUM)];


    bool myIsDirty : 1;
  };

  struct ResourceState
  {
    Texture* myDSV;
  
    bool myIsDirty : 1;
  };

	class RendererDX12
	{
	public:
		virtual ~RendererDX12();

    void init(void* aNativeWindowHandle);
		/// Sets the render-system to a valid state. Should be called just before the first frame
		void postInit();

		void beginFrame();
		void endFrame();

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

		void setReadTexture(const Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
		void setWriteTexture(const Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
		void setReadBuffer(const GpuBuffer* pBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
		void setConstantBuffer(const GpuBuffer* pConstantBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
		void setTextureSampler(const TextureSampler* pSampler, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
		void setGpuProgram(const GpuProgram* pProgram, const ShaderStage eShaderStage);

		void renderGeometry(const Geometry::GeometryData* pGeometry);
    PipelineState& getState();

	protected:
    void applyViewport();
	  void applyPipelineState();
    void computeRequestedPipelineHash(uint& someHashOut) const;

	  RendererDX12();

    PipelineState myState;  // There will be one pipeline state per thread later on...

    uint myPSOhash;
    ID3D12PipelineState* myPSO;
    std::unordered_map<uint, ID3D12PipelineState*> myPSOcache;

    glm::uvec4 myViewportParams;
    bool myViewportDirty;

    // Synchronization objects.
    uint myFrameIndex;
    FenceDX12 myFrameDone;

    static const uint kBackbufferCount = 2u;
    Microsoft::WRL::ComPtr<ID3D12Device> myDevice;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> mySwapChain;
    Microsoft::WRL::ComPtr<ID3D12Resource> myBackbuffers[kBackbufferCount];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> myCommandAllocator;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> myCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> myPipelineState;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> myCommandList;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myRtvHeap;
    uint myRtvDescriptorSize;
	};

} } } 

#endif