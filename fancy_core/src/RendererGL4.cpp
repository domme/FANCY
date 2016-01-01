#include "RendererGL4.h"
#include "MathUtil.h"
#include "Texture.h"
#include "GLDebug.h"
#include "GeometryData.h"
#include "GPUProgram.h"
#include "TextureSampler.h"
#include "AdapterGL4.h"
#include "vsDebugLib.h"
#include "DebugOutStream.h"

#if defined (RENDERER_OPENGL4)

namespace Fancy { namespace Rendering { namespace GL4 {
//-----------------------------------------------------------------------//
  #define GL_SET_CAP(eCap, bEnabled) { if(bEnabled) glEnable(eCap); else glDisable(eCap); }
  #define GL_SET_CAPi(eCap, index, bEnabled) { if(bEnabled) glEnablei(eCap, index); else glDisablei(eCap, index); }
//-----------------------------------------------------------------------//
  enum class DepthStencilRebindFlags {
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
  enum class BlendStateRebindFlags {
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
  enum class PipelineRebindFlags {
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
  enum class ResourceRebindFlags {
    NONE              = 0x0000,
    READ_TEXTURES     = 0x0001,
    WRITE_TEXTURES    = 0x0002,
    READ_BUFFERS      = 0x0004,
    WRITE_BUFFERS     = 0x0008,
    CONSTANT_BUFFERS  = 0x0010,
    TEXTURE_SAMPLERS  = 0x0020,
    GPU_PROGRAMS      = 0x0040,
    ALL               = 0xFFFF
  };
//---------------------------------------------------------------------------//
  enum class VertexBufferBindingFlags {
    GEOMETRY_STREAM        = 0x001,
    INSTANCE_STREAM_1      = 0x002,
    INSTANCE_STREAM_2      = 0x004,
    INSTANCE_STREAM_3      = 0x008,
    PATCHING_STREAM_VEC4   = 0x010
  };
//---------------------------------------------------------------------------//
  enum class VertexBufferBindingPoints {
    GEOMETRY_STREAM        = 0,
    INSTANCE_STREAM_1      = 1,
    INSTANCE_STREAM_2      = 2,
    INSTANCE_STREAM_3      = 3,
    PATCHING_STREAM        = 4
  };
//---------------------------------------------------------------------------//
 
//---------------------------------------------------------------------------//
  namespace Internal {
    GLuint getGLShaderStageBit(ShaderStage eShaderStage);
    GLenum getColorAttachmentFromIndex(uint32 uIndex);
    GLenum getTextureUnitFromIndex(uint32 uIndex);
    GLuint getGLTextureTypeFromGLSLresourceType(GpuResourceType eResourceType);
  }
//---------------------------------------------------------------------------//
  GLuint Internal::getGLShaderStageBit(ShaderStage eShaderStage)
  {
    switch (eShaderStage)
    {
      case Fancy::Rendering::ShaderStage::VERTEX: return GL_VERTEX_SHADER_BIT;
      case Fancy::Rendering::ShaderStage::FRAGMENT: return GL_FRAGMENT_SHADER_BIT;
      case Fancy::Rendering::ShaderStage::GEOMETRY: return GL_GEOMETRY_SHADER_BIT;
      case Fancy::Rendering::ShaderStage::TESS_HULL: return GL_TESS_CONTROL_SHADER_BIT;
      case Fancy::Rendering::ShaderStage::TESS_DOMAIN: return GL_TESS_EVALUATION_SHADER_BIT;
      case Fancy::Rendering::ShaderStage::COMPUTE: return GL_COMPUTE_SHADER_BIT;
      default: ASSERT(false); return GL_VERTEX_SHADER_BIT;
    }
  }
//---------------------------------------------------------------------------//
  GLenum Internal::getColorAttachmentFromIndex(uint32 uIndex) 
  {
    switch (uIndex)
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
  GLenum Internal::getTextureUnitFromIndex(uint32 uIndex)
  {
    ASSERT(uIndex < kMaxNumReadTextures);
    return GL_TEXTURE0 + uIndex;
  }
//---------------------------------------------------------------------------//
  GLenum Internal::getGLTextureTypeFromGLSLresourceType(GpuResourceType eResourceType)
  {
    switch (eResourceType)
    {
      case Fancy::Rendering::GpuResourceType::TEXTURE_1D:
        return GL_TEXTURE_1D;
      case Fancy::Rendering::GpuResourceType::TEXTURE_2D:
        return GL_TEXTURE_2D;
      case Fancy::Rendering::GpuResourceType::TEXTURE_3D:
        return GL_TEXTURE_3D;
      case Fancy::Rendering::GpuResourceType::TEXTURE_CUBE:
        return GL_TEXTURE_CUBE_MAP;
      case Fancy::Rendering::GpuResourceType::TEXTURE_1D_SHADOW:
        return GL_TEXTURE_1D;
      case Fancy::Rendering::GpuResourceType::TEXTURE_2D_SHADOW:
        return GL_TEXTURE_2D;
      case Fancy::Rendering::GpuResourceType::TEXTURE_CUBE_SHADOW:
        return GL_TEXTURE_CUBE_MAP;
      case Fancy::Rendering::GpuResourceType::BUFFER_TEXTURE:
        return GL_TEXTURE_BUFFER;
      default:
        ASSERT_M(false, "The provided glsl resource is no texture");
    }

    return GL_TEXTURE_1D;
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  RendererGL4::RendererGL4() :
    m_uPipelineRebindMask(UINT_MAX),
    m_uDepthStencilRebindMask(UINT_MAX),
    m_uBlendStateRebindMask(UINT_MAX),
    m_u8BlendStateRebindRTmask(UINT_MAX),
    m_u8BlendStateRebindRTcount(1u),
    m_pCachedDepthStencilTarget(nullptr),
    m_uCurrentFBO(0u),
    m_uCurrentGpuProgramPipeline(0u),
    m_uCurrentVAO(0u),
    m_uViewportParams(0u, 0u, 1u, 1u),
    m_bViewportDirty(true) ,
    m_clBlendState(ObjectName()),
    m_clDepthStencilState(ObjectName())
  {
    memset(m_uResourceRebindMask, UINT_MAX, sizeof(m_uResourceRebindMask));

    memset(m_pCachedReadTextures, 0, sizeof(m_pCachedReadTextures));
    memset(m_uReadTextureBindMask, UINT_MAX, sizeof(m_uReadTextureBindMask));
    memset(m_uNumReadTexturesToBind, 0u, sizeof(m_uNumReadTexturesToBind));

    memset(m_pCachedReadBuffers, 0, sizeof(m_pCachedReadBuffers));
    memset(m_uReadBufferBindMask, UINT_MAX, sizeof(m_uReadBufferBindMask));
    memset(m_uNumReadBuffersToBind, 0u, sizeof(m_uNumReadBuffersToBind));

    memset(m_pCachedConstantBuffers, 0, sizeof(m_pCachedConstantBuffers));
    memset(m_uConstantBufferBindMask, UINT_MAX, sizeof(m_uConstantBufferBindMask));
    memset(m_uNumConstantBuffersToBind, 0u, sizeof(m_uNumConstantBuffersToBind));

    memset(m_pCachedTextureSamplers, 0, sizeof(m_pCachedTextureSamplers));
    memset(m_uTextureSamplerBindMask, UINT_MAX, sizeof(m_uTextureSamplerBindMask));
    memset(m_uNumTextureSamplersToBind, 0u, sizeof(m_uNumTextureSamplersToBind));

    memset(m_pCachedRenderTargets, 0, sizeof(m_pCachedRenderTargets));
    // Don't try to set non-existent rendertargets
    m_uPipelineRebindMask &= ~(uint32) PipelineRebindFlags::RENDERTARGETS;

    memset(m_pBoundGPUPrograms, 0, sizeof(m_pBoundGPUPrograms));

#if defined (_DEBUG)
    VSDebugLib::init(&DebugOutStream::out);
    VSDebugLib::enableLowSeverityMessages(true);
#endif  // _DEBUG
  }
//-----------------------------------------------------------------------//
  RendererGL4::~RendererGL4()
  {
    // TODO: Release all pooled GL-Objects
  }
//-----------------------------------------------------------------------//

//-----------------------------------------------------------------------//
  void RendererGL4::postInit()
  {
    // TODO: Set the render-system to the OpenGL-default values


  }
//---------------------------------------------------------------------------//
  void RendererGL4::setViewport( const glm::uvec4& uViewportParams )
  {
    if (m_uViewportParams != uViewportParams)
    {
      m_bViewportDirty = true;
      m_uViewportParams = uViewportParams;
    }
  }
//-----------------------------------------------------------------------//
  void RendererGL4::setDepthStencilState(const DepthStencilState& clDepthStencilState)
  {
    if (clDepthStencilState == m_clDepthStencilState) {
      return;
    }
  
    uint32 uDepthStencilRebindMask = m_uDepthStencilRebindMask;

    if(m_clDepthStencilState.m_bDepthTestEnabled != clDepthStencilState.m_bDepthTestEnabled) {
      m_clDepthStencilState.m_bDepthTestEnabled = clDepthStencilState.m_bDepthTestEnabled;
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::DEPTH_TEST;
    }

    if(m_clDepthStencilState.m_bDepthWriteEnabled != clDepthStencilState.m_bDepthWriteEnabled) {
      m_clDepthStencilState.m_bDepthWriteEnabled = clDepthStencilState.m_bDepthWriteEnabled;
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::DEPTH_WRITE;
    }

    if(m_clDepthStencilState.m_eDepthCompFunc != clDepthStencilState.m_eDepthCompFunc) {
      m_clDepthStencilState.m_eDepthCompFunc = clDepthStencilState.m_eDepthCompFunc;
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::DEPTH_COMP_FUNC;
    }

    if(m_clDepthStencilState.m_bStencilEnabled != clDepthStencilState.m_bStencilEnabled) {
      m_clDepthStencilState.m_bStencilEnabled = clDepthStencilState.m_bStencilEnabled;
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_ENABLED;
    }

    if(m_clDepthStencilState.m_bTwoSidedStencil != clDepthStencilState.m_bTwoSidedStencil) {
      m_clDepthStencilState.m_bTwoSidedStencil = clDepthStencilState.m_bTwoSidedStencil;
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_TWO_SIDED;
    }

    if(m_clDepthStencilState.m_iStencilRef != clDepthStencilState.m_iStencilRef) {
      m_clDepthStencilState.m_iStencilRef = clDepthStencilState.m_iStencilRef;
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_REF;
    }

    if(m_clDepthStencilState.m_uStencilReadMask != clDepthStencilState.m_uStencilReadMask) {
      m_clDepthStencilState.m_uStencilReadMask = clDepthStencilState.m_uStencilReadMask;
      uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_READ_MASK;
    }

    for(uint8 iFaceType = 0; iFaceType < (uint) FaceType::NUM; ++iFaceType)
    {
      if(m_clDepthStencilState.m_uStencilWriteMask[iFaceType] != clDepthStencilState.m_uStencilWriteMask[iFaceType]) {
        m_clDepthStencilState.m_uStencilWriteMask[iFaceType] = clDepthStencilState.m_uStencilWriteMask[iFaceType];
        uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_WRITE_MASK;
      }

      if(m_clDepthStencilState.m_eStencilFailOp[iFaceType] != clDepthStencilState.m_eStencilFailOp[iFaceType]) {
        m_clDepthStencilState.m_eStencilFailOp[iFaceType] = clDepthStencilState.m_eStencilFailOp[iFaceType];
        uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_FAIL_OP;
      }

      if (m_clDepthStencilState.m_eStencilDepthFailOp[iFaceType] != clDepthStencilState.m_eStencilDepthFailOp[iFaceType]) {
        m_clDepthStencilState.m_eStencilDepthFailOp[iFaceType] = clDepthStencilState.m_eStencilDepthFailOp[iFaceType];
        uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_DEPTH_FAIL_OP;
      }

      if (m_clDepthStencilState.m_eStencilPassOp[iFaceType] != clDepthStencilState.m_eStencilPassOp[iFaceType]) {
        m_clDepthStencilState.m_eStencilPassOp[iFaceType] = clDepthStencilState.m_eStencilPassOp[iFaceType];
        uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_PASS_OP;
      }

      if (m_clDepthStencilState.m_eStencilCompFunc[iFaceType] != clDepthStencilState.m_eStencilCompFunc[iFaceType]) {
        m_clDepthStencilState.m_eStencilCompFunc[iFaceType] = clDepthStencilState.m_eStencilCompFunc[iFaceType];
        uDepthStencilRebindMask |= (uint) DepthStencilRebindFlags::STENCIL_COMP_FUNC;
      }
    }
  
    m_clDepthStencilState.updateHash();

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

    if (m_clBlendState.myAlphaToCoverageEnabled != clBlendState.myAlphaToCoverageEnabled) {
      m_clBlendState.myAlphaToCoverageEnabled = clBlendState.myAlphaToCoverageEnabled;
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::ALPHA_TO_COVERAGE;
    }

    if (m_clBlendState.myBlendStatePerRT != clBlendState.myBlendStatePerRT) {
      m_clBlendState.myBlendStatePerRT = clBlendState.myBlendStatePerRT;
      uBlendStateRebindMask |= (uint) BlendStateRebindFlags::BLENDSTATE_PER_RT;
    }

    const uint8 rtUpperBound = m_clBlendState.myBlendStatePerRT ? kMaxNumRenderTargets : 1;
    for (uint8 iRT = 0; iRT < rtUpperBound; ++iRT)
    {
      if (m_clBlendState.myAlphaSeparateBlend[iRT] != clBlendState.myAlphaSeparateBlend[iRT]) {
        m_clBlendState.myAlphaSeparateBlend[iRT] = clBlendState.myAlphaSeparateBlend[iRT];
        uBlendStateRebindMask |= (uint) BlendStateRebindFlags::ALPHA_SEPARATE_BLEND;
        u8BlendStateRebindRTmask |= (1 << iRT);
        u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
      }

      if (m_clBlendState.myBlendEnabled[iRT] != clBlendState.myBlendEnabled[iRT]) {
        m_clBlendState.myBlendEnabled[iRT] = clBlendState.myBlendEnabled[iRT];
        uBlendStateRebindMask |= (uint) BlendStateRebindFlags::BLEND_ENABLED;
        u8BlendStateRebindRTmask |= (1 << iRT);
        u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
      }

      if (m_clBlendState.mySrcBlend[iRT] != clBlendState.mySrcBlend[iRT]) {
        m_clBlendState.mySrcBlend[iRT] = clBlendState.mySrcBlend[iRT];
        uBlendStateRebindMask |= (uint) BlendStateRebindFlags::SRC_BLEND;
        u8BlendStateRebindRTmask |= (1 << iRT);
        u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
      }

      if (m_clBlendState.myDestBlend[iRT] != clBlendState.myDestBlend[iRT]) {
        m_clBlendState.myDestBlend[iRT] = clBlendState.myDestBlend[iRT];
        uBlendStateRebindMask |= (uint) BlendStateRebindFlags::DEST_BLEND;
        u8BlendStateRebindRTmask |= (1 << iRT);
        u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
      }

      if (m_clBlendState.myBlendOp[iRT] != clBlendState.myBlendOp[iRT]) {
        m_clBlendState.myBlendOp[iRT] = clBlendState.myBlendOp[iRT];
        uBlendStateRebindMask |= (uint) BlendStateRebindFlags::BLEND_OP;
        u8BlendStateRebindRTmask |= (1 << iRT);
        u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
      }

      if (m_clBlendState.mySrcBlendAlpha[iRT] != clBlendState.mySrcBlendAlpha[iRT]) {
        m_clBlendState.mySrcBlendAlpha[iRT] = clBlendState.mySrcBlendAlpha[iRT];
        uBlendStateRebindMask |= (uint) BlendStateRebindFlags::SRC_BLEND_ALPHA;
        u8BlendStateRebindRTmask |= (1 << iRT);
        u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
      }

      if (m_clBlendState.myDestBlendAlpha[iRT] != clBlendState.myDestBlendAlpha[iRT]) {
        m_clBlendState.myDestBlendAlpha[iRT] = clBlendState.myDestBlendAlpha[iRT];
        uBlendStateRebindMask |= (uint) BlendStateRebindFlags::DEST_BLEND_ALPHA;
        u8BlendStateRebindRTmask |= (1 << iRT);
        u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
      }

      if (m_clBlendState.myBlendOpAlpha[iRT] != clBlendState.myBlendOpAlpha[iRT]) {
        m_clBlendState.myBlendOpAlpha[iRT] = clBlendState.myBlendOpAlpha[iRT];
        uBlendStateRebindMask |= (uint) BlendStateRebindFlags::BLEND_OP_ALPHA;
        u8BlendStateRebindRTmask |= (1 << iRT);
        u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
      }

      if (m_clBlendState.myRTwriteMask[iRT] != clBlendState.myRTwriteMask[iRT]) {
        m_clBlendState.myRTwriteMask[iRT] = clBlendState.myRTwriteMask[iRT];
        uBlendStateRebindMask |= (uint) BlendStateRebindFlags::RT_WRITE_MASK;
        u8BlendStateRebindRTmask |= (1u << iRT);
        u8BlendStateRebindRTcount = std::max(u8BlendStateRebindRTcount, (uint8) (iRT+1u));
      }
    }
  
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
    ASSERT_M(u8RenderTargetIndex < kMaxNumRenderTargets, "RenderTarget-index not supported");
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
    for (uint8 i = 0; i < kMaxNumRenderTargets; ++i) {
      setRenderTarget(nullptr, i);
    }
  }
//-----------------------------------------------------------------------//
  void RendererGL4::setReadTexture(const Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8Index )
  {
    ASSERT_M(u8Index < kMaxNumReadTextures, "Referenced an undefined texture unit");
  
    if(m_pCachedReadTextures[static_cast<uint>(eShaderStage)][u8Index] == pTexture) {
      return;
    }

    m_pCachedReadTextures[static_cast<uint>(eShaderStage)][u8Index] = pTexture;
    m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::READ_TEXTURES);

    m_uNumReadTexturesToBind[static_cast<uint>(eShaderStage)] = 
      glm::max(m_uNumReadTexturesToBind[static_cast<uint>(eShaderStage)], u8Index + 1u);
    m_uReadTextureBindMask[static_cast<uint>(eShaderStage)] |= (1u<<u8Index);
  }
//-----------------------------------------------------------------------//
  void RendererGL4::setWriteTexture(const Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8Index )
  {
    ASSERT_M(u8Index < kMaxNumWriteTextures, "Referenced an undefined image unit");

    if(m_pCachedWriteTextures[static_cast<uint>(eShaderStage)][u8Index] == pTexture) {
      return;
    }

    m_pCachedWriteTextures[static_cast<uint>(eShaderStage)][u8Index] = pTexture;
    m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::WRITE_TEXTURES);

    m_uNumWriteTexturesToBind[static_cast<uint>(eShaderStage)] = 
      glm::max(m_uNumWriteTexturesToBind[static_cast<uint>(eShaderStage)], u8Index + 1u);
    m_uWriteTextureBindMask[static_cast<uint>(eShaderStage)] |= (1u<<u8Index);
  }
//-----------------------------------------------------------------------//
  void RendererGL4::setReadBuffer(const GpuBuffer* pBuffer, const ShaderStage eShaderStage, const uint8 u8Index )
  {
    ASSERT_M(u8Index < kMaxNumReadBuffers, "Referenced an undefined buffer register");
  
    if(m_pCachedReadBuffers[static_cast<uint>(eShaderStage)][u8Index] == pBuffer) {
      return;
    }

    m_pCachedReadBuffers[static_cast<uint>(eShaderStage)][u8Index] = pBuffer;
    m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::READ_BUFFERS);

