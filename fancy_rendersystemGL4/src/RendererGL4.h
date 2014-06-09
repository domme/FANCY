#ifndef INCLUDE_RENDERERGL4
#define INCLUDE_RENDERERGL4

#include <FancyCorePrerequisites.h>
#include <Renderer.h>

#include "OpenGLprerequisites.h"

namespace FANCY { namespace Core { namespace Rendering {

// Adapter-methods to translate between the state-enums
namespace AdapterGL {
  template<typename T, typename R>
  R toGLType(const T& generalType) {ASSERT_M(false, "Dummy-template called");}
  template<typename T, typename R>
  T toGeneralType(const R& glType) {ASSERT_M(false, "Dummy-template called");}

  // Comp Func
  template<>
  GLenum toGLType(const CompFunc::Enum& generalType) {
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
  GLenum toGLType(const StencilOp::Enum& generalType) {
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
  GLenum toGLType(const FillMode::Enum& generalType) {
    switch (generalType) {
      case FillMode::WIREFRAME: return GL_LINE;
      case FillMode::SOLID:     return GL_FILL;
      default: ASSERT_M(false, "Missing GL values"); return GL_FILL;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLenum toGLType(const CullMode::Enum& generalType) {
    switch (generalType) {
      case CullMode::NONE:  return GL_NONE; // Special type. Requires call to glDisable(CULLING)
      case CullMode::FRONT: return GL_FRONT;
      case CullMode::BACK:  return GL_BACK;
      default: ASSERT_M(false, "Missing GL values"); return GL_BACK;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLenum toGLType(const WindingOrder::Enum& generalType) {
    switch (generalType) {
      case WindingOrder::CCW: return GL_CCW;
      case WindingOrder::CW:  return GL_CW;
      default: ASSERT_M(false, "Missing GL values"); return GL_CCW;
    }
  }
//////////////////////////////////////////////////////////////////////////
  template<>
  GLenum toGLType(const BlendInput::Enum& generalType) {
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
  GLenum toGLType(const BlendOp::Enum& generalType) {
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
  GLenum toGLType(const ShaderStage::Enum& generalType) {
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
  /// Retrieve the shader-stage flag used for calls like "glUseProgramStages()"
  GLuint getGLShaderStageFlag(const ShaderStage::Enum& generalType) {
    switch (generalType) {
      case ShaderStage::VERTEX:       return GL_VERTEX_SHADER_BIT;
      case ShaderStage::FRAGMENT:     return GL_FRAGMENT_SHADER_BIT;
      case ShaderStage::GEOMETRY:     return GL_GEOMETRY_SHADER_BIT;
      case ShaderStage::TESS_DOMAIN:  return GL_TESS_EVALUATION_SHADER_BIT;
      case ShaderStage::TESS_HULL:    return GL_TESS_CONTROL_SHADER_BIT;
      case ShaderStage::COMPUTE:      return GL_COMPUTE_SHADER_BIT;
      default: ASSERT_M(false, "Missing GL values"); return GL_ALL_SHADER_BITS;
    }
  }
//////////////////////////////////////////////////////////////////////////
}  // end namespace AdapterGL



class DLLEXPORT RendererGL4 : public FANCY::Core::Rendering::RendererImpl
{
public:
  RendererGL4();
  virtual ~RendererGL4();

  virtual bool _init() override;
  virtual bool _destroy() override;

  virtual void _setDepthStencilState(const DepthStencilState& clState) override;
  virtual void _setFillMode(FillMode::Enum eFillMode) override;
  virtual void _setCullMode(CullMode::Enum eCullMode) override;
  virtual void _setWindingOrder(WindingOrder::Enum eWindingOrder) override;
  virtual void _setBlendState(const BlendState& clState) override;

  // TODO: Mesh will become a thin geometric representation in the future
  virtual void _render(Mesh* pMesh) override;  
  virtual void _renderIndirect( /*params ?*/ ) override;
  virtual void _dispatchCompute( /*params ?*/ ) override;
  virtual void _dispatchComputeIndirect( /*params ?*/ ) override;

  /// Resource-bindings
  virtual void _bindRenderTargets(Texture** pTexList, uint8 u8NumRTs) override;
  virtual void _bindReadTextures(ShaderStage::Enum eShaderStage, const Texture** pTexList, uint8 u8NumTextures) override;
  virtual void _bindReadBuffers(ShaderStage::Enum eShaderStage,  const Buffer** pBufferList, uint8 u8NumBuffers) override;
  virtual void _bindConstantBuffers(ShaderStage::Enum eShaderStage, const ConstantBuffer** pBufferList, uint8 u8NumBuffers) override;
  virtual void _bindTextureSamplers(ShaderStage::Enum eShaderStage, const TextureSampler** pTexSamplerList, uint8 u8NumTexSamplers) override;
  virtual void _bindGPUProgram(ShaderStage::Enum  eShaderStage, const GPUProgram* pProgram) override;
};

} // end of namespace Rendering
} // end of namespace Core
} // end of namespace FANCY

#endif  // INCLUDE_RENDERERGL4