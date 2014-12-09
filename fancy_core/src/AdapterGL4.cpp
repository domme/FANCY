#include "AdapterGL4.h"

namespace Fancy { namespace Rendering { namespace GL4 {

    //---------------------------------------------------------------------------//
    GLenum Adapter::mapResourceTypeToGLbindingTarget(const GpuResourceType& generalType) {
      switch (generalType)
      {
      case GpuResourceType::TEXTURE_1D:   return GL_TEXTURE_1D;
      case GpuResourceType::TEXTURE_1D_SHADOW: return GL_TEXTURE_1D;
      case GpuResourceType::TEXTURE_2D:   return GL_TEXTURE_2D;
      case GpuResourceType::TEXTURE_2D_SHADOW : return GL_TEXTURE_2D;
      case GpuResourceType::TEXTURE_3D: return GL_TEXTURE_3D;
      case GpuResourceType::TEXTURE_CUBE: return GL_TEXTURE_CUBE_MAP;
      case GpuResourceType::TEXTURE_CUBE_SHADOW: return GL_TEXTURE_CUBE_MAP;
      case GpuResourceType::BUFFER_TEXTURE: return GL_TEXTURE_BUFFER;
      case GpuResourceType::BUFFER: return GL_SHADER_STORAGE_BUFFER;
      default:
        ASSERT_M(false, "Resource type has no binding target");
        return GL_TEXTURE_2D;
        break;
      }
    }
    // Adapter-methods to translate between the state-enums
    //-----------------------------------------------------------------------//
    // Comp Func
    GLenum Adapter::toGLType(const CompFunc& generalType) {
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
    //---------------------------------------------------------------------------//
    GLenum Adapter::toGLType(const StencilOp& generalType) {
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
    //---------------------------------------------------------------------------//
    GLenum Adapter::toGLType(const FillMode& generalType) {
      switch (generalType) {
      case FillMode::WIREFRAME: return GL_LINE;
      case FillMode::SOLID:     return GL_FILL;
      default: ASSERT_M(false, "Missing GL values"); return GL_FILL;
      }
    }
    //---------------------------------------------------------------------------//
    GLenum Adapter::toGLType(const CullMode& generalType) {
      switch (generalType) {
      case CullMode::NONE:  return GL_NONE; // Special type. Requires call to glDisable(CULLING)
      case CullMode::FRONT: return GL_FRONT;
      case CullMode::BACK:  return GL_BACK;
      default: ASSERT_M(false, "Missing GL values"); return GL_BACK;
      }
    }
    //---------------------------------------------------------------------------//
    GLenum Adapter::toGLType(const WindingOrder& generalType) {
      switch (generalType) {
      case WindingOrder::CCW: return GL_CCW;
      case WindingOrder::CW:  return GL_CW;
      default: ASSERT_M(false, "Missing GL values"); return GL_CCW;
      }
    }
    //---------------------------------------------------------------------------//
    GLenum Adapter::toGLType(const BlendInput& generalType) {
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
    //---------------------------------------------------------------------------//
    GLenum Adapter::toGLType(const BlendOp& generalType) {
      switch (generalType) {
      case BlendOp::ADD:            return GL_FUNC_ADD;
      case BlendOp::SUBTRACT:       return GL_FUNC_SUBTRACT;
      case BlendOp::REV_SUBTRACT:   return GL_FUNC_REVERSE_SUBTRACT;
      case BlendOp::MIN:            return GL_MIN;
      case BlendOp::MAX:            return GL_MAX;
      default: ASSERT_M(false, "Missing GL values"); return GL_FUNC_ADD;
      }
    }
    //---------------------------------------------------------------------------//
    GLenum Adapter::toGLType(const ShaderStage& generalType) {
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
    //---------------------------------------------------------------------------//
    GLuint Adapter::toGLFlag(const ShaderStageFlag& generalType) {
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
    //---------------------------------------------------------------------------//
    /// Returns the internal format (prior to any modifications)
    GLenum Adapter::toGLType(const DataFormat& generalType) 
    {
      switch (generalType)
      {
      case DataFormat::SRGB_8_A_8:     return GL_SRGB8_ALPHA8;
      case DataFormat::RGBA_8:         return GL_RGBA8;
      case DataFormat::SRGB_8:         return GL_SRGB8;
      case DataFormat::RGB_8:          return GL_RGB8;
      case DataFormat::RGB_11_11_10F:  return GL_R11F_G11F_B10F;
      case DataFormat::RGBA_16F:       return GL_RGBA16F;
      case DataFormat::RGB_16F:        return GL_RGB16F;
      case DataFormat::RG_16F:         return GL_RG16F;
      case DataFormat::R_16F:          return GL_R16F;
      case DataFormat::RGBA_32F:       return GL_RGBA32F;
      case DataFormat::RGB_32F:        return GL_RGB32F;
      case DataFormat::RG_32F:         return GL_RG32F;
      case DataFormat::R_32F:          return GL_R32F;
      case DataFormat::RGBA_32UI:      return GL_RGBA32UI;
      case DataFormat::RGB_32UI:       return GL_RGB32UI;
      case DataFormat::RG_32UI:        return GL_RG32UI;
      case DataFormat::R_32UI:         return GL_R32UI;
      case DataFormat::RGBA_16UI:      return GL_RGBA16UI;
      case DataFormat::RGB_16UI:       return GL_RGB16UI;
      case DataFormat::RG_16UI:        return GL_RG16UI;
      case DataFormat::R_16UI:         return GL_R16UI;
      case DataFormat::RGBA_8UI:       return GL_RGBA8UI;
      case DataFormat::RGB_8UI:        return GL_RGB8UI;
      case DataFormat::RG_8UI:         return GL_RG8UI;
      case DataFormat::R_8UI:          return GL_R8UI;
      case DataFormat::DS_24_8:        return GL_DEPTH24_STENCIL8;
      default: ASSERT_M(false, "Missing GL values"); return GL_RGBA8;
      }
    }
    //---------------------------------------------------------------------------//
    /// Retrieves matching values for OpenGL format, internalFormat and pixelFormat
    void Adapter::mapGLpixelFormats(const DataFormat& generalPixelFormat, bool isDepthStencilFormat,
      GLenum& eFormat, GLenum& eInternalFormat, GLenum& ePixelType) 
    {
      eInternalFormat = toGLType(generalPixelFormat);

      switch (eInternalFormat) {
      case GL_SRGB8_ALPHA8:
      case GL_RGBA8:          eFormat = GL_RGBA; ePixelType = GL_UNSIGNED_BYTE; break;
      case GL_SRGB8:
      case GL_RGB8:           eFormat = GL_RGB;   ePixelType = GL_UNSIGNED_BYTE;  break;
      case GL_R11F_G11F_B10F: eFormat = GL_RGB;   ePixelType = GL_UNSIGNED_INT_10F_11F_11F_REV; break;
      case GL_RGBA16F:        eFormat = GL_RGBA;  ePixelType = GL_HALF_FLOAT;     break;
      case GL_RGB16F:         eFormat = GL_RGB;   ePixelType = GL_HALF_FLOAT;     break;
      case GL_RG16F:          eFormat = GL_RG;    ePixelType = GL_HALF_FLOAT;     break;
      case GL_R16F:           eFormat = GL_RED;   ePixelType = GL_HALF_FLOAT;     break;
      case GL_RGBA32F:        eFormat = GL_RGBA;  ePixelType = GL_FLOAT;          break;
      case GL_RGB32F:         eFormat = GL_RGB;   ePixelType = GL_FLOAT;          break;
      case GL_RG32F:          eFormat = GL_RG;    ePixelType = GL_FLOAT;          break;
      case GL_R32F:           eFormat = GL_RED;   ePixelType = GL_FLOAT;          break;
      case GL_RGBA32UI:       eFormat = GL_RGBA;  ePixelType = GL_UNSIGNED_INT;   break;
      case GL_RGB32UI:        eFormat = GL_RGB;   ePixelType = GL_UNSIGNED_INT;   break;
      case GL_RG32UI:         eFormat = GL_RG;    ePixelType = GL_UNSIGNED_INT;   break;
      case GL_R32UI:          eFormat = GL_RED;   ePixelType = GL_UNSIGNED_INT;   break;
      case GL_RGBA16UI:       eFormat = GL_RGBA;  ePixelType = GL_UNSIGNED_SHORT; break;
      case GL_RGB16UI:        eFormat = GL_RGB;   ePixelType = GL_UNSIGNED_SHORT; break;
      case GL_RG16UI:         eFormat = GL_RG;    ePixelType = GL_UNSIGNED_SHORT; break;
      case GL_R16UI:          eFormat = GL_RED;   ePixelType = GL_UNSIGNED_SHORT; break;
      case GL_RGBA8UI:        eFormat = GL_RGBA;  ePixelType = GL_UNSIGNED_BYTE;  break;
      case GL_RGB8UI:         eFormat = GL_RGB;   ePixelType = GL_UNSIGNED_BYTE;  break;
      case GL_RG8UI:          eFormat = GL_RG;    ePixelType = GL_UNSIGNED_BYTE;  break;
      case GL_R8UI:           eFormat = GL_RED;   ePixelType = GL_UNSIGNED_BYTE;  break;
      case GL_DEPTH24_STENCIL8: eFormat = GL_DEPTH_STENCIL; ePixelType = GL_UNSIGNED_INT_24_8; break;
      default: ASSERT_M(false, "Missing GL values"); break;
      }

      // Check if a depth-surface is requested without the usual DS_24_8 format
      // In this case, the texture is assumed to be a Depth-only texture without stencil
      if (isDepthStencilFormat && eFormat != GL_DEPTH_STENCIL) 
      {
        eFormat = GL_DEPTH_COMPONENT;
        GLenum newInternalFormat;
        switch (eInternalFormat) {
        case GL_R16F: newInternalFormat = GL_DEPTH_COMPONENT16;  break;
        case GL_R32F: newInternalFormat = GL_DEPTH_COMPONENT32F; break;
        default: ASSERT_M(false, "Invalid depth format requested"); break;
        }

        eInternalFormat = newInternalFormat;
      }
    }  
    //---------------------------------------------------------------------------//
    void Adapter::getGLtypeAndNumComponentsFromFormat(const DataFormat& eFormat, 
      uint32& ruComponents, GLenum& reTypeGL)
    {
      switch (eFormat)
      {
      case DataFormat::RGBA_8: 
      case DataFormat::SRGB_8:    ruComponents = 4; reTypeGL = GL_UNSIGNED_BYTE; break;
      case DataFormat::RGB_8:     ruComponents = 3; reTypeGL = GL_UNSIGNED_BYTE; break;
      case DataFormat::RGBA_16F:  ruComponents = 4; reTypeGL = GL_HALF_FLOAT; break;
      case DataFormat::RGB_16F:   ruComponents = 3; reTypeGL = GL_HALF_FLOAT; break;
      case DataFormat::RG_16F:    ruComponents = 2; reTypeGL = GL_HALF_FLOAT; break;
      case DataFormat::R_16F:     ruComponents = 1; reTypeGL = GL_HALF_FLOAT; break;
      case DataFormat::RGBA_32F:  ruComponents = 4; reTypeGL = GL_FLOAT; break;
      case DataFormat::RGB_32F:   ruComponents = 3; reTypeGL = GL_FLOAT; break;
      case DataFormat::RG_32F:    ruComponents = 2; reTypeGL = GL_FLOAT; break;
      case DataFormat::R_32F:     ruComponents = 1; reTypeGL = GL_FLOAT; break;
      case DataFormat::RGBA_32UI: ruComponents = 4; reTypeGL = GL_UNSIGNED_INT; break;
      case DataFormat::RGB_32UI:  ruComponents = 3; reTypeGL = GL_UNSIGNED_INT; break;
      case DataFormat::RG_32UI:   ruComponents = 2; reTypeGL = GL_UNSIGNED_INT; break;
      case DataFormat::R_32UI:    ruComponents = 1; reTypeGL = GL_UNSIGNED_INT; break;
      case DataFormat::RGBA_16UI: ruComponents = 4; reTypeGL = GL_UNSIGNED_SHORT; break;
      case DataFormat::RGB_16UI:  ruComponents = 3; reTypeGL = GL_UNSIGNED_SHORT; break;
      case DataFormat::RG_16UI:   ruComponents = 2; reTypeGL = GL_UNSIGNED_SHORT; break;
      case DataFormat::R_16UI:    ruComponents = 1; reTypeGL = GL_UNSIGNED_SHORT; break;
      case DataFormat::RGBA_8UI:  ruComponents = 4; reTypeGL = GL_UNSIGNED_BYTE; break;
      case DataFormat::RGB_8UI:   ruComponents = 3; reTypeGL = GL_UNSIGNED_BYTE; break;
      case DataFormat::RG_8UI:    ruComponents = 2; reTypeGL = GL_UNSIGNED_BYTE; break;
      case DataFormat::R_8UI:     ruComponents = 1; reTypeGL = GL_UNSIGNED_BYTE; break;
      default: ASSERT_M(false, "Format cannot be converted to a GL-type");
      }
    }
    //---------------------------------------------------------------------------//

} } } // end of namespace Fancy::Rendering::GL4