    m_uNumReadBuffersToBind[static_cast<uint>(eShaderStage)] = 
      glm::max(m_uNumReadBuffersToBind[static_cast<uint>(eShaderStage)], u8Index + 1u);
    m_uReadBufferBindMask[static_cast<uint>(eShaderStage)] |= (1u<<u8Index);
  }
//-----------------------------------------------------------------------//
  void RendererGL4::setConstantBuffer(const GpuBuffer* pConstantBuffer, const ShaderStage eShaderStage, const uint8 u8Index)
  {
    ASSERT_M(u8Index < (uint32) ConstantBufferType::NUM, "Referenced an undefined constant buffer register");
  
    if(m_pCachedConstantBuffers[static_cast<uint>(eShaderStage)][u8Index] == pConstantBuffer) {
      return;
    }

    m_pCachedConstantBuffers[static_cast<uint>(eShaderStage)][u8Index] = pConstantBuffer;
    m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::CONSTANT_BUFFERS);

    m_uNumConstantBuffersToBind[static_cast<uint>(eShaderStage)] = 
      glm::max(m_uNumConstantBuffersToBind[static_cast<uint>(eShaderStage)], u8Index + 1u);
    m_uConstantBufferBindMask[static_cast<uint>(eShaderStage)] |= (1u<<u8Index);
  }
//-----------------------------------------------------------------------//
  void RendererGL4::setTextureSampler(const TextureSampler* pSampler, const ShaderStage eShaderStage, const uint8 u8Index )
  {
    ASSERT_M(u8Index < kMaxNumTextureSamplers, "Referenced an undefined sampler register");
  
    if(m_pCachedTextureSamplers[static_cast<uint>(eShaderStage)][u8Index] == pSampler) {
      return;
    }

    m_pCachedTextureSamplers[static_cast<uint>(eShaderStage)][u8Index] = pSampler;
    m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::TEXTURE_SAMPLERS);

