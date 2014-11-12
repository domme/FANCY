#include "TextureSamplerGL4.h"
#include "AdapterGL4.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  namespace internal {
    GLuint getGLfilteringType(SamplerFilterMode eFilterMode, bool bUseMipmaps);
    GLuint getGLaddressMode(SamplerAddressMode eAddressMode);
  }
//---------------------------------------------------------------------------//
  GLuint internal::getGLfilteringType(SamplerFilterMode eFilterMode, bool bUseMipmaps)
  {
    switch (eFilterMode)
    {
      case Fancy::Rendering::SamplerFilterMode::NEAREST:
        return bUseMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
      case Fancy::Rendering::SamplerFilterMode::BILINEAR:
        return bUseMipmaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
      case Fancy::Rendering::SamplerFilterMode::TRILINEAR:
      case Fancy::Rendering::SamplerFilterMode::ANISOTROPIC:
        return GL_LINEAR_MIPMAP_LINEAR;
      default:
        ASSERT_M(false, "Invalid filtering mode");
        break;
      }

    return GL_NEAREST;
  }
//---------------------------------------------------------------------------//
  GLuint internal::getGLaddressMode(SamplerAddressMode eAddressMode)
  {
    switch (eAddressMode)
    {
      case Fancy::Rendering::SamplerAddressMode::WRAP:
        return GL_WRAP_BORDER;
      case Fancy::Rendering::SamplerAddressMode::CLAMP_EDGE:
        return GL_CLAMP_TO_EDGE;
      case Fancy::Rendering::SamplerAddressMode::MIRROR_CLAMP_EDGE:
        return GL_MIRROR_CLAMP_TO_EDGE;
      case Fancy::Rendering::SamplerAddressMode::CLAMP_BORDER:
        return GL_CLAMP_TO_BORDER;
      case Fancy::Rendering::SamplerAddressMode::REPEAT:
        return GL_REPEAT;
      case Fancy::Rendering::SamplerAddressMode::MIRROR_REPEAT:
        return GL_MIRRORED_REPEAT;
      default:
        ASSERT_M(false, "Invalid address mode");
        break;
    }

    return GL_REPEAT;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  TextureSamplerGL4::TextureSamplerGL4() :
    m_uHandleGL(GLUINT_HANDLE_INVALID)
  {

  }
//---------------------------------------------------------------------------//
  TextureSamplerGL4::~TextureSamplerGL4()
  {

  }
//---------------------------------------------------------------------------//
  void TextureSamplerGL4::create( const ObjectName& rName, 
    const TextureSamplerProperties& rProperties )
  {
    destroy();

    const bool useMipmaps = rProperties.fMaxLod > 0.0f;

#if defined (FANCY_RENDERSYSTEM_USE_VALIDATION)
    // Some sanity-checks
    if (rProperties.minFiltering == SamplerFilterMode::ANISOTROPIC
      || rProperties.magFiltering == SamplerFilterMode::ANISOTROPIC)
    {
      ASSERT_M(rProperties.fMaxAnisotropy > 1.0f, 
        "Anisotropic filtering requested but maxAnisotropy level is 0");
    }

    if (!useMipmaps)
    {
      ASSERT_M(rProperties.minFiltering <= SamplerFilterMode::BILINEAR 
              && rProperties.magFiltering <= SamplerFilterMode::BILINEAR,
              "No mipmaps available but mipmap-filtering requested");
    }
#endif  // FANCY_RENDERSYSTEM_USE_VALIDATION

    const GLuint uglMinFiltering = internal::getGLfilteringType(rProperties.minFiltering, useMipmaps);
    const GLuint uglMagFiltering = internal::getGLfilteringType(rProperties.magFiltering, useMipmaps);
    const GLuint uglAddressModeX = internal::getGLaddressMode(rProperties.addressModeX);
    const GLuint uglAddressModeY = internal::getGLaddressMode(rProperties.addressModeY);
    const GLuint uglAddressModeZ = internal::getGLaddressMode(rProperties.addressModeZ);
    const GLenum eglComparisonFunc = Adapter::toGLType(rProperties.comparisonFunc);
    const GLenum eglComparisonMode = (eglComparisonFunc == GL_ALWAYS || eglComparisonFunc == GL_NEVER) ?
                                       GL_NONE : GL_COMPARE_R_TO_TEXTURE;
    

    GLuint uSampler;
    glGenSamplers(1u, &uSampler);

    glSamplerParameteri(uSampler, GL_TEXTURE_MIN_FILTER, uglMinFiltering);
    glSamplerParameteri(uSampler, GL_TEXTURE_MAG_FILTER, uglMagFiltering);
    glSamplerParameteri(uSampler, GL_TEXTURE_WRAP_S, uglAddressModeX);
    glSamplerParameteri(uSampler, GL_TEXTURE_WRAP_T, uglAddressModeY);
    glSamplerParameteri(uSampler, GL_TEXTURE_WRAP_R, uglAddressModeZ);
    glSamplerParameteri(uSampler, GL_TEXTURE_COMPARE_FUNC, eglComparisonFunc);
    glSamplerParameteri(uSampler, GL_TEXTURE_COMPARE_MODE, eglComparisonMode);
    glSamplerParameterfv(uSampler, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(rProperties.borderColor));
    glSamplerParameterf(uSampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, rProperties.fMaxAnisotropy);
    glSamplerParameterf(uSampler, GL_TEXTURE_LOD_BIAS, rProperties.fLodBias);
    glSamplerParameterf(uSampler, GL_TEXTURE_MIN_LOD, rProperties.fMinLod);
    glSamplerParameterf(uSampler, GL_TEXTURE_MAX_LOD, rProperties.fMaxLod);

    m_Name = rName;
    m_uHandleGL = uSampler;
    m_properties = rProperties;
  }
//---------------------------------------------------------------------------//
  void TextureSamplerGL4::destroy()
  {
    if (m_uHandleGL != GLUINT_HANDLE_INVALID)
    {
      glDeleteSamplers(1u, &m_uHandleGL);
      m_uHandleGL = GLUINT_HANDLE_INVALID;

      m_properties = TextureSamplerProperties();
    }
  }
} } }  // end of namespace Fancy::Rendering::GL4