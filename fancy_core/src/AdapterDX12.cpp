#include "AdapterDX12.h"

#if defined (RENDERER_DX12)

namespace Fancy {
	namespace Rendering {
		namespace DX12 {
		//---------------------------------------------------------------------------//
		// Adapter-methods to translate between the state-enums
		//-----------------------------------------------------------------------//
			D3D12_COMPARISON_FUNC Adapter::toNativeType(const CompFunc& generalType) 
			{	
				static D3D12_COMPARISON_FUNC ourTranslationTable[] =
				{
					D3D12_COMPARISON_FUNC_NEVER,
					D3D12_COMPARISON_FUNC_LESS,
					D3D12_COMPARISON_FUNC_EQUAL,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					D3D12_COMPARISON_FUNC_GREATER,
					D3D12_COMPARISON_FUNC_NOT_EQUAL,
					D3D12_COMPARISON_FUNC_GREATER_EQUAL,
					D3D12_COMPARISON_FUNC_NEVER
				};
				STATIC_ASSERT(ARRAY_LENGTH(ourTranslationTable) == (uint32)CompFunc::NUM, "Missing DX12 implementations");

				return ourTranslationTable[(uint32)generalType];
			}
		//---------------------------------------------------------------------------//
			D3D12_STENCIL_OP Adapter::toNativeType(const StencilOp& generalType) 
			{
        switch (generalType)
        {
          case StencilOp::KEEP: return D3D12_STENCIL_OP_KEEP;
          case StencilOp::ZERO: return D3D12_STENCIL_OP_ZERO;
          case StencilOp::REPLACE: return D3D12_STENCIL_OP_REPLACE;
          case StencilOp::INCREMENT_CLAMP: return D3D12_STENCIL_OP_INCR_SAT;
          case StencilOp::DECREMENT_CLAMP: return D3D12_STENCIL_OP_DECR_SAT;
          case StencilOp::INVERT: return D3D12_STENCIL_OP_INVERT;
          case StencilOp::INCEMENT_WRAP: return D3D12_STENCIL_OP_INCR;
          case StencilOp::DECREMENT_WRAP: return D3D12_STENCIL_OP_DECR;
          default: ASSERT(false); return D3D12_STENCIL_OP_KEEP;
        }
			}
		//---------------------------------------------------------------------------//
			D3D12_FILL_MODE Adapter::toNativeType(const FillMode& generalType) 
			{
				switch (generalType) 
				{
					case FillMode::WIREFRAME: return D3D12_FILL_MODE_WIREFRAME;
					case FillMode::SOLID:     return D3D12_FILL_MODE_SOLID;
					default: ASSERT(false, "Missing native values"); return D3D12_FILL_MODE_SOLID;
				}
			}
		//---------------------------------------------------------------------------//
			D3D12_CULL_MODE Adapter::toNativeType(const CullMode& generalType) 
			{
				switch (generalType) {
					case CullMode::NONE:  return D3D12_CULL_MODE_NONE; 
					case CullMode::FRONT: return D3D12_CULL_MODE_FRONT;
					case CullMode::BACK:  return D3D12_CULL_MODE_BACK;
					default: ASSERT(false, "Missing native values"); return D3D12_CULL_MODE_NONE;
				}
			}
		//---------------------------------------------------------------------------//
			// Is there a pendant in D3D?
			/*GLenum Adapter::toGLType(const WindingOrder& generalType) {
				switch (generalType) {
				case WindingOrder::CCW: return GL_CCW;
				case WindingOrder::CW:  return GL_CW;
				default: ASSERT(false, "Missing GL values"); return GL_CCW;
				}
			}*/
		//---------------------------------------------------------------------------//
			D3D12_BLEND Adapter::toNativeType(const BlendInput& generalType) 
			{
				switch (generalType) {
					case BlendInput::ZERO:                return D3D12_BLEND_ZERO;
					case BlendInput::ONE:                 return D3D12_BLEND_ONE;
					case BlendInput::SRC_COLOR:           return D3D12_BLEND_SRC_COLOR;
					case BlendInput::INV_SRC_COLOR:       return D3D12_BLEND_INV_SRC_COLOR;
					case BlendInput::SRC_ALPHA:           return D3D12_BLEND_SRC_ALPHA;
					case BlendInput::INV_SRC_ALPHA:       return D3D12_BLEND_INV_SRC_ALPHA;
					case BlendInput::DEST_ALPHA:          return D3D12_BLEND_DEST_ALPHA;
					case BlendInput::INV_DEST_ALPHA:      return D3D12_BLEND_INV_DEST_ALPHA;
					case BlendInput::DEST_COLOR:          return D3D12_BLEND_DEST_COLOR;
					case BlendInput::INV_DEST_COLOR:      return D3D12_BLEND_INV_DEST_COLOR;
					case BlendInput::SRC_ALPHA_CLAMPED:   return D3D12_BLEND_SRC_ALPHA_SAT;
					case BlendInput::CONSTANT_COLOR:
					case BlendInput::CONSTANT_ALPHA:
					case BlendInput::BLEND_FACTOR:        return D3D12_BLEND_BLEND_FACTOR;
					case BlendInput::INV_CONSTANT_COLOR:
					case BlendInput::INV_CONSTANT_ALPHA:
					case BlendInput::INV_BLEND_FACTOR:    return D3D12_BLEND_INV_BLEND_FACTOR;
					case BlendInput::SRC1_COLOR:          return D3D12_BLEND_SRC1_COLOR;
					case BlendInput::INV_SRC1_COLOR:      return D3D12_BLEND_INV_SRC1_COLOR;
					case BlendInput::SRC1_ALPHA:          return D3D12_BLEND_SRC1_ALPHA;
					case BlendInput::INV_SRC1_ALPHA:      return D3D12_BLEND_INV_SRC1_ALPHA;
					default: ASSERT(false, "Missing native values"); return D3D12_BLEND_ZERO;
				}
			}
		//---------------------------------------------------------------------------//
			D3D12_BLEND_OP Adapter::toNativeType(const BlendOp& generalType) 
			{
				switch (generalType) {
					case BlendOp::ADD:            return D3D12_BLEND_OP_ADD;
					case BlendOp::SUBTRACT:       return D3D12_BLEND_OP_SUBTRACT;
					case BlendOp::REV_SUBTRACT:   return D3D12_BLEND_OP_REV_SUBTRACT;
					case BlendOp::MIN:            return D3D12_BLEND_OP_MIN;
					case BlendOp::MAX:            return D3D12_BLEND_OP_MAX;
					default: ASSERT(false, "Missing implementation"); return D3D12_BLEND_OP_ADD;
				}
			}
		//---------------------------------------------------------------------------//
			/*GLenum Adapter::toGLType(const ShaderStage& generalType) 
			{
				switch (generalType) {
				case ShaderStage::VERTEX:       return GL_VERTEX_SHADER;
				case ShaderStage::FRAGMENT:     return GL_FRAGMENT_SHADER;
				case ShaderStage::GEOMETRY:     return GL_GEOMETRY_SHADER;
				case ShaderStage::TESS_DOMAIN:  return GL_TESS_EVALUATION_SHADER;
				case ShaderStage::TESS_HULL:    return GL_TESS_CONTROL_SHADER;
				case ShaderStage::COMPUTE:      return GL_COMPUTE_SHADER;
				default: ASSERT(false, "Missing GL values"); return GL_VERTEX_SHADER;
				}
			}*/
		//---------------------------------------------------------------------------//
			/*GLuint Adapter::toGLFlag(const ShaderStageFlag& generalType) {
				switch (generalType) {
				case ShaderStageFlag::VERTEX:       return GL_VERTEX_SHADER_BIT;
				case ShaderStageFlag::FRAGMENT:     return GL_FRAGMENT_SHADER_BIT;
				case ShaderStageFlag::GEOMETRY:     return GL_GEOMETRY_SHADER_BIT;
				case ShaderStageFlag::TESS_DOMAIN:  return GL_TESS_EVALUATION_SHADER_BIT;
				case ShaderStageFlag::TESS_HULL:    return GL_TESS_CONTROL_SHADER_BIT;
				case ShaderStageFlag::COMPUTE:      return GL_COMPUTE_SHADER_BIT;
				case ShaderStageFlag::ALL:          return GL_ALL_SHADER_BITS;
				default: ASSERT(false, "Missing GL values"); return GL_ALL_SHADER_BITS;
				}
			}*/
		//---------------------------------------------------------------------------//
			/// Returns the internal format (prior to any modifications)
			DXGI_FORMAT Adapter::toNativeType(const DataFormat& generalType)
			{
				switch (generalType)
				{
					case DataFormat::SRGB_8_A_8:     return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
					case DataFormat::RGBA_8:         return DXGI_FORMAT_R8G8B8A8_UNORM;
					case DataFormat::SRGB_8:         return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
					case DataFormat::RGB_8:          return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
					case DataFormat::RGB_11_11_10F:  return DXGI_FORMAT_R11G11B10_FLOAT;
					case DataFormat::RGBA_16F:       return DXGI_FORMAT_R16G16B16A16_FLOAT;
					case DataFormat::RGB_16F:        return DXGI_FORMAT_R16G16B16A16_FLOAT;
					case DataFormat::RG_16F:         return DXGI_FORMAT_R16G16_FLOAT;
					case DataFormat::R_16F:          return DXGI_FORMAT_R16_FLOAT;
					case DataFormat::RGBA_32F:       return DXGI_FORMAT_R32G32B32A32_FLOAT;
					case DataFormat::RGB_32F:        return DXGI_FORMAT_R32G32B32_FLOAT;
					case DataFormat::RG_32F:         return DXGI_FORMAT_R32G32_FLOAT;
					case DataFormat::R_32F:          return DXGI_FORMAT_R32_FLOAT;
					case DataFormat::RGBA_32UI:      return DXGI_FORMAT_R32G32B32A32_UINT;
					case DataFormat::RGB_32UI:       return DXGI_FORMAT_R32G32B32_UINT;
					case DataFormat::RG_32UI:        return DXGI_FORMAT_R32G32_UINT;
					case DataFormat::R_32UI:         return DXGI_FORMAT_R32_UINT;
					case DataFormat::RGBA_16UI:      return DXGI_FORMAT_R16G16B16A16_UINT;
					case DataFormat::RGB_16UI:       return DXGI_FORMAT_R16G16B16A16_UINT;
					case DataFormat::RG_16UI:        return DXGI_FORMAT_R16G16_UINT;
					case DataFormat::R_16UI:         return DXGI_FORMAT_R16_UINT;
					case DataFormat::RGBA_8UI:       return DXGI_FORMAT_R8G8B8A8_UINT;
					case DataFormat::RGB_8UI:        return DXGI_FORMAT_R8G8B8A8_UINT;
					case DataFormat::RG_8UI:         return DXGI_FORMAT_R8G8_UINT;
					case DataFormat::R_8UI:          return DXGI_FORMAT_R8_UINT;
					case DataFormat::DS_24_8:        return DXGI_FORMAT_D24_UNORM_S8_UINT;
          case DataFormat::UNKNOWN:        return DXGI_FORMAT_UNKNOWN;
					default: ASSERT(false, "Missing implementation"); return DXGI_FORMAT_R8G8B8A8_UNORM;
				}
			}
		//---------------------------------------------------------------------------//
		/*	void Adapter::getGLtypeAndNumComponentsFromFormat(const DataFormat& eFormat,
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
				default: ASSERT(false, "Format cannot be converted to a GL-type");
				}
			}
		*/	//---------------------------------------------------------------------------//

		}
	}
} // end of namespace Fancy::Rendering::DX12

#endif