    m_uNumTextureSamplersToBind[static_cast<uint>(eShaderStage)] = 
      glm::max(m_uNumTextureSamplersToBind[static_cast<uint>(eShaderStage)], u8Index + 1u);
    m_uTextureSamplerBindMask[static_cast<uint>(eShaderStage)] |= (1u<<u8Index);
  }
//-----------------------------------------------------------------------//
  void RendererGL4::setGpuProgram(const GpuProgram* pProgram, const ShaderStage eShaderStage )
  {
    if(m_pBoundGPUPrograms[static_cast<uint>(eShaderStage)] == pProgram) {
      return;
    }

    m_pBoundGPUPrograms[static_cast<uint>(eShaderStage)] = pProgram;
    m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::GPU_PROGRAMS);

    // For now we re-set all resources if a shader has changed.
    // TODO: Avoid this in the future. Try to hide individual setGpuProgram() calls in a 
    // monolithic call to the renderer API (e.g. by wrapping it in a material-object) 
    // that is the only public thing to set in the renderer interface.
    if (pProgram)
    {
      invalidateResourceCache((uint32)eShaderStage);
    }
  }
//---------------------------------------------------------------------------//
  void RendererGL4::invalidateResourceCache(const uint32 uShaderStageIdx)
  {
    m_uResourceRebindMask[uShaderStageIdx] = static_cast<uint>(ResourceRebindFlags::ALL);

    // Reset this stuff during material change?
    // Determine the needed rebind-counts
    // m_uNumReadTexturesToBind[uShaderStageIdx] = kMaxNumReadTextures;
    // for(uint32 i = kMaxNumReadTextures - 1u; i >= 0u; --i)
    // {
    //   if (m_pCachedReadTextures[uShaderStageIdx][i] != nullptr)
    //   {
    //     m_uNumReadTexturesToBind[]
    //   }
    // }
  }
//---------------------------------------------------------------------------//
  void RendererGL4::bindStatesToPipeline()
  {
    if (m_uPipelineRebindMask == 0) {
      return;
    }

    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::WINDINGORDER)) > 0u) {
      glFrontFace(Adapter::toGLType(m_eWindingOrder));
    }

    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::CULLMODE)) > 0u) {
      if (m_eCullMode == CullMode::NONE) {
        GL_SET_CAP(GL_CULL_FACE, false);  // Disables culling in general
      } else {
        glCullFace(Adapter::toGLType(m_eCullMode));
      }
    }

    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::FILLMODE)) > 0u) {
      glPolygonMode(GL_FRONT_AND_BACK, Adapter::toGLType(m_eFillMode));
    }

    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL)) > 0u) {
      bindDepthStencilState();
    }

    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::BLENDING)) > 0u) {
      bindBlendState();
    }

    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::RENDERTARGETS)) > 0u) {
      bindRenderTargets();
    }

    m_uPipelineRebindMask = 0;
  }
