#include "fancy_core_precompile.h"
#include "ResourceBarriersVk.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  ResourceBarrierInfoVk ResourceBarriersVk::GetBarrierInfoVk(GpuResourceState aState, GpuResourceCategory aResourceCategory)
  {
    const bool isImage = aResourceCategory == GpuResourceCategory::TEXTURE;
    VkImageLayout shaderResourceReadImageLayout = isImage ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;

    switch(aState)
    {
    case GpuResourceState::COMMON: break;
    case GpuResourceState::READ_INDIRECT_ARGUMENT: 
      return { VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
        VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED };
    case GpuResourceState::READ_VERTEX_BUFFER:
      return { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED };
    case GpuResourceState::READ_INDEX_BUFFER:
      return { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_ACCESS_INDEX_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED };

    case GpuResourceState::READ_VERTEX_SHADER_CONSTANT_BUFFER:
      return { VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        VK_ACCESS_UNIFORM_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED };
    case GpuResourceState::READ_VERTEX_SHADER_RESOURCE:
      return { VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        shaderResourceReadImageLayout };

    case GpuResourceState::READ_PIXEL_SHADER_CONSTANT_BUFFER:
      return { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_ACCESS_UNIFORM_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED };
    case GpuResourceState::READ_PIXEL_SHADER_RESOURCE:
      return { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        shaderResourceReadImageLayout };

    case GpuResourceState::READ_COMPUTE_SHADER_CONSTANT_BUFFER:
      return { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_ACCESS_UNIFORM_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED };
    case GpuResourceState::READ_COMPUTE_SHADER_RESOURCE:
      return { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        shaderResourceReadImageLayout };

    case GpuResourceState::READ_ANY_SHADER_CONSTANT_BUFFER:
      return { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_ACCESS_UNIFORM_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED };
    case GpuResourceState::READ_ANY_SHADER_RESOURCE:
      return { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        shaderResourceReadImageLayout };

    case GpuResourceState::READ_COPY_SOURCE:
      return { VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL };

    case GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH:
      if (isImage)
      {
        
      }
      else
      {
        
      }
      break;
    case GpuResourceState::READ_DEPTH: break;
    case GpuResourceState::READ_PRESENT: break;
    case GpuResourceState::WRITE_VERTEX_SHADER_UAV: break;
    case GpuResourceState::WRITE_PIXEL_SHADER_UAV: break;
    case GpuResourceState::WRITE_COMPUTE_SHADER_UAV: break;
    case GpuResourceState::WRITE_ANY_SHADER_UAV: break;
    case GpuResourceState::WRITE_RENDER_TARGET: break;
    case GpuResourceState::WRITE_COPY_DEST: break;
    case GpuResourceState::WRITE_DEPTH: break;
    case GpuResourceState::UNKNOWN: break;
    case GpuResourceState::NUM: break;
    default: ;
    }
  }
}
