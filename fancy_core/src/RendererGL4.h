#ifndef INCLUDE_RENDERERGL4_H
#define INCLUDE_RENDERERGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "LoadableObject.h"
#include "GeometryVertexLayout.h"
#include "VertexInputLayout.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "ShaderConstantsManager.h"

namespace Fancy { namespace Rendering { namespace GL4 {

//---------------------------------------------------------------------------//
class RendererGL4
{
public:
  virtual ~RendererGL4();

  /// Sets the render-system to a valid state. Should be called just before the first frame
  void postInit();

  void beginFrame();
  void endFrame();

  /// x, y, width, height
  void setViewport(const glm::uvec4& uViewportParams);
  /// x, y, width, height
  const glm::uvec4 getViewport() const {return m_uViewportParams;}

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

  void setReadTexture(const Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
  void setWriteTexture(Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
  void setReadBuffer(GpuBuffer* pBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
  void setConstantBuffer(const GpuBuffer* pConstantBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
  void setTextureSampler(TextureSampler* pSampler, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
  void setGpuProgram(const GpuProgram* pProgram, const ShaderStage eShaderStage);

  void renderGeometry(const Geometry::GeometryData* pGeometry);
protected:
//-----------------------------------------------------------------------//
  enum Constants {
    kPoolSizeFBO = 20u,
    kPoolSizeGpuProgramPipelines = 50u,
    kPoolSizeVAO = 20u
  };
//-----------------------------------------------------------------------//
  struct GpuCacheEntry {
    GpuCacheEntry() : glHandle(GLUINT_HANDLE_INVALID), hash(0u) {}
    GLuint glHandle;
    uint hash;
  };
//---------------------------------------------------------------------------//
  struct VaoCacheEntry {
    VaoCacheEntry() : glHandle(GLUINT_HANDLE_INVALID), hash(0u) {}
    GLuint glHandle;
    uint hash;
    uint32 uStreamMask;
  };
//-----------------------------------------------------------------------//
  RendererGL4();

  /// Applies all dirty states and resources to the hardware
  void bindStatesToPipeline();
  /// Applies all dirty resources to the current program pipeline
  void bindResourcesToPipeline();
  /// Sets all gpuProgram resources to a 'dirty' state so that they are re-bound during the next draw
  void invalidateResourceCache(const uint32 uShaderStageIdx);

  void applyViewport();
  void bindBlendState();
  void _bindBlendValuesMultiRT(const uint32 uBlendStateRebindMask, 
    const uint8 u8BlendStateRebindRTcount, const uint8 u8BlendStateRebindRTmask);
  void _bindBlendValuesSingleRT(const uint32 uBlendStateRebindMask);
  void bindDepthStencilState();
  void bindRenderTargets();
  void bindIBO(GLuint uIBO) {if (m_uCurrentIBO == uIBO) return; m_uCurrentIBO = uIBO; glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uIBO);}
  void bindVAO(GLuint uVAO) {if (m_uCurrentVAO == uVAO) return; m_uCurrentVAO = uVAO; glBindVertexArray(uVAO);}

  GLuint createOrRetrieveFBO(Texture** ppRenderTextures, uint8 u8RenderTextureCount, Texture* pDStexture);
  GLuint createOrRetrieveProgramPipeline();
  const VaoCacheEntry& createOrRetrieveVAO(const GeometryVertexLayout* pGeoVertexLayout, const VertexInputLayout* pVertexInputLayout);
  
  glm::uvec4 m_uViewportParams;
  bool m_bViewportDirty;

  /// Mask indicating which pipeline states have to be re-bound to the pipeline
  uint          m_uPipelineRebindMask;  // Needed?
  /// Mask indicating which resources have to be re-bound to each shaderStage
  uint32        m_uResourceRebindMask[ShaderStage::NUM];  // Needed?
  
  /// Cached textures per shaderStage bound/to bind to the pipeline
  const Texture*	    m_pCachedReadTextures [ShaderStage::NUM][kMaxNumReadTextures];
  /// Mask identifying which textures need to be bind in the next draw call
  uint32		    m_uReadTextureBindMask [ShaderStage::NUM];
  uint32        m_uNumReadTexturesToBind[ShaderStage::NUM];

  Texture*      m_pCachedWriteTextures [ShaderStage::NUM][kMaxNumWriteTextures];
  uint32        m_uWriteTextureBindMask [ShaderStage::NUM];
  uint32        m_uNumWriteTexturesToBind[ShaderStage::NUM];

  GpuBuffer*		m_pCachedReadBuffers [ShaderStage::NUM][kMaxNumReadBuffers];
  uint32		    m_uReadBufferBindMask [ShaderStage::NUM];
  uint32        m_uNumReadBuffersToBind[ShaderStage::NUM];

  const GpuBuffer*		m_pCachedConstantBuffers [ShaderStage::NUM][(uint) ConstantBufferType::NUM];
  uint32				      m_uConstantBufferBindMask[ShaderStage::NUM];
  uint32              m_uNumConstantBuffersToBind[ShaderStage::NUM];

  TextureSampler*		m_pCachedTextureSamplers [ShaderStage::NUM][kMaxNumBoundSamplers];
  uint32				    m_uTextureSamplerBindMask[ShaderStage::NUM];
  uint32            m_uNumTextureSamplersToBind[ShaderStage::NUM];

  Texture*			    m_pCachedRenderTargets [kMaxNumRenderTargets];
  Texture*          m_pCachedDepthStencilTarget;

  /// The currently bound FBO
  GLuint            m_uCurrentFBO;
  /// Pool of available FBO formats
  GpuCacheEntry     m_FBOpool[kPoolSizeFBO];

  /// The currently bound GPU program pipeline
  GLuint            m_uCurrentGpuProgramPipeline;
  /// Pool of available GpuProgramPipeline Objects
  GpuCacheEntry     m_GpuProgramPipelinePool[kPoolSizeGpuProgramPipelines];

  /// The currently bound VAO
  GLuint            m_uCurrentVAO;
  /// Pool of available VAOs
  VaoCacheEntry     m_VAOpool[kPoolSizeVAO];

  GLuint            m_uCurrentVBO;
  GLuint            m_uCurrentIBO;

  const GpuProgram*		m_pBoundGPUPrograms [ShaderStage::NUM];

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
} // end of namespace Fancy


#endif  // INCLUDE_RENDERERGL4_H