//-----------------------------------------------------------------------//
  void RendererGL4::bindResourcesToPipeline()
  {
    for (uint32 uStageIdx = 0u; uStageIdx < (uint32) ShaderStage::NUM; ++uStageIdx)
    {
      const GpuProgram* pGpuProgram = m_pBoundGPUPrograms[uStageIdx];
      const uint32 uResourceRebindMask = m_uResourceRebindMask[uStageIdx];
      m_uResourceRebindMask[uStageIdx] = 0u;
      
      if (!pGpuProgram || uResourceRebindMask == 0u)
      {
        continue;
      }

      // Constant buffers
      if (uResourceRebindMask & (uint32) ResourceRebindFlags::CONSTANT_BUFFERS)
      {
        const GpuBuffer** ppConstantBuffersToBind = m_pCachedConstantBuffers[uStageIdx];
        const uint32 uConstantBufferBindMask = m_uConstantBufferBindMask[uStageIdx];
        m_uConstantBufferBindMask[uStageIdx] = 0u;

        const uint32 uNumConstantBuffersToBind = m_uNumConstantBuffersToBind[uStageIdx];
        m_uNumConstantBuffersToBind[uStageIdx] = 0u;

        for (uint32 i = 0u; i < uNumConstantBuffersToBind; ++i)
        {
          if ((uConstantBufferBindMask & (1 << i)) > 0)
          {
            const GpuBuffer* pConstantBuffer = ppConstantBuffersToBind[i];
            const GLuint uGLhandle = pConstantBuffer ? pConstantBuffer->getGLhandle() : 0u;

            glBindBufferBase(GL_UNIFORM_BUFFER, i, uGLhandle);
          }
        }
      }

      // Read textures
      if (uResourceRebindMask & (uint32) ResourceRebindFlags::READ_TEXTURES)
      {
        const Texture** ppTexturesToBind = m_pCachedReadTextures[uStageIdx];
        const uint32 uReadTextureBindMask = m_uReadTextureBindMask[uStageIdx];
        m_uReadTextureBindMask[uStageIdx] = 0u;

        const uint32 uNumReadTexturesToBind = m_uNumReadTexturesToBind[uStageIdx];
        m_uNumReadTexturesToBind[uStageIdx] = 0u;
        for (uint32 i = 0u; i < uNumReadTexturesToBind; ++i)
        {
          if ((uReadTextureBindMask & (1 << i)) > 0)
          {
            const Texture* pTexture = ppTexturesToBind[i];
            const GLuint uGlhandle = pTexture ? pTexture->getGLhandle() : 0u;

            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(pTexture ? pTexture->getTextureTypeGL() : GL_TEXTURE_2D, uGlhandle);
          }
        }
      }

      // Sampler objects
      if (uResourceRebindMask & (uint32) ResourceRebindFlags::TEXTURE_SAMPLERS)
      {
        const TextureSampler** ppSamplersToBind = m_pCachedTextureSamplers[uStageIdx];
        const uint32 uSamplerBindMask = m_uTextureSamplerBindMask[uStageIdx];
        m_uTextureSamplerBindMask[uStageIdx] = 0u;

        const uint32 uNumSamplersToBind = m_uNumTextureSamplersToBind[uStageIdx];
        m_uNumTextureSamplersToBind[uStageIdx] = 0u;
        for (uint32 i = 0u; i < uNumSamplersToBind; ++i)
        {
          if ((uSamplerBindMask & (1 << i)) > 0)
          {
            const TextureSampler* pSampler = ppSamplersToBind[i];
            const GLuint uGlhandle = pSampler ? pSampler->getGLhandle() : 0u;

            glBindSampler(i, uGlhandle);  
          }
        }
      }

      // Write textures
      if (uResourceRebindMask & (uint32) ResourceRebindFlags::WRITE_TEXTURES)
      {
        const Texture** ppTexturesToBind = m_pCachedWriteTextures[uStageIdx];
        const uint32 uWriteTextureBindMask = m_uWriteTextureBindMask[uStageIdx];
        m_uWriteTextureBindMask[uStageIdx] = 0u;

        const uint uNumWriteTexturesTobind = m_uNumWriteTexturesToBind[uStageIdx];
        m_uNumWriteTexturesToBind[uStageIdx] = 0u;
        for (uint32 i = 0u; i < uNumWriteTexturesTobind; ++i)
        {
          if ((uWriteTextureBindMask & (1 << i)) > 0)
          {
            const Texture* pTexture = ppTexturesToBind[i];
            const GLuint uGlhandle = pTexture ? pTexture->getGLhandle() : 0u;
            const GLenum eGLinternalFormat = pTexture ? pTexture->getInternalFormatGL() : GL_RGBA8;

            glBindImageTexture(i, uGlhandle, 0, GL_FALSE, 0, GL_READ_WRITE, eGLinternalFormat);
          }
        }
      }

      // Read buffers
      if (uResourceRebindMask & (uint32) ResourceRebindFlags::READ_BUFFERS)
      {
        // TODO: Have to be handled as either ShaderStorage buffers or BufferTextures internally... 
      }

      // Write buffers
      if (uResourceRebindMask & (uint32) ResourceRebindFlags::WRITE_BUFFERS)
      {
        // TODO: Have to be handled as either ShaderStorage buffers or BufferTextures internally... 
      }
    }
  }
