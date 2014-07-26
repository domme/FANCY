#ifndef INCLUDE_RENDERERGL4_H
#define INCLUDE_RENDERERGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "LoadableObject.h"

namespace FANCY { namespace Core { namespace Rendering { namespace GL4 {

// Adapter-methods to translate between the state-enums
//-----------------------------------------------------------------------//
  template<typename T>
  GLenum toGLType(const T& generalType) {ASSERT_M(false, "Dummy-template called"); return 0;}
  template<typename T>
  T toGeneralType(const GLenum& glType) {ASSERT_M(false, "Dummy-template called");}
//-----------------------------------------------------------------------//
  template<typename T>
  GLuint toGLFlag(const T& generalType) {ASSERT_M(false, "Dummy-template called"); return 0;}
  template<typename T>
  T toGeneralFlag(const GLuint& glType) {ASSERT_M(false, "Dummy-template called");}
//-----------------------------------------------------------------------//
  // Comp Func
  template<>
  GLenum toGLType(const CompFunc& generalType) {
    switch (generalType) {
      case CompFunc::NEVER:     return GL_NEVER;
      case CompFunc::LESS:      return GL_LESS;
      case CompFunc::EQUAL:     return GL_EQUAL;
      case CompFunc::LEQUAL:    return GL_LEQUAL;
      case CompFunc::GREATER:   return GL_GREATER;
      case CompFunc::NOTEQUAL:  return GL_NOTEQUAL;
      case CompFunc::GEQUAL:    return GL_GEQUAL;
      case CompFunc::ALWAYS:    return GL_ALWAYS;
      default: ASSERT_M(false, "Missing GL values"); return GL_LESS;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLenum toGLType(const StencilOp& generalType) {
    switch (generalType) {
      case StencilOp::KEEP:             return GL_KEEP;
      case StencilOp::ZERO:             return GL_ZERO;
      case StencilOp::REPLACE:          return GL_REPLACE;
      case StencilOp::INCREMENT_CLAMP:  return GL_INCR; 
      case StencilOp::DECREMENT_CLAMP:  return GL_DECR;
      case StencilOp::INVERT:           return GL_INVERT;
      case StencilOp::INCEMENT_WRAP:    return GL_INCR_WRAP;
      case StencilOp::DECREMENT_WRAP:   return GL_DECR_WRAP;
      default: ASSERT_M(false, "Missing GL values"); return GL_KEEP;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLenum toGLType(const FillMode& generalType) {
    switch (generalType) {
      case FillMode::WIREFRAME: return GL_LINE;
      case FillMode::SOLID:     return GL_FILL;
      default: ASSERT_M(false, "Missing GL values"); return GL_FILL;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLenum toGLType(const CullMode& generalType) {
    switch (generalType) {
      case CullMode::NONE:  return GL_NONE; // Special type. Requires call to glDisable(CULLING)
      case CullMode::FRONT: return GL_FRONT;
      case CullMode::BACK:  return GL_BACK;
      default: ASSERT_M(false, "Missing GL values"); return GL_BACK;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLenum toGLType(const WindingOrder& generalType) {
    switch (generalType) {
      case WindingOrder::CCW: return GL_CCW;
      case WindingOrder::CW:  return GL_CW;
      default: ASSERT_M(false, "Missing GL values"); return GL_CCW;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLenum toGLType(const BlendInput& generalType) {
    switch (generalType) {
      case BlendInput::ZERO:                return GL_ZERO;
      case BlendInput::ONE:                 return GL_ONE;
      case BlendInput::SRC_COLOR:           return GL_SRC_COLOR;
      case BlendInput::INV_SRC_COLOR:       return GL_ONE_MINUS_SRC_COLOR;
      case BlendInput::SRC_ALPHA:           return GL_SRC_ALPHA;
      case BlendInput::INV_SRC_ALPHA:       return GL_ONE_MINUS_SRC_ALPHA;
      case BlendInput::DEST_ALPHA:          return GL_DST_ALPHA;
      case BlendInput::INV_DEST_ALPHA:      return GL_ONE_MINUS_DST_ALPHA;
      case BlendInput::DEST_COLOR:          return GL_DST_COLOR;
      case BlendInput::INV_DEST_COLOR:      return GL_ONE_MINUS_DST_COLOR;
      case BlendInput::SRC_ALPHA_CLAMPED:   return GL_SRC_ALPHA;
      case BlendInput::BLEND_FACTOR:        ASSERT_M(false, "BLEND_FACTOR not supported by OpenGL"); return GL_ONE;
      case BlendInput::INV_BLEND_FACTOR:    ASSERT_M(false, "INV_BLEND_FACTOR not supported by OpenGL"); return GL_ZERO;
      case BlendInput::SRC1_COLOR:          return GL_SRC1_COLOR;
      case BlendInput::INV_SRC1_COLOR:      return GL_ONE_MINUS_SRC1_COLOR;
      case BlendInput::SRC1_ALPHA:          return GL_SRC1_ALPHA;
      case BlendInput::INV_SRC1_ALPHA:      return GL_ONE_MINUS_SRC1_ALPHA;
      case BlendInput::CONSTANT_COLOR:      return GL_CONSTANT_COLOR;
      case BlendInput::INV_CONSTANT_COLOR:  return GL_ONE_MINUS_CONSTANT_COLOR;
      case BlendInput::CONSTANT_ALPHA:      return GL_CONSTANT_ALPHA;
      case BlendInput::INV_CONSTANT_ALPHA:  return GL_ONE_MINUS_CONSTANT_ALPHA;
      default: ASSERT_M(false, "Missing GL values"); return GL_ONE;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLenum toGLType(const BlendOp& generalType) {
    switch (generalType) {
      case BlendOp::ADD:            return GL_FUNC_ADD;
      case BlendOp::SUBTRACT:       return GL_FUNC_SUBTRACT;
      case BlendOp::REV_SUBTRACT:   return GL_FUNC_REVERSE_SUBTRACT;
      case BlendOp::MIN:            return GL_MIN;
      case BlendOp::MAX:            return GL_MAX;
      default: ASSERT_M(false, "Missing GL values"); return GL_FUNC_ADD;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLenum toGLType(const ShaderStage& generalType) {
    switch (generalType) {
      case ShaderStage::VERTEX:       return GL_VERTEX_SHADER;
      case ShaderStage::FRAGMENT:     return GL_FRAGMENT_SHADER;
      case ShaderStage::GEOMETRY:     return GL_GEOMETRY_SHADER;
      case ShaderStage::TESS_DOMAIN:  return GL_TESS_EVALUATION_SHADER;
      case ShaderStage::TESS_HULL:    return GL_TESS_CONTROL_SHADER;
      case ShaderStage::COMPUTE:      return GL_COMPUTE_SHADER;
      default: ASSERT_M(false, "Missing GL values"); return GL_VERTEX_SHADER;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLuint toGLFlag(const ShaderStageFlag& generalType) {
    switch (generalType) {
      case ShaderStageFlag::VERTEX:       return GL_VERTEX_SHADER_BIT;
      case ShaderStageFlag::FRAGMENT:     return GL_FRAGMENT_SHADER_BIT;
      case ShaderStageFlag::GEOMETRY:     return GL_GEOMETRY_SHADER_BIT;
      case ShaderStageFlag::TESS_DOMAIN:  return GL_TESS_EVALUATION_SHADER_BIT;
      case ShaderStageFlag::TESS_HULL:    return GL_TESS_CONTROL_SHADER_BIT;
      case ShaderStageFlag::COMPUTE:      return GL_COMPUTE_SHADER_BIT;
      case ShaderStageFlag::ALL:          return GL_ALL_SHADER_BITS;
      default: ASSERT_M(false, "Missing GL values"); return GL_ALL_SHADER_BITS;
    }
  }
//-----------------------------------------------------------------------//
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

  void setColorWriteMask(const uint32 uWriteMask);
  void setColorWriteMask(const bool bRed, const bool bGreen, const bool bBlue, const bool bAlpha);
  uint32 getColorWriteMask() const { return m_uColorWriteMask; }

	void setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex);
  void removeAllRenderTargets();
	Texture* getBoundRenderTarget(const uint8 u8RenderTargetIndex) const 
	{ ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_RENDERTARGETS); return m_pCachedRenderTargets[u8RenderTargetIndex]; }

	void setReadTexture(Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
	void setReadBuffer(Buffer* pBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
	void setConstantBuffer(ConstantBuffer* pConstantBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
	void setTextureSampler(TextureSampler* pSampler, const ShaderStage eShaderStage, const uint8 u8RegisterIndex);
	void setGPUProgram(GPUProgram* pProgram, const ShaderStage eShaderStage);

protected:
	RendererGL4();

  enum Constants {
    kPoolSizeFBO = 20
  };
  
  /// Applies all dirty states and resources to the hardware
  void bindStatesToPipeline();
  void bindResourcesToPipeline(const ShaderStage eShaderStage);
  void bindBlendState();
  void _bindBlendValuesMultiRT(const uint32 uBlendStateRebindMask, 
    const uint8 u8BlendStateRebindRTcount, const uint8 u8BlendStateRebindRTmask);
  void _bindBlendValuesSingleRT(const uint32 uBlendStateRebindMask);
  void bindDepthStencilState();
  void bindRenderTargets();
  GLuint createOrRetrieveFBO(Texture** ppRenderTextures, uint8 u8RenderTextureCount);

  //void applyReadTextures(
	
  /// Mask indicating which pipeline states have to be re-bound to the pipeline
	uint          m_uPipelineRebindMask;  // Needed?
	/// Mask indicating which resources have to be re-bound to each shaderStage
	uint32        m_uResourceRebindMask[ShaderStage::NUM];  // Needed?
	
	/// Cached textures per shaderStage bound/to bind to the pipeline
	Texture*	    m_pCachedReadTextures [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_READ_TEXTURES];
	/// Mask identifying which textures need to be bind in the next draw call
	uint32		    m_uReadTextureBindMask [ShaderStage::NUM];

	Buffer*		    m_pCachedReadBuffers [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_READ_BUFFERS];
	uint32		    m_uReadBufferBindMask [ShaderStage::NUM];

	ConstantBuffer*		m_pCachedConstantBuffers [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_CONSTANT_BUFFERS];
	uint32				    m_uConstantBufferBindMask[ShaderStage::NUM];

	TextureSampler*		m_pCachedTextureSamplers [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_SAMPLERS];
	uint32				    m_uTextureSamplerBindMask[ShaderStage::NUM];

	Texture*			    m_pCachedRenderTargets [FANCY_MAX_NUM_RENDERTARGETS];
  GLuint            m_uCachedFBO;
  GLuint            m_uFBOpool[kPoolSizeFBO];

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
  uint32              m_uColorWriteMask;
};

} // end of namespace GL4
} // end of namespace Rendering
} // end of namespace Core
} // end of namespace FANCY


#endif  // INCLUDE_RENDERERGL4_H