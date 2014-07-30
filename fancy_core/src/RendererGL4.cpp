#include "RendererGL4.h"
#include "MathUtil.h"
#include "Texture.h"
#include "GLDebug.h"
#include "AdapterGL4.h"

using namespace FANCY::Core::Rendering::GL4;
//-----------------------------------------------------------------------//
#define GL_SET_CAP(eCap, bEnabled) { if(bEnabled) glEnable(eCap); else glDisable(eCap); }
#define GL_SET_CAPi(eCap, index, bEnabled) { if(bEnabled) glEnablei(eCap, index); else glDisablei(eCap, index); }
//-----------------------------------------------------------------------//
static enum class DepthStencilRebindFlags {
  DEPTH_TEST              = 0x00000001,
  DEPTH_WRITE             = 0x00000002,
  DEPTH_COMP_FUNC         = 0x00000004,
  STENCIL_ENABLED         = 0x00000008,
  STENCIL_TWO_SIDED       = 0x00000010,
  STENCIL_REF             = 0x00000020,
  STENCIL_READ_MASK       = 0x00000040,
  STENCIL_COMP_FUNC       = 0x00000080,
  STENCIL_WRITE_MASK      = 0x00000100,
  STENCIL_FAIL_OP         = 0x00000200,
  STENCIL_DEPTH_FAIL_OP   = 0x00000400,
  STENCIL_PASS_OP         = 0x00000800
};
//-----------------------------------------------------------------------//
static enum class BlendStateRebindFlags {
  ALPHA_TO_COVERAGE     = 0x00000001,
  BLENDSTATE_PER_RT     = 0x00000002,
  ALPHA_SEPARATE_BLEND  = 0x00000004,
  BLEND_ENABLED         = 0x00000008,
  SRC_BLEND             = 0x00000010,
  DEST_BLEND            = 0x00000020,
  BLEND_OP              = 0x00000040,
  SRC_BLEND_ALPHA       = 0x00000080,
  DEST_BLEND_ALPHA      = 0x00000100,
  BLEND_OP_ALPHA        = 0x00000200,
  RT_WRITE_MASK         = 0x00000400
};
//-----------------------------------------------------------------------//
static enum class PipelineRebindFlags {
  NONE                        = 0x0000,
  DEPTHSTENCIL                = 0x0001,
  BLENDING                    = 0x0002,
  FILLMODE                    = 0x0004,
  CULLMODE                    = 0x0008,
  WINDINGORDER                = 0x0010,
  RENDERTARGETS               = 0x0020,
  ALL                         = 0xFFFF
};
//-----------------------------------------------------------------------//
static enum class ResourceRebindFlags {
  NONE              = 0x0000,
  READ_TEXTURES     = 0x0001,
  READ_BUFFERS      = 0x0002,
  CONSTANT_BUFFERS  = 0x0004,
  TEXTURE_SAMPLERS  = 0x0008,
  GPU_PROGRAMS      = 0x0010,
  ALL               = 0xFFFF
};
//---------------------------------------------------------------------------//
GLenum getColorAttachmentFromIndex(uint8 i) 
{
  switch (i)
  {
    case 0: return GL_COLOR_ATTACHMENT0;
    case 1: return GL_COLOR_ATTACHMENT1;
    case 2: return GL_COLOR_ATTACHMENT2;
    case 3: return GL_COLOR_ATTACHMENT3;
    case 4: return GL_COLOR_ATTACHMENT4;
    case 5: return GL_COLOR_ATTACHMENT5;
    case 6: return GL_COLOR_ATTACHMENT6;
    case 7: return GL_COLOR_ATTACHMENT7;
  default:
    ASSERT_M(false, "Color attachment index not supported");
    return GL_COLOR_ATTACHMENT8;
    break;
  }
}
//---------------------------------------------------------------------------//
RendererGL4::RendererGL4() :
  m_uPipelineRebindMask(0),
  m_uGPUprogramBindMask(0),
  m_uDepthStencilRebindMask(0),
  m_uBlendStateRebindMask(0),
  m_u8BlendStateRebindRTmask(0),
  m_u8BlendStateRebindRTcount(0),
  m_pCachedDepthStencilTarget(0),
  m_uCachedFBO(0),
  LoadableObject::LoadableObject()
{
  memset(m_uResourceRebindMask, 0, sizeof(m_uResourceRebindMask));

  memset(m_pCachedReadTextures, 0, sizeof(m_pCachedReadTextures));
  memset(m_uReadTextureBindMask, 0, sizeof(m_uReadTextureBindMask));

  memset(m_pCachedReadBuffers, 0, sizeof(m_pCachedReadBuffers));
  memset(m_uReadBufferBindMask, 0, sizeof(m_uReadBufferBindMask));

  memset(m_pCachedConstantBuffers, 0, sizeof(m_pCachedConstantBuffers));
  memset(m_uConstantBufferBindMask, 0, sizeof(m_uConstantBufferBindMask));

  memset(m_pCachedTextureSamplers, 0, sizeof(m_pCachedTextureSamplers));
  memset(m_uTextureSamplerBindMask, 0, sizeof(m_uTextureSamplerBindMask));

  memset(m_pCachedRenderTargets, 0, sizeof(m_pCachedRenderTargets));

  memset(m_pBoundGPUPrograms, 0, sizeof(m_pBoundGPUPrograms)); 
}
//-----------------------------------------------------------------------//
RendererGL4::~RendererGL4()
{

}
//-----------------------------------------------------------------------//
bool RendererGL4::_init()
{
  
}
//-----------------------------------------------------------------------//
bool RendererGL4::_destroy()
{

}
//-----------------------------------------------------------------------//
void RendererGL4::postInit()
{
  // TODO: Set the render-system to the OpenGL-default values
}
//-----------------------------------------------------------------------//
void RendererGL4::setDepthStencilState(const DepthStencilState& clDepthStencilState)
{
  if (clDepthStencilState == m_clDepthStencilState) {
    return;
  }
  
  uint32 uDepthStencilRebindMask = m_uDepthStencilRebindMask;

  if(m_clDepthStencilState.bDepthTestEnabled != clDepthStencilState.bDepthTestEnabled) {
    m_clDepthStencilState.bDepthTestEnabled = clDepthStencilState.bDepthTestEnabled;
    uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::DEPTH_TEST;
  }

  if(m_clDepthStencilState.bDepthWriteEnabled != clDepthStencilState.bDepthWriteEnabled) {
    m_clDepthStencilState.bDepthWriteEnabled = clDepthStencilState.bDepthWriteEnabled;
    uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::DEPTH_WRITE;
  }

  if(m_clDepthStencilState.eDepthCompFunc != clDepthStencilState.eDepthCompFunc) {
    m_clDepthStencilState.eDepthCompFunc = clDepthStencilState.eDepthCompFunc;
    uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::DEPTH_COMP_FUNC;
  }

  if(m_clDepthStencilState.bStencilEnabled != clDepthStencilState.bStencilEnabled) {
    m_clDepthStencilState.bStencilEnabled = clDepthStencilState.bStencilEnabled;
    uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_ENABLED;
  }

  if(m_clDepthStencilState.bTwoSidedStencil != clDepthStencilState.bTwoSidedStencil) {
    m_clDepthStencilState.bTwoSidedStencil = clDepthStencilState.bTwoSidedStencil;
    uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_TWO_SIDED;
  }

  if(m_clDepthStencilState.iStencilRef != clDepthStencilState.iStencilRef) {
    m_clDepthStencilState.iStencilRef = clDepthStencilState.iStencilRef;
    uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_REF;
  }

  if(m_clDepthStencilState.uStencilReadMask != clDepthStencilState.uStencilReadMask) {
    m_clDepthStencilState.uStencilReadMask = clDepthStencilState.uStencilReadMask;
    uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_READ_MASK;
  }

  for(uint8 iFaceType = 0; iFaceType < (uint) FaceType::NUM; ++iFaceType)
  {
    if(m_clDepthStencilState.uStencilWriteMask[iFaceType] != clDepthStencilState.uStencilWriteMask[iFaceType]) {
      m_clDepthStencilState.uStencilWriteMask[iFaceType] = clDepthStencilState.uStencilWriteMask[iFaceType];
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_WRITE_MASK;
    }

    if(m_clDepthStencilState.eStencilFailOp[iFaceType] != clDepthStencilState.eStencilFailOp[iFaceType]) {
      m_clDepthStencilState.eStencilFailOp[iFaceType] = clDepthStencilState.eStencilFailOp[iFaceType];
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_FAIL_OP;
    }

    if (m_clDepthStencilState.eStencilDepthFailOp[iFaceType] != clDepthStencilState.eStencilDepthFailOp[iFaceType]) {
      m_clDepthStencilState.eStencilDepthFailOp[iFaceType] = clDepthStencilState.eStencilDepthFailOp[iFaceType];
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_DEPTH_FAIL_OP;
    }

    if (m_clDepthStencilState.eStencilPassOp[iFaceType] != clDepthStencilState.eStencilPassOp[iFaceType]) {
      m_clDepthStencilState.eStencilPassOp[iFaceType] = clDepthStencilState.eStencilPassOp[iFaceType];
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_PASS_OP;
    }

    if (m_clDepthStencilState.eStencilCompFunc[iFaceType] != clDepthStencilState.eStencilCompFunc[iFaceType]) {
      m_clDepthStencilState.eStencilCompFunc[iFaceType] = clDepthStencilState.eStencilCompFunc[iFaceType];
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_COMP_FUNC;
    }
  }
  
  m_clDepthStencilState.u32Hash = clDepthStencilState.u32Hash;

  m_uDepthStencilRebindMask = uDepthStencilRebindMask;
  m_uPipelineRebindMask |= (uint) PipelineRebindFlags::DEPTHSTENCIL;  
}
//-----------------------------------------------------------------------//
void RendererGL4::setBlendState(const BlendState& clBlendState)
{
  if (m_clBlendState == clBlendState) {
    return;
  }

  uint32 uBlendStateRebindMask = m_uBlendStateRebindMask;
  uint8 u8BlendStateRebindRTmask = m_u8BlendStateRebindRTmask;
  uint8 u8BlendStateRebindRTcount = m_u8BlendStateRebindRTcount;

  if (m_clBlendState.bAlphaToCoverageEnabled != clBlendState.bAlphaToCoverageEnabled) {
    m_clBlendState.bAlphaToCoverageEnabled = clBlendState.bAlphaToCoverageEnabled;
    uBlendStateRebindMask |= (uint) BlendStateRebindFlags::ALPHA_TO_COVERAGE;
  }

  if (m_clBlendState.bBlendStatePerRT != clBlendState.bBlendStatePerRT) {
    m_clBlendState.bBlendStatePerRT = clBlendState.bBlendStatePerRT;
    uBlendStateRebindMask |= (uint) BlendStateRebindFlags::BLENDSTATE_PER_RT;
  }

  const uint8 rtUpperBound = m_clBlendState.bBlendStatePerRT ? FANCY_MAX_NUM_RENDERTARGETS : 1;
  for (uint8 iRT = 0; iRT < rtUpperBound; ++iRT)
  {
    if (m_clBlendState.bAlphaSeparateBlend[iRT] != clBlendState.bAlphaSeparateBlend[iRT]) {
      m_clBlendState.bAlphaSeparateBlend[iRT] = clBlendState.bAlphaSeparateBlend[iRT];
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::ALPHA_SEPARATE_BLEND;
      u8BlendStateRebindRTmask |= (1 << iRT);
      u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
    }

    if (m_clBlendState.bBlendEnabled[iRT] != clBlendState.bBlendEnabled[iRT]) {
      m_clBlendState.bBlendEnabled[iRT] = clBlendState.bBlendEnabled[iRT];
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::BLEND_ENABLED;
      u8BlendStateRebindRTmask |= (1 << iRT);
      u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
    }

    if (m_clBlendState.eSrcBlend[iRT] != clBlendState.eSrcBlend[iRT]) {
      m_clBlendState.eSrcBlend[iRT] = clBlendState.eSrcBlend[iRT];
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::SRC_BLEND;
      u8BlendStateRebindRTmask |= (1 << iRT);
      u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
    }

    if (m_clBlendState.eDestBlend[iRT] != clBlendState.eDestBlend[iRT]) {
      m_clBlendState.eDestBlend[iRT] = clBlendState.eDestBlend[iRT];
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::DEST_BLEND;
      u8BlendStateRebindRTmask |= (1 << iRT);
      u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
    }

    if (m_clBlendState.eBlendOp[iRT] != clBlendState.eBlendOp[iRT]) {
      m_clBlendState.eBlendOp[iRT] = clBlendState.eBlendOp[iRT];
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::BLEND_OP;
      u8BlendStateRebindRTmask |= (1 << iRT);
      u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
    }

    if (m_clBlendState.eSrcBlendAlpha[iRT] != clBlendState.eSrcBlendAlpha[iRT]) {
      m_clBlendState.eSrcBlendAlpha[iRT] = clBlendState.eSrcBlendAlpha[iRT];
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::SRC_BLEND_ALPHA;
      u8BlendStateRebindRTmask |= (1 << iRT);
      u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
    }

    if (m_clBlendState.eDestBlendAlpha[iRT] != clBlendState.eDestBlendAlpha[iRT]) {
      m_clBlendState.eDestBlendAlpha[iRT] = clBlendState.eDestBlendAlpha[iRT];
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::DEST_BLEND_ALPHA;
      u8BlendStateRebindRTmask |= (1 << iRT);
      u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
    }

    if (m_clBlendState.eBlendOpAlpha[iRT] != clBlendState.eBlendOpAlpha[iRT]) {
      m_clBlendState.eBlendOpAlpha[iRT] = clBlendState.eBlendOpAlpha[iRT];
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::BLEND_OP_ALPHA;
      u8BlendStateRebindRTmask |= (1 << iRT);
      u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
    }

    if (m_clBlendState.uRTwriteMask[iRT] != clBlendState.uRTwriteMask[iRT]) {
      m_clBlendState.uRTwriteMask[iRT] = clBlendState.uRTwriteMask[iRT];
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::RT_WRITE_MASK;
      u8BlendStateRebindRTmask |= (1 << iRT);
      u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
    }
  }
  
  m_clBlendState.u32Hash = clBlendState.u32Hash;

  m_uBlendStateRebindMask = uBlendStateRebindMask;
  m_u8BlendStateRebindRTcount = u8BlendStateRebindRTcount;
  m_u8BlendStateRebindRTmask = u8BlendStateRebindRTmask;

  m_uPipelineRebindMask |= (uint) PipelineRebindFlags::BLENDING;
}
//-----------------------------------------------------------------------//
void RendererGL4::setFillMode( const FillMode eFillMode )
{
  if(getFillMode() == eFillMode) {
    return;
  }

  m_eFillMode = eFillMode;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::FILLMODE);
}
//-----------------------------------------------------------------------//
void RendererGL4::setCullMode( const CullMode eCullMode )
{
  if(getCullMode() == eCullMode) {
    return;
  }

  m_eCullMode = eCullMode;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::CULLMODE);
}
//-----------------------------------------------------------------------//
void RendererGL4::setWindingOrder( const WindingOrder eWindingOrder ) 
{
  if(getWindingOrder() == eWindingOrder) {
    return;
  }

  m_eWindingOrder = eWindingOrder;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::WINDINGORDER);
}
//-----------------------------------------------------------------------//
void RendererGL4::setRenderTarget( Texture* pRTTexture, const uint8 u8RenderTargetIndex )
{
  ASSERT_M(u8RenderTargetIndex < FANCY_MAX_NUM_RENDERTARGETS, "RenderTarget-index not supported");
  //ASSERT_M(pRTTexture->isDepthStencilTexture(), "Tried to set depthStencil-texture as color-rendertexture");
  
  if(getCachedRenderTarget(u8RenderTargetIndex) == pRTTexture) {
    return;
  }

  m_pCachedRenderTargets[u8RenderTargetIndex] = pRTTexture;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::RENDERTARGETS);
}
//---------------------------------------------------------------------------//
void RendererGL4::setDepthStencilRenderTarget( Texture* pDStexture )
{
  if (getCachedDepthStencilRenderTarget() == pDStexture) {
    return;
  }

  m_pCachedDepthStencilTarget = pDStexture;
  m_uPipelineRebindMask |= (uint) PipelineRebindFlags::RENDERTARGETS;
}
//-----------------------------------------------------------------------//
void RendererGL4::removeAllRenderTargets()
{
  for (uint8 i = 0; i < FANCY_MAX_NUM_RENDERTARGETS; ++i) {
    setRenderTarget(nullptr, i);
  }
}
//-----------------------------------------------------------------------//
void RendererGL4::setReadTexture( Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8RegisterIndex )
{
  ASSERT_M(u8RegisterIndex < FANCY_MAX_NUM_BOUND_READ_TEXTURES, "Referenced an undefined texture register");
  
  if(m_pCachedReadTextures[static_cast<uint>(eShaderStage)][u8RegisterIndex] == pTexture) {
    return;
  }

  m_pCachedReadTextures[static_cast<uint>(eShaderStage)][u8RegisterIndex] = pTexture;
  m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::READ_TEXTURES);
}
//-----------------------------------------------------------------------//
void RendererGL4::setReadBuffer( Buffer* pBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex )
{
  ASSERT_M(u8RegisterIndex < FANCY_MAX_NUM_BOUND_READ_BUFFERS, "Referenced an undefined buffer register");
  
  if(m_pCachedReadBuffers[static_cast<uint>(eShaderStage)][u8RegisterIndex] == pBuffer) {
    return;
  }

  m_pCachedReadBuffers[static_cast<uint>(eShaderStage)][u8RegisterIndex] = pBuffer;
  m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::READ_BUFFERS);
}
//-----------------------------------------------------------------------//
void RendererGL4::setConstantBuffer( ConstantBuffer* pConstantBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex )
{
  ASSERT_M(u8RegisterIndex < FANCY_MAX_NUM_BOUND_CONSTANT_BUFFERS, "Referenced an undefined constant buffer register");
  
  if(m_pCachedConstantBuffers[static_cast<uint>(eShaderStage)][u8RegisterIndex] == pConstantBuffer) {
    return;
  }

  m_pCachedConstantBuffers[static_cast<uint>(eShaderStage)][u8RegisterIndex] = pConstantBuffer;
  m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::CONSTANT_BUFFERS);
}
//-----------------------------------------------------------------------//
void RendererGL4::setTextureSampler( TextureSampler* pSampler, const ShaderStage eShaderStage, const uint8 u8RegisterIndex )
{
  ASSERT_M(u8RegisterIndex < FANCY_MAX_NUM_BOUND_SAMPLERS, "Referenced an undefined sampler register");
  
  if(m_pCachedTextureSamplers[static_cast<uint>(eShaderStage)][u8RegisterIndex] == pSampler) {
    return;
  }

  m_pCachedTextureSamplers[static_cast<uint>(eShaderStage)][u8RegisterIndex] = pSampler;
  m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::TEXTURE_SAMPLERS);
}
//-----------------------------------------------------------------------//
void RendererGL4::setGPUProgram( GPUProgram* pProgram, const ShaderStage eShaderStage )
{
  if(m_pBoundGPUPrograms[static_cast<uint>(eShaderStage)] == pProgram) {
    return;
  }

  m_pBoundGPUPrograms[static_cast<uint>(eShaderStage)] = pProgram;
  m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::GPU_PROGRAMS);
}
//-----------------------------------------------------------------------//
void RendererGL4::bindStatesToPipeline()
{
  if (m_uPipelineRebindMask == 0) {
    return;
  }

  if (m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::WINDINGORDER) > 0) {
    glFrontFace(Adapter::toGLType(m_eWindingOrder));
  }

  if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::CULLMODE)) > 0) {
    if (m_eCullMode == CullMode::NONE) {
      GL_SET_CAP(GL_CULL_FACE, false);  // Disables culling in general
    } else {
      glCullFace(Adapter::toGLType(m_eCullMode));
    }
  }

  if (m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::FILLMODE) > 0) {
    glPolygonMode(GL_FRONT_AND_BACK, Adapter::toGLType(m_eWindingOrder));
  }

  if (m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL) > 0) {
    bindDepthStencilState();
  }

  if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::BLENDING)) > 0) {
    bindBlendState();
  }

  if (m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::RENDERTARGETS) > 0) {
    bindRenderTargets();
  }

  m_uPipelineRebindMask = 0;
}
//-----------------------------------------------------------------------//
void RendererGL4::bindResourcesToPipeline( const ShaderStage eShaderStage )
{

}
//-----------------------------------------------------------------------//
void RendererGL4::bindBlendState()
{
  ASSERT_M(m_uBlendStateRebindMask > 0, 
    "pipelineRebindMask not in sync with blendStateRebindMask");

  uint32 uBlendStateRebindMask = m_uBlendStateRebindMask;
  bool bBlendingPerRTenabled = m_clBlendState.bBlendStatePerRT;

  if ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::ALPHA_TO_COVERAGE)) > 0)   {
     GL_SET_CAP(GL_SAMPLE_ALPHA_TO_COVERAGE, m_clBlendState.bAlphaToCoverageEnabled);
  }

  if ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLENDSTATE_PER_RT)) > 0)  {
    // Noting todo. Just use glBlendFuncSeperatei() and related functions if enabled
  }

  if (m_clBlendState.bBlendStatePerRT) {
    _bindBlendValuesMultiRT(uBlendStateRebindMask, m_u8BlendStateRebindRTcount, m_u8BlendStateRebindRTmask);
  }
  else {
    _bindBlendValuesSingleRT(uBlendStateRebindMask);
  }

  m_u8BlendStateRebindRTcount = 0;
  m_u8BlendStateRebindRTmask = 0;
  m_uBlendStateRebindMask = 0;
}
//-----------------------------------------------------------------------//
void RendererGL4::_bindBlendValuesSingleRT(const uint32 uBlendStateRebindMask)
{
  if ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLEND_ENABLED)) > 0) {
    GL_SET_CAP(GL_BEVEL_NV, m_clBlendState.bBlendEnabled[0]);
  }

  const bool bSeperateBlend = m_clBlendState.bAlphaSeparateBlend[0];

  const bool bApplyBlendFunc = 
    uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::SRC_BLEND) > 0 ||
    uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::DEST_BLEND) > 0 ||
    (bSeperateBlend &&
      (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::SRC_BLEND_ALPHA) > 0 ||
       uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::DEST_BLEND_ALPHA)> 0));

  const bool bApplyBlendOp =
    uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLEND_OP) > 0 ||
    (bSeperateBlend &&
      (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLEND_OP_ALPHA) > 0));

  if(bApplyBlendFunc) 
  {
    if (!bSeperateBlend) {
      glBlendFunc(Adapter::toGLType(m_clBlendState.eSrcBlend[0]), Adapter::toGLType(m_clBlendState.eDestBlend[0]));
    } else {
      glBlendFuncSeparate(Adapter::toGLType(m_clBlendState.eSrcBlend[0]), Adapter::toGLType(m_clBlendState.eDestBlend[0]),
                          Adapter::toGLType(m_clBlendState.eSrcBlendAlpha[0]), Adapter::toGLType(m_clBlendState.eDestBlendAlpha[0]));
    }
  }

  if (bApplyBlendOp)
  {
    if (!bSeperateBlend) {
      glBlendEquation(Adapter::toGLType(m_clBlendState.eBlendOp[0]));
    } else {
      glBlendEquationSeparate(Adapter::toGLType(m_clBlendState.eBlendOp[0]), Adapter::toGLType(m_clBlendState.eBlendOpAlpha[0]));
    }
  }
  
  if ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::RT_WRITE_MASK)) > 0) {
    glColorMask(
      (m_clBlendState.uRTwriteMask[0] & 0xFF00000000) > 0,
      (m_clBlendState.uRTwriteMask[0] & 0x00FF000000) > 0,
      (m_clBlendState.uRTwriteMask[0] & 0x0000FF0000) > 0,
      (m_clBlendState.uRTwriteMask[0] & 0x000000FF00) > 0);
  }
}
//-----------------------------------------------------------------------//
void RendererGL4::_bindBlendValuesMultiRT(const uint32 uBlendStateRebindMask, 
     const uint8 u8BlendStateRebindRTcount, const uint8 u8BlendStateRebindRTmask)
{
  for (uint8 iRT = 0; iRT < u8BlendStateRebindRTcount; ++iRT)
  {
    if ((u8BlendStateRebindRTmask & (1 << iRT)) == 0) {
      continue;
    }

    if ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLEND_ENABLED)) > 0) {
      GL_SET_CAPi(GL_BEVEL_NV, iRT, m_clBlendState.bBlendEnabled[iRT]);
    }

    const bool bSeperateBlend = m_clBlendState.bAlphaSeparateBlend[iRT];

    const bool bApplyBlendFunc = 
      uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::SRC_BLEND) > 0 ||
      uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::DEST_BLEND) > 0 ||
      (bSeperateBlend &&
      (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::SRC_BLEND_ALPHA) > 0 ||
      uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::DEST_BLEND_ALPHA)> 0));

    const bool bApplyBlendOp =
      uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLEND_OP) > 0 ||
      (bSeperateBlend &&
      (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLEND_OP_ALPHA) > 0));

    if(bApplyBlendFunc) 
    {
      if (!bSeperateBlend) {
        glBlendFunci(iRT, Adapter::toGLType(m_clBlendState.eSrcBlend[iRT]), Adapter::toGLType(m_clBlendState.eDestBlend[iRT]));
      } else {
        glBlendFuncSeparatei(iRT, Adapter::toGLType(m_clBlendState.eSrcBlend[iRT]), Adapter::toGLType(m_clBlendState.eDestBlend[iRT]),
          Adapter::toGLType(m_clBlendState.eSrcBlendAlpha[iRT]), Adapter::toGLType(m_clBlendState.eDestBlendAlpha[iRT]));
      }
    }

    if (bApplyBlendOp)
    {
      if (!bSeperateBlend) {
        glBlendEquation(Adapter::toGLType(m_clBlendState.eBlendOp[iRT]));
      } else {
        glBlendEquationSeparate(Adapter::toGLType(m_clBlendState.eBlendOp[iRT]), Adapter::toGLType(m_clBlendState.eBlendOpAlpha[iRT]));
      }
    }

    if ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::RT_WRITE_MASK)) > 0) {
      glColorMaski( iRT,
        (m_clBlendState.uRTwriteMask[iRT] & 0xFF00000000) > 0,
        (m_clBlendState.uRTwriteMask[iRT] & 0x00FF000000) > 0,
        (m_clBlendState.uRTwriteMask[iRT] & 0x0000FF0000) > 0,
        (m_clBlendState.uRTwriteMask[iRT] & 0x000000FF00) > 0);
    }
  }
}
//-----------------------------------------------------------------------//
void RendererGL4::bindDepthStencilState()
{
  uint32 uDepthStencilRebindMask = m_uDepthStencilRebindMask;

  ASSERT_M(m_uDepthStencilRebindMask > 0, 
    "pipelineRebindMask not in sync with depthStencilRebindMask");

  if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::DEPTH_TEST) > 0 ) {
    GL_SET_CAP(GL_DEPTH_TEST, m_clDepthStencilState.bDepthTestEnabled);
  }

  if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::DEPTH_WRITE) > 0 ) {
    glDepthMask(m_clDepthStencilState.bDepthWriteEnabled);
  }

  if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::DEPTH_COMP_FUNC) > 0) {
    glDepthFunc(Adapter::toGLType(m_clDepthStencilState.eDepthCompFunc));
  }

  if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_ENABLED) > 0) {
    GL_SET_CAP(GL_STENCIL_TEST, m_clDepthStencilState.bStencilEnabled);
  }

  if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_TWO_SIDED) > 0) {
    // Nothing special to set here - just use the "separate" calls for stencil-ops and funcs
  }
  const bool bUseTwoSidedStencil = m_clDepthStencilState.bTwoSidedStencil;

  if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_REF) > 0) {
    GL_SET_CAP(GL_STENCIL_TEST, m_clDepthStencilState.bStencilEnabled);
  }

  const bool bNeedStencilFunc =
       (uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_REF) > 0
    || (uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_READ_MASK) > 0
    || (uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_COMP_FUNC) > 0;

  if (bNeedStencilFunc && !bUseTwoSidedStencil) 
  {
    glStencilFunc(Adapter::toGLType(m_clDepthStencilState.eStencilCompFunc[0]),
                  m_clDepthStencilState.iStencilRef,
                  m_clDepthStencilState.uStencilReadMask);
  }
  else if (bNeedStencilFunc)
  {
    glStencilFuncSeparate(Adapter::toGLType(m_clDepthStencilState.eStencilCompFunc[(uint)FaceType::FRONT]),
                          Adapter::toGLType(m_clDepthStencilState.eStencilCompFunc[(uint)FaceType::BACK]),
                          m_clDepthStencilState.iStencilRef,
                          m_clDepthStencilState.uStencilReadMask);
  }

  if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_WRITE_MASK ) > 0 ) {
    if(!bUseTwoSidedStencil) {
      glStencilMask(m_clDepthStencilState.uStencilWriteMask[0]);
    } else {
      glStencilMaskSeparate(GL_FRONT, m_clDepthStencilState.uStencilWriteMask[(uint) FaceType::FRONT]);
      glStencilMaskSeparate(GL_BACK, m_clDepthStencilState.uStencilWriteMask[(uint) FaceType::BACK]);
    }
  }
  
  m_uDepthStencilRebindMask = 0;
}
//-----------------------------------------------------------------------//
void RendererGL4::bindRenderTargets()
{
  // The actual (packed) list of renderTextures to use
  Texture* rtListPatched[FANCY_MAX_NUM_RENDERTARGETS] = {nullptr};
  uint8 u8RenderTextureCount = 0;

  // Detect "holes" in renderTexture-array
  for(uint8 i = 0; i < FANCY_MAX_NUM_RENDERTARGETS; ++i) 
  {
    if (m_pCachedRenderTargets[i] != nullptr) 
    {
      rtListPatched[i] = m_pCachedRenderTargets[i];
      ++u8RenderTextureCount;
    } 
    else 
    {
      // There is a hole in the list: skip it
      while(m_pCachedRenderTargets[i] == nullptr 
        && i < FANCY_MAX_NUM_RENDERTARGETS) {
        ++i;
      }
    }
  }
  
  const GLuint uFBOtoUse = createOrRetrieveFBO(rtListPatched, u8RenderTextureCount, m_pCachedDepthStencilTarget);

  if (uFBOtoUse == m_uCachedFBO) {
    return;
  }
  m_uCachedFBO = uFBOtoUse;
  glBindFramebuffer(GL_FRAMEBUFFER, uFBOtoUse);

  // Default: Enable all drawbuffers. 
  // TODO: Better to determine drawbuffers from blendState::RTwritemask?
  static GLenum drawBuffers[FANCY_MAX_NUM_RENDERTARGETS] = 
  {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, 
  GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6};
  glDrawBuffers(u8RenderTextureCount, drawBuffers);
}
//-----------------------------------------------------------------------//
GLuint RendererGL4::createOrRetrieveFBO(Texture** pRenderTextures, uint8 u8RenderTextureCount, Texture* pDStexture)
{
  size_t hash = 0;
  for (uint8 i = 0; i < u8RenderTextureCount; ++i) {
    MathUtil::hash_combine(hash, reinterpret_cast<size_t>(*pRenderTextures));
  }
  MathUtil::hash_combine(hash, reinterpret_cast<size_t>(pDStexture));

  for (uint8 i = 0; i < _countof(m_FBOpool); ++i) 
  {
    if (m_FBOpool[i].hash == hash) {
      return m_FBOpool[i].glHandle;
    }
  }

  // No suitable FBO found in the pool: create one in a lazy fashion...
  // find the first unused FBO in the pool
  RendererGL4::FBOcacheEntry* pFBOentry = nullptr;
  for (uint8 i = 0; i < _countof(m_FBOpool); ++i) 
  {
    if (m_FBOpool[i].glHandle == GLUINT_HANDLE_INVALID) {
      pFBOentry = &m_FBOpool[i];
      break;
    }
  }

  ASSERT_M(pFBOentry != nullptr, "No free FBO found in the pool. Increase its size?" );

  pFBOentry->hash = hash;
  glGenFramebuffers(1, &pFBOentry->glHandle);
  glBindFramebuffer(GL_FRAMEBUFFER, pFBOentry->glHandle);
  for (uint8 i = 0; i < u8RenderTextureCount; ++i )
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER, getColorAttachmentFromIndex(i), GL_TEXTURE_2D, pRenderTextures[i]->getGLhandle(), 0);
  }
  if (pDStexture) 
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, pDStexture->getGLhandle(), 0);
  }
#if defined (FANCY_RENDERSYSTEM_USE_VALIDATION)
  GLDebug::validateFBOcompleteness();
#endif // FANCY_RENDERSYSTEM_USE_VALIDATION
  glBindFramebuffer(GL_FRAMEBUFFER, m_uCachedFBO);  // Is this necessary?
  
  // TODO: Add support for 1D, 3D, cubemap RenderTargets
}
//-----------------------------------------------------------------------//