//---------------------------------------------------------------------------//
  void RendererGL4::applyViewport()
  {
    if (m_bViewportDirty)
    {
      glViewport(m_uViewportParams.x, m_uViewportParams.y,
        m_uViewportParams.z, m_uViewportParams.w);

      m_bViewportDirty = false;
    }
  }
//---------------------------------------------------------------------------//
  void RendererGL4::bindBlendState()
  {
    ASSERT_M(m_uBlendStateRebindMask > 0, 
      "pipelineRebindMask not in sync with blendStateRebindMask");

    uint32 uBlendStateRebindMask = m_uBlendStateRebindMask;
    bool bBlendingPerRTenabled = m_clBlendState.myBlendStatePerRT;

    if ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::ALPHA_TO_COVERAGE)) > 0)   {
       GL_SET_CAP(GL_SAMPLE_ALPHA_TO_COVERAGE, m_clBlendState.myAlphaToCoverageEnabled);
    }

    if ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLENDSTATE_PER_RT)) > 0)  {
      // Noting to do. Just use glBlendFuncSeperatei() and related functions if enabled
    }

    if (m_clBlendState.myBlendStatePerRT) {
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
      GL_SET_CAP(GL_BLEND, m_clBlendState.myBlendEnabled[0]);
    }

    const bool bSeperateBlend = m_clBlendState.myAlphaSeparateBlend[0];

    const bool bApplyBlendFunc = 
      (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::SRC_BLEND)) > 0 ||
      (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::DEST_BLEND)) > 0 ||
      (bSeperateBlend &&
        ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::SRC_BLEND_ALPHA)) > 0 ||
         (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::DEST_BLEND_ALPHA)) > 0));

    const bool bApplyBlendOp =
      (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLEND_OP)) > 0 ||
      (bSeperateBlend &&
        ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLEND_OP_ALPHA)) > 0));

    if(bApplyBlendFunc) 
    {
      if (!bSeperateBlend) {
        glBlendFunc(Adapter::toGLType(m_clBlendState.mySrcBlend[0]), Adapter::toGLType(m_clBlendState.myDestBlend[0]));
      } else {
        glBlendFuncSeparate(Adapter::toGLType(m_clBlendState.mySrcBlend[0]), Adapter::toGLType(m_clBlendState.myDestBlend[0]),
                            Adapter::toGLType(m_clBlendState.mySrcBlendAlpha[0]), Adapter::toGLType(m_clBlendState.myDestBlendAlpha[0]));
      }
    }

    if (bApplyBlendOp)
    {
      if (!bSeperateBlend) {
        glBlendEquation(Adapter::toGLType(m_clBlendState.myBlendOp[0]));
      } else {
        glBlendEquationSeparate(Adapter::toGLType(m_clBlendState.myBlendOp[0]), Adapter::toGLType(m_clBlendState.myBlendOpAlpha[0]));
      }
    }
  
    if ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::RT_WRITE_MASK)) > 0) {
       
      const bool red =    (m_clBlendState.myRTwriteMask[0] & 0xFF000000) > 0u;
      const bool green =  (m_clBlendState.myRTwriteMask[0] & 0x00FF0000) > 0u;
      const bool blue =   (m_clBlendState.myRTwriteMask[0] & 0x0000FF00) > 0u;
      const bool alpha =  (m_clBlendState.myRTwriteMask[0] & 0x000000FF) > 0u;

      glColorMask(red, green, blue, alpha);
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
        GL_SET_CAPi(GL_BEVEL_NV, iRT, m_clBlendState.myBlendEnabled[iRT]);
      }

      const bool bSeperateBlend = m_clBlendState.myAlphaSeparateBlend[iRT];

      const bool bApplyBlendFunc = 
        (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::SRC_BLEND)) > 0 ||
        (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::DEST_BLEND)) > 0 ||
        (bSeperateBlend &&
        ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::SRC_BLEND_ALPHA)) > 0 ||
        (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::DEST_BLEND_ALPHA)) > 0));

      const bool bApplyBlendOp =
        (uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLEND_OP)) > 0 ||
        (bSeperateBlend &&
        ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::BLEND_OP_ALPHA)) > 0));

      if(bApplyBlendFunc) 
      {
        if (!bSeperateBlend) {
          glBlendFunci(iRT, Adapter::toGLType(m_clBlendState.mySrcBlend[iRT]), Adapter::toGLType(m_clBlendState.myDestBlend[iRT]));
        } else {
          glBlendFuncSeparatei(iRT, Adapter::toGLType(m_clBlendState.mySrcBlend[iRT]), Adapter::toGLType(m_clBlendState.myDestBlend[iRT]),
            Adapter::toGLType(m_clBlendState.mySrcBlendAlpha[iRT]), Adapter::toGLType(m_clBlendState.myDestBlendAlpha[iRT]));
        }
      }

      if (bApplyBlendOp)
      {
        if (!bSeperateBlend) {
          glBlendEquation(Adapter::toGLType(m_clBlendState.myBlendOp[iRT]));
        } else {
          glBlendEquationSeparate(Adapter::toGLType(m_clBlendState.myBlendOp[iRT]), Adapter::toGLType(m_clBlendState.myBlendOpAlpha[iRT]));
        }
      }

      if ((uBlendStateRebindMask & static_cast<uint>(BlendStateRebindFlags::RT_WRITE_MASK)) > 0) {
        glColorMaski( iRT,
          (m_clBlendState.myRTwriteMask[iRT] & 0xFF00000000) > 0,
          (m_clBlendState.myRTwriteMask[iRT] & 0x00FF000000) > 0,
          (m_clBlendState.myRTwriteMask[iRT] & 0x0000FF0000) > 0,
          (m_clBlendState.myRTwriteMask[iRT] & 0x000000FF00) > 0);
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

      GL_SET_CAP(GL_DEPTH_TEST, m_clDepthStencilState.m_bDepthTestEnabled);
    }

    if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::DEPTH_WRITE) > 0 ) {
      glDepthMask(m_clDepthStencilState.m_bDepthWriteEnabled);
    }

    if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::DEPTH_COMP_FUNC) > 0) {
      glDepthFunc(Adapter::toGLType(m_clDepthStencilState.m_eDepthCompFunc));
    }

    if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_ENABLED) > 0) {
      GL_SET_CAP(GL_STENCIL_TEST, m_clDepthStencilState.m_bStencilEnabled);
    }

    if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_TWO_SIDED) > 0) {
      // Nothing special to set here - just use the "separate" calls for stencil-ops and funcs
    }
    const bool bUseTwoSidedStencil = m_clDepthStencilState.m_bTwoSidedStencil;


    if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_REF) > 0) {
      GL_SET_CAP(GL_STENCIL_TEST, m_clDepthStencilState.m_bStencilEnabled);
    }

    const bool bNeedStencilFunc =
         (uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_REF) > 0
      || (uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_READ_MASK) > 0
      || (uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_COMP_FUNC) > 0;

    if (bNeedStencilFunc && !bUseTwoSidedStencil) 
    {
      glStencilFunc(Adapter::toGLType(m_clDepthStencilState.m_eStencilCompFunc[0]),
                    m_clDepthStencilState.m_iStencilRef,
                    m_clDepthStencilState.m_uStencilReadMask);
    }
    else if (bNeedStencilFunc)
    {
      glStencilFuncSeparate(Adapter::toGLType(m_clDepthStencilState.m_eStencilCompFunc[(uint)FaceType::FRONT]),
                            Adapter::toGLType(m_clDepthStencilState.m_eStencilCompFunc[(uint)FaceType::BACK]),
                            m_clDepthStencilState.m_iStencilRef,
                            m_clDepthStencilState.m_uStencilReadMask);
    }

    if ((uDepthStencilRebindMask & (uint) DepthStencilRebindFlags::STENCIL_WRITE_MASK ) > 0 ) {
      if(!bUseTwoSidedStencil) {
        glStencilMask(m_clDepthStencilState.m_uStencilWriteMask[0]);
      } else {
        glStencilMaskSeparate(GL_FRONT, m_clDepthStencilState.m_uStencilWriteMask[(uint) FaceType::FRONT]);
        glStencilMaskSeparate(GL_BACK, m_clDepthStencilState.m_uStencilWriteMask[(uint) FaceType::BACK]);
      }
    }
  
    m_uDepthStencilRebindMask = 0;
  }
  //-----------------------------------------------------------------------//
  void RendererGL4::bindRenderTargets()
  {
    // The actual (packed) list of renderTextures to use
    Texture* rtListPatched[kMaxNumRenderTargets] = {nullptr};
    uint8 u8RenderTextureCount = 0;

    // Detect "holes" in renderTexture-array
    for(uint8 i = 0; i < kMaxNumRenderTargets; ++i) 
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
          && i < kMaxNumRenderTargets) 
        {
          ++i;
        }
      }
    }
  
    const GLuint uFBOtoUse = 
      createOrRetrieveFBO(rtListPatched, u8RenderTextureCount, m_pCachedDepthStencilTarget);

    if (uFBOtoUse == m_uCurrentFBO) {
      return;
    }
    m_uCurrentFBO = uFBOtoUse;
    glBindFramebuffer(GL_FRAMEBUFFER, uFBOtoUse);

    // Default: Enable all drawbuffers. 
    // TODO: Better to determine drawbuffers from blendState::RTwritemask?
    static GLenum drawBuffers[kMaxNumRenderTargets] = 
    {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, 
    GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6};
    glDrawBuffers(u8RenderTextureCount, drawBuffers);
  }
