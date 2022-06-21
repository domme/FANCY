#include "fancy_core_precompile.h"

#include "Rendering/Shader.h"
#include "Rendering/ShaderCompiler.h"
#include "Rendering/RenderCore.h"
#include "Common/CommandLine.h"

#include "RenderCore_PlatformDX12.h"
#include "DescriptorDX12.h"
#include "ShaderCompilerDX12.h"
#include "ShaderDX12.h"
#include "TextureDX12.h"
#include "GpuBufferDX12.h"

#include "ShaderPipelineDX12.h"
#include "ShaderVisibleDescriptorHeapDX12.h"
#include "RenderOutputDX12.h"
#include "CommandListDX12.h"
#include "GpuResourceDataDX12.h"
#include "AdapterDX12.h"
#include "GpuQueryHeapDX12.h"
#include "TextureSamplerDX12.h"
#include "RtAccelerationStructureDX12.h"
#include "RtPipelineStateDX12.h"

#if FANCY_ENABLE_DX12

using namespace Fancy;

//---------------------------------------------------------------------------//
namespace {
  //---------------------------------------------------------------------------//
  D3D12_SRV_DIMENSION GetSRVDimension(GpuResourceDimension aDimension)
  {
    switch (aDimension)
    {
    case GpuResourceDimension::UNKONWN: return D3D12_SRV_DIMENSION_BUFFER;
    case GpuResourceDimension::BUFFER: return D3D12_SRV_DIMENSION_BUFFER;
    case GpuResourceDimension::TEXTURE_1D: return D3D12_SRV_DIMENSION_TEXTURE1D;
    case GpuResourceDimension::TEXTURE_2D: return D3D12_SRV_DIMENSION_TEXTURE2D;
    case GpuResourceDimension::TEXTURE_3D: return D3D12_SRV_DIMENSION_TEXTURE3D;
    case GpuResourceDimension::TEXTURE_CUBE: return D3D12_SRV_DIMENSION_TEXTURECUBE;
    case GpuResourceDimension::TEXTURE_1D_ARRAY: return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
    case GpuResourceDimension::TEXTURE_2D_ARRAY: return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
    case GpuResourceDimension::TEXTURE_CUBE_ARRAY: return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
    default: ASSERT(false); return D3D12_SRV_DIMENSION_BUFFER;
    }
  }
  //---------------------------------------------------------------------------//
  bool TryGetUAVDimension(GpuResourceDimension aDimension, D3D12_UAV_DIMENSION& aUAVDimension)
  {
    switch (aDimension)
    {
    case GpuResourceDimension::UNKONWN:
      aUAVDimension = D3D12_UAV_DIMENSION_BUFFER;
      return true;
    case GpuResourceDimension::BUFFER:
      aUAVDimension = D3D12_UAV_DIMENSION_BUFFER;
      return true;
    case GpuResourceDimension::TEXTURE_1D:
      aUAVDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
      return true;
    case GpuResourceDimension::TEXTURE_2D:
      aUAVDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
      return true;
    case GpuResourceDimension::TEXTURE_3D:
      aUAVDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
      return true;
    case GpuResourceDimension::TEXTURE_1D_ARRAY:
      aUAVDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
      return true;
    case GpuResourceDimension::TEXTURE_2D_ARRAY:
      aUAVDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
      return true;
    default: aUAVDimension = D3D12_UAV_DIMENSION_BUFFER; return false;
    }
  }
  //---------------------------------------------------------------------------//
  CommandListType locGetCommandListType(D3D12_COMMAND_LIST_TYPE aType)
  {
    switch (aType)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT: return CommandListType::Graphics;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE: return CommandListType::Compute;
    case D3D12_COMMAND_LIST_TYPE_COPY: return CommandListType::DMA;
    default:
      ASSERT("Command list type %d not supported", (uint)aType);
      return CommandListType::Graphics;
    }
  }
  //---------------------------------------------------------------------------//
}  // namespace
//---------------------------------------------------------------------------//
D3D12_LOGIC_OP RenderCore_PlatformDX12::ResolveLogicOp(LogicOp aLogicOp)
{
  switch (aLogicOp)
  {
  case LogicOp::CLEAR:          return D3D12_LOGIC_OP_CLEAR;
  case LogicOp::AND:            return D3D12_LOGIC_OP_AND;
  case LogicOp::AND_REVERSE:    return D3D12_LOGIC_OP_AND_REVERSE;
  case LogicOp::COPY:           return D3D12_LOGIC_OP_COPY;
  case LogicOp::AND_INVERTED:   return D3D12_LOGIC_OP_AND_INVERTED;
  case LogicOp::NO_OP:          return D3D12_LOGIC_OP_NOOP;
  case LogicOp::XOR:            return D3D12_LOGIC_OP_XOR;
  case LogicOp::OR:             return D3D12_LOGIC_OP_OR;
  case LogicOp::NOR:            return D3D12_LOGIC_OP_NOR;
  case LogicOp::EQUIVALENT:     return D3D12_LOGIC_OP_EQUIV;
  case LogicOp::INVERT:         return D3D12_LOGIC_OP_INVERT;
  case LogicOp::OR_REVERSE:     return D3D12_LOGIC_OP_OR_REVERSE;
  case LogicOp::COPY_INVERTED:  return D3D12_LOGIC_OP_COPY_INVERTED;
  case LogicOp::OR_INVERTED:    return D3D12_LOGIC_OP_OR_INVERTED;
  case LogicOp::NAND:           return D3D12_LOGIC_OP_NAND;
  case LogicOp::SET:            return D3D12_LOGIC_OP_SET;
  default: ASSERT(false, "Missing implementation"); return D3D12_LOGIC_OP_NOOP;
  }
}
//---------------------------------------------------------------------------//
D3D12_COMPARISON_FUNC RenderCore_PlatformDX12::ResolveCompFunc(const CompFunc& aCompFunc)
{
  switch (aCompFunc)
  {
  case CompFunc::NEVER:     return D3D12_COMPARISON_FUNC_NEVER;
  case CompFunc::LESS:      return D3D12_COMPARISON_FUNC_LESS;
  case CompFunc::EQUAL:     return D3D12_COMPARISON_FUNC_EQUAL;
  case CompFunc::LEQUAL:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
  case CompFunc::GREATER:   return D3D12_COMPARISON_FUNC_GREATER;
  case CompFunc::NOTEQUAL:  return D3D12_COMPARISON_FUNC_NOT_EQUAL;
  case CompFunc::GEQUAL:    return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
  case CompFunc::ALWAYS:    return D3D12_COMPARISON_FUNC_ALWAYS;
  default: ASSERT(false, "Missing implementation"); return D3D12_COMPARISON_FUNC_ALWAYS;
  }
}
//---------------------------------------------------------------------------//
D3D12_STENCIL_OP RenderCore_PlatformDX12::ResolveStencilOp(const StencilOp& aStencilOp)
{
  switch (aStencilOp)
  {
  case StencilOp::KEEP:             return D3D12_STENCIL_OP_KEEP;
  case StencilOp::ZERO:             return D3D12_STENCIL_OP_ZERO;
  case StencilOp::REPLACE:          return D3D12_STENCIL_OP_REPLACE;
  case StencilOp::INCREMENT_CLAMP:  return D3D12_STENCIL_OP_INCR_SAT;
  case StencilOp::DECREMENT_CLAMP:  return D3D12_STENCIL_OP_DECR_SAT;
  case StencilOp::INVERT:           return D3D12_STENCIL_OP_INVERT;
  case StencilOp::INCEMENT_WRAP:    return D3D12_STENCIL_OP_INCR;
  case StencilOp::DECREMENT_WRAP:   return D3D12_STENCIL_OP_DECR;
  default: ASSERT(false); return D3D12_STENCIL_OP_KEEP;
  }
}
//---------------------------------------------------------------------------//
DXGI_FORMAT RenderCore_PlatformDX12::ResolveFormat(DataFormat aFormat)
{
  switch (aFormat)
  {
  case DataFormat::SRGB_8_A_8:        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  case DataFormat::RGBA_8:            return DXGI_FORMAT_R8G8B8A8_UNORM;
  case DataFormat::BGRA_8:            return DXGI_FORMAT_B8G8R8A8_UNORM;
  case DataFormat::RG_8:              return DXGI_FORMAT_R8G8_UNORM;
  case DataFormat::R_8:               return DXGI_FORMAT_R8_UNORM;
  case DataFormat::RGBA_16:           return DXGI_FORMAT_R16G16B16A16_UNORM;
  case DataFormat::RG_16:             return DXGI_FORMAT_R16G16_UNORM;
  case DataFormat::R_16:              return DXGI_FORMAT_R16_UNORM;
  case DataFormat::RGB_11_11_10F:     return DXGI_FORMAT_R11G11B10_FLOAT;
  case DataFormat::RGBA_16F:          return DXGI_FORMAT_R16G16B16A16_FLOAT;
  case DataFormat::RG_16F:            return DXGI_FORMAT_R16G16_FLOAT;
  case DataFormat::R_16F:             return DXGI_FORMAT_R16_FLOAT;
  case DataFormat::RGBA_32F:          return DXGI_FORMAT_R32G32B32A32_FLOAT;
  case DataFormat::RGB_32F:           return DXGI_FORMAT_R32G32B32_FLOAT;
  case DataFormat::RG_32F:            return DXGI_FORMAT_R32G32_FLOAT;
  case DataFormat::R_32F:             return DXGI_FORMAT_R32_FLOAT;
  case DataFormat::RGBA_32UI:         return DXGI_FORMAT_R32G32B32A32_UINT;
  case DataFormat::RGB_32UI:          return DXGI_FORMAT_R32G32B32_UINT;
  case DataFormat::RG_32UI:           return DXGI_FORMAT_R32G32_UINT;
  case DataFormat::R_32UI:            return DXGI_FORMAT_R32_UINT;
  case DataFormat::RGBA_16UI:         return DXGI_FORMAT_R16G16B16A16_UINT;
  case DataFormat::RG_16UI:           return DXGI_FORMAT_R16G16_UINT;
  case DataFormat::R_16UI:            return DXGI_FORMAT_R16_UINT;
  case DataFormat::RGBA_8UI:          return DXGI_FORMAT_R8G8B8A8_UINT;
  case DataFormat::RG_8UI:            return DXGI_FORMAT_R8G8_UINT;
  case DataFormat::R_8UI:             return DXGI_FORMAT_R8_UINT;
  case DataFormat::D_24UNORM_S_8UI:   return DXGI_FORMAT_D24_UNORM_S8_UINT;
  case DataFormat::UNKNOWN:           return DXGI_FORMAT_UNKNOWN;

  case DataFormat::RGB_16F:
  case DataFormat::RGB_16UI:
  case DataFormat::RGB_8UI:
  default: ASSERT(false, "Missing implementation or unsupported format"); return DXGI_FORMAT_R8G8B8A8_UNORM;
  }
}
//---------------------------------------------------------------------------//
DXGI_FORMAT RenderCore_PlatformDX12::GetDepthStencilTextureFormat(DXGI_FORMAT aFormat)
{
  switch (aFormat)
  {
    // 32-bit Z w/ Stencil
  case DXGI_FORMAT_R32G8X24_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
  case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
  case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    return DXGI_FORMAT_R32G8X24_TYPELESS;

    // No Stencil
  case DXGI_FORMAT_R32_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT:
  case DXGI_FORMAT_R32_FLOAT:
    return DXGI_FORMAT_R32_TYPELESS;

    // 24-bit Z
  case DXGI_FORMAT_R24G8_TYPELESS:
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
  case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
  case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    return DXGI_FORMAT_R24G8_TYPELESS;

    // 16-bit Z w/o Stencil
  case DXGI_FORMAT_R16_TYPELESS:
  case DXGI_FORMAT_D16_UNORM:
  case DXGI_FORMAT_R16_UNORM:
    return DXGI_FORMAT_R16_TYPELESS;

  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}
//---------------------------------------------------------------------------//
DXGI_FORMAT RenderCore_PlatformDX12::GetDepthStencilViewFormat(DXGI_FORMAT aFormat)
{
  switch (aFormat)
  {
    // 32-bit Z w/ Stencil
  case DXGI_FORMAT_R32G8X24_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
  case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
  case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

    // No Stencil
  case DXGI_FORMAT_R32_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT:
  case DXGI_FORMAT_R32_FLOAT:
    return DXGI_FORMAT_D32_FLOAT;

    // 24-bit Z
  case DXGI_FORMAT_R24G8_TYPELESS:
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
  case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
  case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    return DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 16-bit Z w/o Stencil
  case DXGI_FORMAT_R16_TYPELESS:
  case DXGI_FORMAT_D16_UNORM:
  case DXGI_FORMAT_R16_UNORM:
    return DXGI_FORMAT_D16_UNORM;

  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}
//---------------------------------------------------------------------------//
DXGI_FORMAT RenderCore_PlatformDX12::GetDepthViewFormat(DXGI_FORMAT aFormat)
{
  switch (aFormat)
  {
    // 32-bit Z w/ Stencil
  case DXGI_FORMAT_R32G8X24_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
  case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
  case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

    // No Stencil
  case DXGI_FORMAT_R32_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT:
  case DXGI_FORMAT_R32_FLOAT:
    return DXGI_FORMAT_R32_FLOAT;

    // 24-bit Z
  case DXGI_FORMAT_R24G8_TYPELESS:
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
  case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
  case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

    // 16-bit Z w/o Stencil
  case DXGI_FORMAT_R16_TYPELESS:
  case DXGI_FORMAT_D16_UNORM:
  case DXGI_FORMAT_R16_UNORM:
    return DXGI_FORMAT_R16_UNORM;

  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}
//---------------------------------------------------------------------------//
DXGI_FORMAT RenderCore_PlatformDX12::GetStencilViewFormat(DXGI_FORMAT aFormat)
{
  switch (aFormat)
  {
    // 32-bit Z w/ Stencil
  case DXGI_FORMAT_R32G8X24_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
  case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
  case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

    // 24-bit Z
  case DXGI_FORMAT_R24G8_TYPELESS:
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
  case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
  case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}
//---------------------------------------------------------------------------//
DXGI_FORMAT RenderCore_PlatformDX12::GetTypelessFormat(DXGI_FORMAT aFormat)
{
  switch (aFormat)
  {
  case DXGI_FORMAT_R32G32B32A32_FLOAT:
  case DXGI_FORMAT_R32G32B32A32_UINT:
  case DXGI_FORMAT_R32G32B32A32_SINT:
  case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    return DXGI_FORMAT_R32G32B32A32_TYPELESS;

  case DXGI_FORMAT_R32G32B32_FLOAT:
  case DXGI_FORMAT_R32G32B32_UINT:
  case DXGI_FORMAT_R32G32B32_SINT:
  case DXGI_FORMAT_R32G32B32_TYPELESS:
    return DXGI_FORMAT_R32G32B32_TYPELESS;

  case DXGI_FORMAT_R16G16B16A16_FLOAT:
  case DXGI_FORMAT_R16G16B16A16_UNORM:
  case DXGI_FORMAT_R16G16B16A16_UINT:
  case DXGI_FORMAT_R16G16B16A16_SNORM:
  case DXGI_FORMAT_R16G16B16A16_SINT:
  case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    return DXGI_FORMAT_R16G16B16A16_TYPELESS;

  case DXGI_FORMAT_R32G32_FLOAT:
  case DXGI_FORMAT_R32G32_UINT:
  case DXGI_FORMAT_R32G32_SINT:
  case DXGI_FORMAT_R32G32_TYPELESS:
    return DXGI_FORMAT_R32G32_TYPELESS;

  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
  case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

  case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
  case DXGI_FORMAT_R32G8X24_TYPELESS:
    return DXGI_FORMAT_R32G8X24_TYPELESS;

  case DXGI_FORMAT_R10G10B10A2_UNORM:
  case DXGI_FORMAT_R10G10B10A2_UINT:
  case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
  case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    // case DXGI_FORMAT_R11G11B10_FLOAT  // This is most likely not a valid format to cast into from DXGI_FORMAT_R10G10B10A2_TYPELESS...
    return DXGI_FORMAT_R10G10B10A2_TYPELESS;

  case DXGI_FORMAT_R8G8B8A8_UNORM:
  case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
  case DXGI_FORMAT_R8G8B8A8_UINT:
  case DXGI_FORMAT_R8G8B8A8_SNORM:
  case DXGI_FORMAT_R8G8B8A8_SINT:
  case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    return DXGI_FORMAT_R8G8B8A8_TYPELESS;

  case DXGI_FORMAT_R16G16_FLOAT:
  case DXGI_FORMAT_R16G16_UNORM:
  case DXGI_FORMAT_R16G16_UINT:
  case DXGI_FORMAT_R16G16_SNORM:
  case DXGI_FORMAT_R16G16_SINT:
  case DXGI_FORMAT_R16G16_TYPELESS:
    return DXGI_FORMAT_R16G16_TYPELESS;

  case DXGI_FORMAT_D32_FLOAT:
  case DXGI_FORMAT_R32_FLOAT:
  case DXGI_FORMAT_R32_UINT:
  case DXGI_FORMAT_R32_SINT:
  case DXGI_FORMAT_R32_TYPELESS:
    return DXGI_FORMAT_R32_TYPELESS;

  case DXGI_FORMAT_D24_UNORM_S8_UINT:
  case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

  case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
  case DXGI_FORMAT_R24G8_TYPELESS:
    return DXGI_FORMAT_R24G8_TYPELESS;

  case DXGI_FORMAT_R8G8_UNORM:
  case DXGI_FORMAT_R8G8_UINT:
  case DXGI_FORMAT_R8G8_SNORM:
  case DXGI_FORMAT_R8G8_SINT:
  case DXGI_FORMAT_R8G8_TYPELESS:
    return DXGI_FORMAT_R8G8_TYPELESS;

  case DXGI_FORMAT_R16_FLOAT:
  case DXGI_FORMAT_D16_UNORM:
  case DXGI_FORMAT_R16_UNORM:
  case DXGI_FORMAT_R16_UINT:
  case DXGI_FORMAT_R16_SNORM:
  case DXGI_FORMAT_R16_SINT:
  case DXGI_FORMAT_R16_TYPELESS:
    return DXGI_FORMAT_R16_TYPELESS;

  case DXGI_FORMAT_R8_UNORM:
  case DXGI_FORMAT_R8_UINT:
  case DXGI_FORMAT_R8_SNORM:
  case DXGI_FORMAT_R8_SINT:
  case DXGI_FORMAT_A8_UNORM:
  case DXGI_FORMAT_R8_TYPELESS:
    return DXGI_FORMAT_R8_TYPELESS;

  case DXGI_FORMAT_BC1_UNORM:
  case DXGI_FORMAT_BC1_UNORM_SRGB:
  case DXGI_FORMAT_BC1_TYPELESS:
    return DXGI_FORMAT_BC1_TYPELESS;

  case DXGI_FORMAT_BC2_UNORM:
  case DXGI_FORMAT_BC2_UNORM_SRGB:
  case DXGI_FORMAT_BC2_TYPELESS:
    return DXGI_FORMAT_BC2_TYPELESS;

  case DXGI_FORMAT_BC3_UNORM:
  case DXGI_FORMAT_BC3_UNORM_SRGB:
  case DXGI_FORMAT_BC3_TYPELESS:
    return DXGI_FORMAT_BC3_TYPELESS;

  case DXGI_FORMAT_BC4_UNORM:
  case DXGI_FORMAT_BC4_SNORM:
  case DXGI_FORMAT_BC4_TYPELESS:
    return DXGI_FORMAT_BC4_TYPELESS;

  case DXGI_FORMAT_BC5_UNORM:
  case DXGI_FORMAT_BC5_SNORM:
  case DXGI_FORMAT_BC5_TYPELESS:
    return DXGI_FORMAT_BC5_TYPELESS;

  case DXGI_FORMAT_B8G8R8A8_UNORM:
  case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
  case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    return DXGI_FORMAT_B8G8R8A8_TYPELESS;

  case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
  case DXGI_FORMAT_B8G8R8X8_UNORM:
  case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    return DXGI_FORMAT_B8G8R8X8_TYPELESS;

  case DXGI_FORMAT_BC6H_UF16:
  case DXGI_FORMAT_BC6H_SF16:
  case DXGI_FORMAT_BC6H_TYPELESS:
    return DXGI_FORMAT_BC6H_TYPELESS;

  case DXGI_FORMAT_BC7_UNORM:
  case DXGI_FORMAT_BC7_UNORM_SRGB:
  case DXGI_FORMAT_BC7_TYPELESS:
    return DXGI_FORMAT_BC7_TYPELESS;

  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}
//---------------------------------------------------------------------------//
DXGI_FORMAT RenderCore_PlatformDX12::GetCopyableFormat(DXGI_FORMAT aFormat, uint aPlaneIndex)
{
  switch (aFormat)
  {
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
    ASSERT(aPlaneIndex < 2);
    return aPlaneIndex == 0 ? DXGI_FORMAT_R32_FLOAT : DXGI_FORMAT_R8_UINT;
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    ASSERT(aPlaneIndex < 2);
    return aPlaneIndex == 0 ? DXGI_FORMAT_R32_FLOAT : DXGI_FORMAT_R32_UINT;
  case DXGI_FORMAT_D32_FLOAT:
    ASSERT(aPlaneIndex == 0u);
    return DXGI_FORMAT_R32_FLOAT;
  }

  ASSERT(aPlaneIndex == 0u);
  return aFormat;
}
//---------------------------------------------------------------------------//
D3D12_COMMAND_LIST_TYPE RenderCore_PlatformDX12::GetCommandListType(CommandListType aType)
{
  switch (aType)
  {
  case CommandListType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
  case CommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
  case CommandListType::DMA: return D3D12_COMMAND_LIST_TYPE_COPY;
  default:
    ASSERT(false);
    return D3D12_COMMAND_LIST_TYPE_DIRECT;
  }
}
//---------------------------------------------------------------------------//
D3D12_HEAP_TYPE RenderCore_PlatformDX12::ResolveHeapType(CpuMemoryAccessType anAccessType)
{
  switch (anAccessType) {
  case CpuMemoryAccessType::NO_CPU_ACCESS: return D3D12_HEAP_TYPE_DEFAULT;
  case CpuMemoryAccessType::CPU_WRITE: return D3D12_HEAP_TYPE_UPLOAD;
  case CpuMemoryAccessType::CPU_READ: return D3D12_HEAP_TYPE_READBACK;
  default: ASSERT(false, "Missing implementation"); return D3D12_HEAP_TYPE_DEFAULT;
  }
}
//---------------------------------------------------------------------------//
D3D12_DESCRIPTOR_HEAP_TYPE RenderCore_PlatformDX12::GetDescriptorHeapType(const GpuResourceViewType& aViewType)
{
  switch (aViewType)
  {
  case GpuResourceViewType::CBV: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  case GpuResourceViewType::SRV: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  case GpuResourceViewType::UAV: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  case GpuResourceViewType::DSV: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  case GpuResourceViewType::RTV: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  default: ASSERT(false, "Missing implementation"); return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  }
}
//---------------------------------------------------------------------------//
D3D12_DESCRIPTOR_RANGE_TYPE RenderCore_PlatformDX12::GetDescriptorRangeType(const GpuResourceViewType& aViewType)
{
  switch (aViewType)
  {
  case GpuResourceViewType::CBV: return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
  case GpuResourceViewType::SRV: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  case GpuResourceViewType::UAV: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
  default: ASSERT(false, "Missing implementation or invalid view type for descriptor range type"); return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  }
}
//---------------------------------------------------------------------------//
D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE RenderCore_PlatformDX12::GetRtAccelerationStructureType(RtAccelerationStructureType aType)
{
  switch (aType)
  {
  case RtAccelerationStructureType::BOTTOM_LEVEL: return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
  case RtAccelerationStructureType::TOP_LEVEL: return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
  default: ASSERT(false); return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
  }
}
//---------------------------------------------------------------------------//
D3D12_RAYTRACING_GEOMETRY_TYPE RenderCore_PlatformDX12::GetRaytracingBVHGeometryType(RtAccelerationStructureGeometryType aGeoType)
{
  switch (aGeoType)
  {
  case RtAccelerationStructureGeometryType::TRIANGLES: return D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
  case RtAccelerationStructureGeometryType::AABBS: return D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
  default: ASSERT(false, "Not implemented") return D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
  }
}
//---------------------------------------------------------------------------//
D3D12_HIT_GROUP_TYPE RenderCore_PlatformDX12::GetRaytracingHitGroupType(RtHitGroupType aType)
{
  static D3D12_HIT_GROUP_TYPE toNativeType[RT_HIT_GROUP_TYPE_NUM] =
  {
    D3D12_HIT_GROUP_TYPE_TRIANGLES,
    D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE,
  };

  ASSERT((uint) aType < (uint) ARRAY_LENGTH(toNativeType));
  return toNativeType[aType];
}
//---------------------------------------------------------------------------//
D3D12_RAYTRACING_PIPELINE_FLAGS RenderCore_PlatformDX12::GetRaytracingPipelineFlags(RtPipelineFlags someFlags)
{
  D3D12_RAYTRACING_PIPELINE_FLAGS dstFlags = (D3D12_RAYTRACING_PIPELINE_FLAGS)0u;
  if (someFlags & RT_PIPELINE_FLAG_SKIP_TRIANGLES)
    dstFlags |= D3D12_RAYTRACING_PIPELINE_FLAG_SKIP_TRIANGLES;

  if (someFlags & RT_PIPELINE_FLAG_SKIP_PROCEDURAL)
    dstFlags |= D3D12_RAYTRACING_PIPELINE_FLAG_SKIP_PROCEDURAL_PRIMITIVES;

  return dstFlags;
}
//---------------------------------------------------------------------------//
RenderCore_PlatformDX12::RenderCore_PlatformDX12(const RenderPlatformProperties& someProperties)
  : RenderCore_Platform(RenderPlatformType::DX12, someProperties)
  , myGpuTicksToMsFactor{}
{
  using namespace Microsoft::WRL;

  memset(ourCommandAllocatorPools, 0u, sizeof(ourCommandAllocatorPools));

  const bool enableDebugLayer = CommandLine::GetInstance()->HasArgument("DebugLayer");

  if (enableDebugLayer)
  {
    ComPtr<ID3D12Debug3> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
    {
      debugInterface->EnableDebugLayer();

      const bool gpuValidation = CommandLine::GetInstance()->HasArgument("GPUValidation");
      debugInterface->SetEnableGPUBasedValidation(gpuValidation);
    }
    else
    {
      LOG_ERROR("Failed to initialize DX12 debug layer");
    }
  }

  ASSERT_HRESULT(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&ourDevice)));

  if (enableDebugLayer)
  {
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(ourDevice->QueryInterface(IID_PPV_ARGS(&infoQueue))))
    {
      D3D12_MESSAGE_SEVERITY severityIds[] =
      {
        D3D12_MESSAGE_SEVERITY_INFO
      };

      D3D12_MESSAGE_ID denyIds[] =
      {
        D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES
      };

      D3D12_INFO_QUEUE_FILTER filter = {};
      filter.DenyList.NumSeverities = ARRAYSIZE(severityIds);
      filter.DenyList.pSeverityList = severityIds;
      filter.DenyList.NumIDs = ARRAYSIZE(denyIds);
      filter.DenyList.pIDList = denyIds;

      infoQueue->PushStorageFilter(&filter);

      if (CommandLine::GetInstance()->HasArgument("DebugLayerBreak"))
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    }
  }

  // Init Caps
  myCaps.myMaxNumVertexAttributes = D3D12_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT;
  myCaps.myCbufferPlacementAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
  myCaps.myTextureRowAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
  myCaps.myTextureSubresourceBufferAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
  myCaps.myMaxTextureAnisotropy = D3D12_DEFAULT_MAX_ANISOTROPY;
  myCaps.myRaytracingShaderIdentifierSizeBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
  myCaps.myRaytracingShaderRecordAlignment = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
  myCaps.myRaytracingShaderTableAddressAlignment = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
  myCaps.myRaytracingMaxShaderRecordSize = D3D12_RAYTRACING_MAX_SHADER_RECORD_STRIDE;
  // DX12 always supports async compute and copy on the API-level, even though there might not
  // be hardware-support for it.
  // TODO: Check if there's a way to detect missing HW-support and disable the missing queues
  myCaps.myHasAsyncCompute = true;
  myCaps.myHasAsyncCopy = true;
}
//---------------------------------------------------------------------------//
void RenderCore_PlatformDX12::BeginFrame()
{
  myShaderVisibleDescriptorHeap->ProcessGlobalDescriptorFrees();
}
//---------------------------------------------------------------------------//
bool RenderCore_PlatformDX12::InitInternalResources()
{
  ourCommandAllocatorPools[(uint)CommandListType::Graphics].reset(new CommandAllocatorPoolDX12(CommandListType::Graphics));
  if (myCaps.myHasAsyncCompute)
    ourCommandAllocatorPools[(uint)CommandListType::Compute].reset(new CommandAllocatorPoolDX12(CommandListType::Compute));

  myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)CpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, CpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
  myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)CpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, CpuMemoryAccessType::CPU_WRITE, 64 * SIZE_MB));
  myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)CpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, CpuMemoryAccessType::CPU_READ, 64 * SIZE_MB));

  myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)CpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, CpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
  myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)CpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, CpuMemoryAccessType::CPU_WRITE, 16 * SIZE_MB));
  myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)CpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, CpuMemoryAccessType::CPU_READ, 16 * SIZE_MB));

  myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)CpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, CpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
  myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)CpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, CpuMemoryAccessType::CPU_WRITE, 16 * SIZE_MB));
  myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)CpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, CpuMemoryAccessType::CPU_READ, 16 * SIZE_MB));

  myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024u));
  myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 512u));
  myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64u));
  myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64u));

  InitNullDescriptors();  // Must be available before creating dynamic descriptor heaps

  myShaderVisibleDescriptorHeap.reset(new ShaderVisibleDescriptorHeapDX12(myProperties));

  myRootSignature.reset(new RootSignatureDX12(myProperties));

  return true;
}
//---------------------------------------------------------------------------//
void RenderCore_PlatformDX12::InitNullDescriptors()
{
  // Create null descriptors to use as binding-dummies in descriptor tables where some elements are optimized away (e.g. because of unused resources)
  for (uint i = 0; i < (uint)GpuResourceDimension::NUM; ++i)
  {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    if (i == (uint)GpuResourceDimension::UNKONWN || i == (uint)GpuResourceDimension::BUFFER)
      srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    else
      srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    srvDesc.ViewDimension = GetSRVDimension(static_cast<GpuResourceDimension>(i));

    switch (srvDesc.ViewDimension)
    {
    case D3D12_SRV_DIMENSION_BUFFER:
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
      srvDesc.Buffer.FirstElement = 0;
      srvDesc.Buffer.NumElements = 1;
      break;
    case D3D12_SRV_DIMENSION_TEXTURE1D:
      srvDesc.Texture1D.MipLevels = 1;
      srvDesc.Texture1D.MostDetailedMip = 0;
      srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
      break;
    case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
      srvDesc.Texture1DArray.MipLevels = 1;
      srvDesc.Texture1DArray.MostDetailedMip = 0;
      srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
      srvDesc.Texture1DArray.ArraySize = 1;
      srvDesc.Texture1DArray.FirstArraySlice = 0;
      break;
    case D3D12_SRV_DIMENSION_TEXTURE2D:
      srvDesc.Texture2D.MipLevels = 1;
      srvDesc.Texture2D.MostDetailedMip = 0;
      srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
      srvDesc.Texture2D.PlaneSlice = 0;
      break;
    case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
      srvDesc.Texture2DArray.MipLevels = 1;
      srvDesc.Texture2DArray.MostDetailedMip = 0;
      srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
      srvDesc.Texture2DArray.ArraySize = 1;
      srvDesc.Texture2DArray.FirstArraySlice = 0;
      srvDesc.Texture2DArray.PlaneSlice = 0;
      break;
    case D3D12_SRV_DIMENSION_TEXTURE3D:
      srvDesc.Texture3D.MipLevels = 1;
      srvDesc.Texture3D.MostDetailedMip = 0;
      srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
      break;
    case D3D12_SRV_DIMENSION_TEXTURECUBE:
      srvDesc.TextureCube.MipLevels = 1;
      srvDesc.TextureCube.MostDetailedMip = 0;
      srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
      break;
    case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
      srvDesc.TextureCubeArray.MipLevels = 1;
      srvDesc.TextureCubeArray.MostDetailedMip = 0;
      srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
      srvDesc.TextureCubeArray.First2DArrayFace = 0;
      srvDesc.TextureCubeArray.NumCubes = 1;
      break;
    default: ASSERT(false);
    }

    DescriptorDX12 descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    ourDevice->CreateShaderResourceView(nullptr, &srvDesc, descriptor.myCpuHandle);
    mySRVNullDescriptors[i] = descriptor;

    D3D12_UAV_DIMENSION uavDimension;
    if (TryGetUAVDimension(static_cast<GpuResourceDimension>(i), uavDimension))
    {
      D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
      if (i == (uint)GpuResourceDimension::UNKONWN || i == (uint)GpuResourceDimension::BUFFER)
        uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      else
        uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      uavDesc.ViewDimension = uavDimension;

      switch (uavDimension)
      {
      case D3D12_UAV_DIMENSION_BUFFER:
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = 1;
        break;
      case D3D12_UAV_DIMENSION_TEXTURE1D:
        uavDesc.Texture1D.MipSlice = 0;
        break;
      case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
        uavDesc.Texture1DArray.MipSlice = 0;
        uavDesc.Texture1DArray.ArraySize = 0;
        uavDesc.Texture1DArray.FirstArraySlice = 0;
        break;
      case D3D12_UAV_DIMENSION_TEXTURE2D:
        uavDesc.Texture2D.MipSlice = 0;
        uavDesc.Texture2D.PlaneSlice = 0;
        break;
      case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
        uavDesc.Texture2DArray.MipSlice = 0;
        uavDesc.Texture2DArray.ArraySize = 0;
        uavDesc.Texture2DArray.FirstArraySlice = 0;
        uavDesc.Texture2DArray.PlaneSlice = 0;
        break;
      case D3D12_UAV_DIMENSION_TEXTURE3D:
        uavDesc.Texture3D.MipSlice = 0;
        uavDesc.Texture3D.FirstWSlice = 0;
        uavDesc.Texture3D.WSize = 1;
        break;
      default: ASSERT(false);
      }

      descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      ourDevice->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, descriptor.myCpuHandle);
      myUAVNullDescriptors[i] = descriptor;
    }
  }

  DescriptorDX12 descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
  cbvDesc.BufferLocation = 0ull;
  cbvDesc.SizeInBytes = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
  ourDevice->CreateConstantBufferView(&cbvDesc, descriptor.myCpuHandle);
  myCBVNullDescriptor = descriptor;

  descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  D3D12_SAMPLER_DESC samplerDesc = {};
  samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
  samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
  samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
  for (uint i = 0u; i < 4u; ++i)
    samplerDesc.BorderColor[i] = 0.0f;
  samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
  samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
  samplerDesc.MaxAnisotropy = 0u;
  samplerDesc.MaxLOD = 1.0f;
  samplerDesc.MinLOD = 0.0f;
  samplerDesc.MipLODBias = 0.0f;
  ourDevice->CreateSampler(&samplerDesc, descriptor.myCpuHandle);
  mySamplerNullDescriptor = descriptor;
}
//---------------------------------------------------------------------------//
void RenderCore_PlatformDX12::Shutdown()
{
  myPipelineStateCache.Clear();

  for (uint i = 0u; i < ARRAY_LENGTH(myStaticDescriptorAllocators); ++i)
    myStaticDescriptorAllocators[i].reset();

  for (uint i = 0u; i < (uint)GpuMemoryType::NUM; ++i)
    for (uint k = 0u; k < (uint)CpuMemoryAccessType::NUM; ++k)
      myGpuMemoryAllocators[i][k].reset();

  for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
    ourCommandAllocatorPools[i].reset();

  myShaderVisibleDescriptorHeap.reset();

  myRootSignature.reset();

  ourDevice.Reset();
}
//---------------------------------------------------------------------------//
RenderCore_PlatformDX12::~RenderCore_PlatformDX12()
{
  Shutdown();
}
//---------------------------------------------------------------------------//
ID3D12CommandAllocator* RenderCore_PlatformDX12::GetCommandAllocator(CommandListType aCmdListType)
{
  return ourCommandAllocatorPools[(uint)aCmdListType]->GetNewAllocator();
}
//---------------------------------------------------------------------------//
void RenderCore_PlatformDX12::ReleaseCommandAllocator(ID3D12CommandAllocator* anAllocator, uint64 aFenceVal)
{
  CommandListType type = CommandQueue::GetCommandListType(aFenceVal);
  ourCommandAllocatorPools[(uint)type]->ReleaseAllocator(anAllocator, aFenceVal);
}
//---------------------------------------------------------------------------//
DescriptorDX12 RenderCore_PlatformDX12::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, const char* aDebugName /* = nullptr*/)
{
  return myStaticDescriptorAllocators[(uint)aHeapType]->AllocateDescriptor(aDebugName);
}
//---------------------------------------------------------------------------//
DescriptorDX12 RenderCore_PlatformDX12::AllocateShaderVisibleDescriptorForGlobalResource(GlobalResourceType aGlobalResourceType, const char* aDebugName)
{
  return myShaderVisibleDescriptorHeap->AllocateDescriptor(aGlobalResourceType, aDebugName);
}
//---------------------------------------------------------------------------//
void RenderCore_PlatformDX12::FreeDescriptor(const DescriptorDX12& aDescriptor)
{
  ASSERT(aDescriptor.myIsManagedByAllocator);

  if (aDescriptor.myIsShaderVisible)
    myShaderVisibleDescriptorHeap->FreeDescriptorAfterFrameDone(aDescriptor);
  else
    myStaticDescriptorAllocators[static_cast<uint>(aDescriptor.myHeapType)]->FreeDescriptor(aDescriptor);
}
//---------------------------------------------------------------------------//
CommandQueueDX12* RenderCore_PlatformDX12::GetCommandQueueDX12(CommandListType aCommandListType)
{
  return static_cast<CommandQueueDX12*>(RenderCore::GetCommandQueue(aCommandListType));
}
//---------------------------------------------------------------------------//
GpuMemoryAllocationDX12 RenderCore_PlatformDX12::AllocateGpuMemory(GpuMemoryType aType, CpuMemoryAccessType anAccessType, uint64 aSize, uint anAlignment, const char* aDebugName /*= nullptr*/)
{
  return myGpuMemoryAllocators[(uint)aType][(uint)anAccessType]->Allocate(aSize, anAlignment, aDebugName);
}
//---------------------------------------------------------------------------//
void RenderCore_PlatformDX12::ReleaseGpuMemory(GpuMemoryAllocationDX12& anAllocation)
{
  GpuMemoryType type = Adapter::ResolveGpuMemoryType(anAllocation.myHeap->GetDesc().Flags);
  CpuMemoryAccessType accessType = Adapter::ResolveGpuMemoryAccessType(anAllocation.myHeap->GetDesc().Properties.Type);
  myGpuMemoryAllocators[(uint)type][(uint)accessType]->Free(anAllocation);
}
//---------------------------------------------------------------------------//
RenderOutput* RenderCore_PlatformDX12::CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
{
  return new RenderOutputDX12(aNativeInstanceHandle, someWindowParams);
}
//---------------------------------------------------------------------------//
ShaderCompiler* RenderCore_PlatformDX12::CreateShaderCompiler()
{
  return new ShaderCompilerDX12();
}
//---------------------------------------------------------------------------//
Shader* RenderCore_PlatformDX12::CreateShader()
{
  return new ShaderDX12();
}
//---------------------------------------------------------------------------//
ShaderPipeline* RenderCore_PlatformDX12::CreateShaderPipeline()
{
  return new ShaderPipelineDX12();
}
//---------------------------------------------------------------------------//
Texture* RenderCore_PlatformDX12::CreateTexture()
{
  return new TextureDX12();
}
//---------------------------------------------------------------------------//
GpuBuffer* RenderCore_PlatformDX12::CreateBuffer()
{
  return new GpuBufferDX12();
}
//---------------------------------------------------------------------------//
TextureSampler* RenderCore_PlatformDX12::CreateTextureSampler(const TextureSamplerProperties& someProperties)
{
  return new TextureSamplerDX12(someProperties);
}
//---------------------------------------------------------------------------//
CommandList* RenderCore_PlatformDX12::CreateCommandList(CommandListType aType)
{
  return new CommandListDX12(aType);
}
//---------------------------------------------------------------------------//
CommandQueue* RenderCore_PlatformDX12::CreateCommandQueue(CommandListType aType)
{
  return new CommandQueueDX12(aType);
}
//---------------------------------------------------------------------------//
TextureView* RenderCore_PlatformDX12::CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* /*aDebugName*/ /* = nullptr */)
{
  return new TextureViewDX12(aTexture, someProperties);
}
//---------------------------------------------------------------------------//
GpuBufferView* RenderCore_PlatformDX12::CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* /*aDebugName*/ /* = nullptr */)
{
  return new GpuBufferViewDX12(aBuffer, someProperties);
}
//---------------------------------------------------------------------------//
RtAccelerationStructure* RenderCore_PlatformDX12::CreateRtBottomLevelAccelerationStructure(const RtAccelerationStructureGeometryData* someGeometries, uint aNumGeometries, uint aSomeFlags, const char* aName)
{
  return new RtAccelerationStructureDX12(someGeometries, aNumGeometries, aSomeFlags, aName);
}
//---------------------------------------------------------------------------//
RtAccelerationStructure* RenderCore_PlatformDX12::CreateRtTopLevelAccelerationStructure(const RtAccelerationStructureInstanceData* someInstances, uint aNumInstances, uint someFlags, const char* aName)
{
  return new RtAccelerationStructureDX12(someInstances, aNumInstances, someFlags, aName);
}
//---------------------------------------------------------------------------//
RtPipelineState* RenderCore_PlatformDX12::CreateRtPipelineState(const RtPipelineStateProperties& someProps)
{
  return new RtPipelineStateDX12(someProps);
}
//---------------------------------------------------------------------------//
GpuQueryHeap* RenderCore_PlatformDX12::CreateQueryHeap(GpuQueryType aType, uint aNumQueries)
{
  return new GpuQueryHeapDX12(aType, aNumQueries);
}
//---------------------------------------------------------------------------//
uint RenderCore_PlatformDX12::GetQueryTypeDataSize(GpuQueryType aType)
{
  switch (aType)
  {
  case GpuQueryType::TIMESTAMP: return sizeof(uint64);
  case GpuQueryType::OCCLUSION: return sizeof(uint64);
  case GpuQueryType::NUM:
  default: ASSERT(false); return sizeof(uint64);
  }
}
//---------------------------------------------------------------------------//
float64 RenderCore_PlatformDX12::GetGpuTicksToMsFactor(CommandListType aCommandListType)
{
  uint64 timestampFrequency = 1u;
  ASSERT_HRESULT(GetCommandQueueDX12(aCommandListType)->myQueue->GetTimestampFrequency(&timestampFrequency));
  return 1000.0f / timestampFrequency;
}
//---------------------------------------------------------------------------//
Microsoft::WRL::ComPtr<IDXGISwapChain> RenderCore_PlatformDX12::CreateSwapChain(const DXGI_SWAP_CHAIN_DESC& aSwapChainDesc)
{
  Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
  ASSERT_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

  DXGI_SWAP_CHAIN_DESC swapChainDesc = aSwapChainDesc;

  Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
  ASSERT_HRESULT(dxgiFactory->CreateSwapChain(GetCommandQueueDX12(CommandListType::Graphics)->myQueue.Get(), &swapChainDesc, &swapChain));
  return swapChain;
}
//---------------------------------------------------------------------------//
const DescriptorDX12& RenderCore_PlatformDX12::GetNullDescriptor(D3D12_DESCRIPTOR_RANGE_TYPE aType, GpuResourceDimension aResouceDimension) const
{
  switch (aType)
  {
  case D3D12_DESCRIPTOR_RANGE_TYPE_SRV: return mySRVNullDescriptors[(uint)aResouceDimension];
  case D3D12_DESCRIPTOR_RANGE_TYPE_UAV: return myUAVNullDescriptors[(uint)aResouceDimension];
  case D3D12_DESCRIPTOR_RANGE_TYPE_CBV: return myCBVNullDescriptor;
  case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER: return mySamplerNullDescriptor;
  default: ASSERT(false); return mySamplerNullDescriptor;
  }
}
//---------------------------------------------------------------------------//

#endif