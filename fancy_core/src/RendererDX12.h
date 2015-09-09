#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined (RENDERER_DX12)

#include "DepthStencilState.h"
#include "BlendState.h"
#include "FenceDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {

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

		/// Sets the blendState that should be active upon in next draw call
		void setBlendState(const BlendState& clBlendState);
		/// Retrieves the cached blendState configured for the next draw call
		const BlendState& getBlendState() const { return myBlendState; }

		/// Sets the depthstencil-state that should be active in the next draw call
		void setDepthStencilState(const DepthStencilState& clDepthStencilState);
		/// Retrieves the depthstencil-state cached for the next draw call
		const DepthStencilState& getDepthStencilState() const { return myDepthStencilState; }

		void setFillMode(const FillMode eFillMode);
		FillMode getFillMode() const { return myFillMode; }

		void setCullMode(const CullMode eCullMode);
		CullMode getCullMode() const { return myCullMode; }

		void setWindingOrder(const WindingOrder eWindingOrder);
		WindingOrder getWindingOrder() const { return myWindingOrder; }

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

	protected:
		RendererDX12();

		FillMode			myFillMode;
		CullMode			myCullMode;
		WindingOrder		myWindingOrder;
		DepthStencilState   myDepthStencilState;
		glm::uvec4			myViewportParams;
		BlendState			myBlendState;

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