//-----------------------------------------------------------------------//
  GLuint RendererGL4::createOrRetrieveFBO(Texture** pRenderTextures, uint8 u8RenderTextureCount, Texture* pDStexture)
  {
    uint hash = 0;
    for (uint8 i = 0; i < u8RenderTextureCount; ++i) {
      MathUtil::hash_combine(hash, reinterpret_cast<uint>(*pRenderTextures));
    }
    MathUtil::hash_combine(hash, reinterpret_cast<uint>(pDStexture));

    for (uint8 i = 0; i < _countof(m_FBOpool); ++i) 
    {
      if (m_FBOpool[i].hash == hash) {
        return m_FBOpool[i].glHandle;
      }
    }

    // No suitable FBO found in the pool: create one in a lazy fashion...
    // find the first unused FBO in the pool
    RendererGL4::GpuCacheEntry* pFBOentry = nullptr;
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
      glFramebufferTexture2D(GL_FRAMEBUFFER, Internal::getColorAttachmentFromIndex(i), 
        GL_TEXTURE_2D, pRenderTextures[i]->getGLhandle(), 0);
    }
    if (pDStexture)
    {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
        GL_TEXTURE_2D, pDStexture->getGLhandle(), 0);
    }
  #if defined (FANCY_RENDERSYSTEM_USE_VALIDATION)
    GLDebug::validateFBOcompleteness();
  #endif // FANCY_RENDERSYSTEM_USE_VALIDATION
    glBindFramebuffer(GL_FRAMEBUFFER, m_uCurrentFBO);  // Is this necessary?
    
    return pFBOentry->glHandle;
    // TODO: Add support for 1D, 3D, cubemap RenderTargets
  }
