#ifndef INCLUDE_RENDERERGL4_H
#define INCLUDE_RENDERERGL4_H


#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "LoadableObject.h"

namespace Fancy { namespace Core { namespace Rendering { namespace GL4 {

//---------------------------------------------------------------------------//
class RendererGL4 : public LoadableObject
{
public:
	virtual ~RendererGL4();

	virtual bool _init() override;
	virtual bool _destroy() override;

  /// Sets the render-system to a valid state. Should be called just before the first frame
  void postInit();

  /// Sets the blendState that should be active upon in next draw call
	void setBlendState(const BlendState& clBlendState);
  /// Retrieves the cached blendState configured for the next draw call
  const BlendState& getBlendState() const {return m_clBlendState;}

  /// Sets the depthstencil-state that should be active in the next draw call
  void setDepthStencilState(const DepthStencilState& clDepthStencilState);
  /// Retrieves the depthstencil-state cached for the next draw call
  const DepthStencilState& getDepthStencilState() const {return m_clDepthStencilState;}

	void setFillMode(const FillMode eFillMode);
	FillMode getFillMode() const { return m_eFillMode; }

	void setCullMode(const CullMode eCullMode);
	CullMode getCullMode() const { return m_eCullMode; }

	void setWindingOrder(const WindingOrder eWindingOrder);
	WindingOrder getWindingOrder() const { return m_eWindingOrder; }

  void setDepthStencilRenderTarget(Texture* pDStexture);
	void setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex);
  void removeAllRenderTargets();
  
  Texture* getCachedDepthStencilRenderTarget() { return m_pCachedDepthStencilTarget; }
	Texture* getCachedRenderTarget(const uint8 u8RenderTargetIndex) const 
	{ ASSERT(u8RenderTargetIndex < kMaxNumRenderTargets); return m_pCachedRenderTargets[u8RenderTargetIndex]; }

	void setReadTexture(Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
	void setReadBuffer(Buffer* pBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
	void setConstantBuffer(ConstantBuffer* pConstantBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
	void setTextureSampler(TextureSampler* pSampler, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
	void setGPUProgram(GPUProgram* pProgram, const ShaderStage eShaderStage);

protected:
//-----------------------------------------------------------------------//
  enum Constants {
    kPoolSizeFBO = 20
  };
//-----------------------------------------------------------------------//
  struct FBOcacheEntry {
    FBOcacheEntry() : glHandle(GLUINT_HANDLE_INVALID), hash(0) {}
    GLuint glHandle;
    uint hash;
  };
//-----------------------------------------------------------------------//

	RendererGL4();

  /// Applies all dirty states and resources to the hardware
  void bindStatesToPipeline();
  void bindResourcesToPipeline(const ShaderStage eShaderStage);
  void bindBlendState();
  void _bindBlendValuesMultiRT(const uint32 uBlendStateRebindMask, 
    const uint8 u8BlendStateRebindRTcount, const uint8 u8BlendStateRebindRTmask);
  void _bindBlendValuesSingleRT(const uint32 uBlendStateRebindMask);
  void bindDepthStencilState();
  void bindRenderTargets();
  GLuint createOrRetrieveFBO(Texture** ppRenderTextures, uint8 u8RenderTextureCount, Texture* pDStexture);

  //void applyReadTextures(
	
  /// Mask indicating which pipeline states have to be re-bound to the pipeline
	uint          m_uPipelineRebindMask;  // Needed?
	/// Mask indicating which resources have to be re-bound to each shaderStage
	uint32        m_uResourceRebindMask[ShaderStage::NUM];  // Needed?
	
	/// Cached textures per shaderStage bound/to bind to the pipeline
	Texture*	    m_pCachedReadTextures [ShaderStage::NUM][kMaxNumReadTextures];
	/// Mask identifying which textures need to be bind in the next draw call
	uint32		    m_uReadTextureBindMask [ShaderStage::NUM];

	GpuBuffer*		m_pCachedReadBuffers [ShaderStage::NUM][kMaxNumBoundReadBuffers];
	uint32		    m_uReadBufferBindMask [ShaderStage::NUM];

	ConstantBuffer*		m_pCachedConstantBuffers [ShaderStage::NUM][kMaxNumBoundConstantBuffers];
	uint32				    m_uConstantBufferBindMask[ShaderStage::NUM];

	TextureSampler*		m_pCachedTextureSamplers [ShaderStage::NUM][kMaxNumBoundSamplers];
	uint32				    m_uTextureSamplerBindMask[ShaderStage::NUM];

	Texture*			    m_pCachedRenderTargets [kMaxNumRenderTargets];
  Texture*          m_pCachedDepthStencilTarget;

  /// The currently bound FBO
  GLuint            m_uCachedFBO;
  /// Pool of available FBO formats
  FBOcacheEntry     m_FBOpool[kPoolSizeFBO];

	GPUProgram*			    m_pBoundGPUPrograms [ShaderStage::NUM];
	uint32				      m_uGPUprogramBindMask;

	DepthStencilState   m_clDepthStencilState;
  uint32              m_uDepthStencilRebindMask;

	BlendState			    m_clBlendState;
  uint32              m_uBlendStateRebindMask;
  uint8               m_u8BlendStateRebindRTmask;
  uint8               m_u8BlendStateRebindRTcount;

	FillMode			      m_eFillMode;
	CullMode			      m_eCullMode;
	WindingOrder		    m_eWindingOrder;
};

} // end of namespace GL4
} // end of namespace Rendering
} // end of namespace Core
} // end of namespace Fancy


#endif  // INCLUDE_RENDERERGL4_H