//---------------------------------------------------------------------------//
  GLuint RendererGL4::createOrRetrieveProgramPipeline()
  {
    uint hash = 0;
    for (uint32 i = 0u; i < _countof(m_pBoundGPUPrograms); ++i)
    {
      MathUtil::hash_combine(hash, reinterpret_cast<uint>(m_pBoundGPUPrograms[i]));
    }

    for (uint32 i = 0u; i < _countof(m_GpuProgramPipelinePool); ++i)
    {
      if (m_GpuProgramPipelinePool[i].hash == hash)
      {
        return m_GpuProgramPipelinePool[i].glHandle;
      }
    }

    // No suitable program pipeline found. Create one layzily...
    RendererGL4::GpuCacheEntry* pCacheEntry = nullptr;
    for (uint32 i = 0u; i < _countof(m_GpuProgramPipelinePool); ++i)
    {
      if (m_GpuProgramPipelinePool[i].glHandle == GLUINT_HANDLE_INVALID)
      {
        pCacheEntry = &m_GpuProgramPipelinePool[i];
      }
    }

    ASSERT_M(pCacheEntry != nullptr, "No free program pipeline object found in the pool");

    uint32 uPipeline;
    glGenProgramPipelines(1, &uPipeline);
    for (uint32 i = 0u; i < _countof(m_pBoundGPUPrograms); ++i)
    {
      const GpuProgram* pProgram = m_pBoundGPUPrograms[i];
      if (pProgram)
      {
        GLuint shaderStageBit = Internal::getGLShaderStageBit(pProgram->getShaderStage());
        glUseProgramStages(uPipeline, shaderStageBit, pProgram->getProgramHandle());  
      }
    }

    pCacheEntry->hash = hash;
    pCacheEntry->glHandle = uPipeline;
    
    return uPipeline;
  }
//---------------------------------------------------------------------------//
  const RendererGL4::VaoCacheEntry& 
    RendererGL4::createOrRetrieveVAO(const GeometryVertexLayout* pGeoVertexLayout, 
                                     const VertexInputLayout* pVertexInputLayout)
  {
    // The fast path: Calc the hash of both layouts and look for an existing VAO to return
    uint uHash = 0;
    MathUtil::hash_combine(uHash, reinterpret_cast<uint>(pGeoVertexLayout));
    MathUtil::hash_combine(uHash, reinterpret_cast<uint>(pVertexInputLayout));

    VaoCacheEntry* pCacheEntry = nullptr;
    for (uint32 i = 0u; i < _countof(m_VAOpool); ++i)
    {
      if (m_VAOpool[i].hash == uHash) 
      {
        return m_VAOpool[i];
      }
      else if (m_VAOpool[i].glHandle == GLUINT_HANDLE_INVALID)
      {
        pCacheEntry = &m_VAOpool[i];
        break;
      }
    }

    // No suitable cached VAO found in the pool. Create one and establish the vertex stream

    ASSERT_M(pCacheEntry != nullptr, "No free VAO found in the pool");

    // Create a mapping between the geometric vertices and the shader attributes
    struct MappingEntry 
    {
      const VertexInputElement* pInputElement;
      const GeometryVertexElement* pGeomElement;
    };

    struct PatchingEntry
    {
      const VertexInputElement* pInputElement;
    };

    FixedArray<MappingEntry, kMaxNumInputVertexAttributes> vMappingEntries;
    FixedArray<PatchingEntry, kMaxNumInputVertexAttributes> vPatchingEntries;

    for (uint32 iInputElem = 0u; iInputElem < pVertexInputLayout->getNumVertexInputElements(); ++iInputElem)
    {
      const VertexInputElement& rInputElement = pVertexInputLayout->getVertexInputElement(iInputElem);
      MappingEntry mappingEntry = {nullptr};

      // Look for the corresponding semantic in the geometry vertices
      for (uint32 iGeomElem = 0u; iGeomElem < pGeoVertexLayout->getNumVertexElements(); ++iGeomElem)
      {
        const GeometryVertexElement& rGeomElement = pGeoVertexLayout->getVertexElement(iGeomElem);
        if (rInputElement.eSemantics == rGeomElement.eSemantics) 
        {
          mappingEntry.pInputElement = &rInputElement;
          mappingEntry.pGeomElement = &rGeomElement;
          break;
        }
      }

      if (mappingEntry.pInputElement)
      {
        // We found a mapping
        vMappingEntries.push_back(mappingEntry);
      }
      else
      {
        // No geometry-stream available for this attribute. We need to patch it
        PatchingEntry patchingEntry;
        patchingEntry.pInputElement = &rInputElement;
        vPatchingEntries.push_back(patchingEntry);
      }
    }

    // Create a new vertex layout from the given geometry and shader layouts
    GLuint uVAO;
    glGenVertexArrays(1, &uVAO);
    bindVAO(uVAO);

    for (uint32 i = 0u; i < kMaxNumInputVertexAttributes; ++i)
    {
      glDisableVertexAttribArray(i);
    }
    
    uint32 uStreamMask = 0u;
    if (vMappingEntries.size() > 0u)
    {
      // We have at least one matching attribute stream
      uStreamMask |= (uint32) VertexBufferBindingFlags::GEOMETRY_STREAM;
    }
    if (vMappingEntries.size() < pVertexInputLayout->getNumVertexInputElements())
    {
      // We need to patch some attributes
      uStreamMask |= (uint32) VertexBufferBindingFlags::PATCHING_STREAM_VEC4;
    }

    // TODO: The patching stream is currently not in use... do we even have to use it in desktop-opengl?
    // Currently, I believe a default value of (0,0,0,1) is fed to the vertex assembly if a shader-attribute is not backed by a geo-stream

    // Set up the vertex format for each mapped attribute
    for (uint32 i = 0u; i < vMappingEntries.size(); ++i)
    {
      const VertexInputElement* pInputElement = vMappingEntries[i].pInputElement;
      const GeometryVertexElement* pGeomElement = vMappingEntries[i].pGeomElement;

      uint32 uNumComponentsGeo = 0u;
      GLenum eDataTypeGeoGL = GL_FLOAT;
      Adapter::getGLtypeAndNumComponentsFromFormat(pGeomElement->eFormat, uNumComponentsGeo, eDataTypeGeoGL);

      glEnableVertexAttribArray(pInputElement->u32RegisterIndex);
      
      glVertexAttribFormat(pInputElement->u32RegisterIndex, uNumComponentsGeo, 
        eDataTypeGeoGL, GL_FALSE, pGeomElement->u32OffsetBytes);

      // TODO: Support mutliple streams (e.g. instancing) at this point...
      glVertexAttribBinding(pInputElement->u32RegisterIndex, (uint32)VertexBufferBindingPoints::GEOMETRY_STREAM);
    }
    
    pCacheEntry->glHandle = uVAO;
    pCacheEntry->hash = uHash;
    pCacheEntry->uStreamMask = uStreamMask;

    return *pCacheEntry;
  }
//---------------------------------------------------------------------------//
  void RendererGL4::beginFrame()
  {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
//---------------------------------------------------------------------------//
  void RendererGL4::endFrame()
  {

  }
//---------------------------------------------------------------------------//
  void RendererGL4::renderGeometry(const Geometry::GeometryData* pGeometry)
  {
    // First create or retrieve a programPipeline object and bind it. 
    const GLuint uProgPipeline = createOrRetrieveProgramPipeline();
    if (m_uCurrentGpuProgramPipeline != uProgPipeline)
    {
      glBindProgramPipeline(uProgPipeline);
      m_uCurrentGpuProgramPipeline = uProgPipeline;
    }
            
    applyViewport();

    // Bind resources to the pipeline
    bindStatesToPipeline();
    bindResourcesToPipeline();
    
    const GLuint uVBOtoUse = pGeometry->getVertexBuffer()->getGLhandle();
    const GLuint uIBOtoUse = pGeometry->getIndexBuffer()->getGLhandle();

    const GeometryVertexLayout* vertLayoutGeo = &pGeometry->getGeometryVertexLayout();
    const VertexInputLayout* vertLayoutShader = 
      m_pBoundGPUPrograms[(uint32)ShaderStage::VERTEX]->getVertexInputLayout();

    const VaoCacheEntry uVaoEntry = 
      createOrRetrieveVAO(vertLayoutGeo, vertLayoutShader);

    bindVAO(uVaoEntry.glHandle);
    
    // Bind the stream-buffers
    if (uVaoEntry.uStreamMask & (uint32)VertexBufferBindingFlags::GEOMETRY_STREAM)
    {
      if (m_uCurrentVBO != uVBOtoUse)
      {
        glBindVertexBuffer((uint32) VertexBufferBindingPoints::GEOMETRY_STREAM, uVBOtoUse, 0, vertLayoutGeo->getStrideBytes());
        m_uCurrentVBO = uVBOtoUse;
      }
    }
    // TODO: Add support for per-instance attributes here...
    // if (uVaoEntry.uStreamMask & (uint32)VertexBufferBindingFlags::INSTANCE_STREAM_1)
    // {
    //   
    // }

    bindIBO(uIBOtoUse);
    
    // TODO: Set up instanced, indirect rendering etc.
    glDrawElements(GL_TRIANGLES, pGeometry->getNumIndices(), GL_UNSIGNED_INT, nullptr);
  }
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::GL4

#endif
