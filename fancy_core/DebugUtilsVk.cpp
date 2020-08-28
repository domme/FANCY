#include "fancy_core_precompile.h"
#include "DebugUtilsVk.h"

namespace Fancy
{
  namespace DebugUtilsVk
  {
//---------------------------------------------------------------------------//
    eastl::string AccessMaskToString(VkAccessFlags anAccessMask)
    {
      if (anAccessMask == 0u)
        return "none";

      eastl::string str;

      if (anAccessMask & VK_ACCESS_INDIRECT_COMMAND_READ_BIT)
        str += "indirect command read|";
      if (anAccessMask & VK_ACCESS_INDEX_READ_BIT)
        str += "index read|";
      if (anAccessMask & VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)
        str += "vertex attribute read|";
      if (anAccessMask & VK_ACCESS_UNIFORM_READ_BIT)
        str += "uniform read|";
      if (anAccessMask & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)
        str += "input attachment read|";
      if (anAccessMask & VK_ACCESS_SHADER_READ_BIT)
        str += "shader read|";
      if (anAccessMask & VK_ACCESS_SHADER_WRITE_BIT)
        str += "shader write|";
      if (anAccessMask & VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
        str += "color attachment read|";
      if (anAccessMask & VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
        str += "color attachment write|";
      if (anAccessMask & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT)
        str += "depth stencil read|";
      if (anAccessMask & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
        str += "depth stencil write|";
      if (anAccessMask & VK_ACCESS_TRANSFER_READ_BIT)
        str += "transfer read|";
      if (anAccessMask & VK_ACCESS_TRANSFER_WRITE_BIT)
        str += "transfer write|";
      if (anAccessMask & VK_ACCESS_HOST_READ_BIT)
        str += "host read|";
      if (anAccessMask & VK_ACCESS_HOST_WRITE_BIT)
        str += "host write|";
      if (anAccessMask & VK_ACCESS_MEMORY_READ_BIT)
        str += "memory read|";
      if (anAccessMask & VK_ACCESS_MEMORY_WRITE_BIT)
        str += "memory write|";
      if (anAccessMask & VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT)
        str += "transform feedback write|";
      if (anAccessMask & VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT)
        str += "transform feedback counter read|";
      if (anAccessMask & VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT)
        str += "transform feedback counter write|";
      if (anAccessMask & VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT)
        str += "conditional rendering read|";
      if (anAccessMask & VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT)
        str += "color attachment read noncoherent|";
      if (anAccessMask & VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR)
        str += "acceleration structure read|";
      if (anAccessMask & VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR)
        str += "acceleration structure write|";
      if (anAccessMask & VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV)
        str += "shading rate image read|";
      if (anAccessMask & VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT)
        str += "fragment density map read|";
      if (anAccessMask & VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV)
        str += "command preprocess read|";
      if (anAccessMask & VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV)
        str += "command preprocess write|";

      if (!str.empty() && str.back() == '|')
        str.erase(str.size() - 1);

      return str;
    }
//---------------------------------------------------------------------------//
    eastl::string PipelineStageMaskToString(VkPipelineStageFlags aPipelineStageMask)
    {
      if (aPipelineStageMask == 0u)
        return "none";

      eastl::string str;

      if (aPipelineStageMask & VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
        str += "top of pipe|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT)
        str += "draw indirect|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_VERTEX_INPUT_BIT)
        str += "vertex input|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_VERTEX_SHADER_BIT)
        str += "vertex shader|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT)
        str += "tessellation control shader|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT)
        str += "tessellation eval shader|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT)
        str += "geometry shader|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
        str += "fragment shader|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
        str += "early fragment tests|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)
        str += "late fragment tests|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
        str += "color attachment output|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
        str += "compute shader|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_TRANSFER_BIT)
        str += "transfer|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
        str += "bottom of pipe|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_HOST_BIT)
        str += "host|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT)
        str += "all graphics|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
        str += "all commands|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT)
        str += "transform feedback|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT)
        str += "conditional rendering|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR)
        str += "ray tracing shader|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR)
        str += "acceleration structure build|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV)
        str += "shading rate image|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV)
        str += "task shader|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV)
        str += "mesh shader|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT)
        str += "fragment density process|";
      if (aPipelineStageMask & VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV)
        str += "command preprocess|";

      if (!str.empty() && str.back() == '|')
        str.erase(str.size() - 1);

      return str;
    }
//---------------------------------------------------------------------------//
    eastl::string ImageLayoutToString(VkImageLayout anImageLayout)
    {
      switch (anImageLayout)
      {
        case VK_IMAGE_LAYOUT_UNDEFINED: return "undefined";
        case VK_IMAGE_LAYOUT_GENERAL: return "general";
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return "color attachment";
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return "depth stencil write";
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return "depth stencil read";
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return "shader read";
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return "transfer src";
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return "transfer dst";
        case VK_IMAGE_LAYOUT_PREINITIALIZED: return "preinitialized";
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: return "depth read stencil write";
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: return "depth write stencil read";
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return "depth write";
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL: return "depth read";
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL: return "stencil write";
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL: return "stencil read";
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return "present";
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR: return "shared present";
        case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV: return "shading rate";
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT: return "fragment density map";
        default: ASSERT(false); return "";
      }
    }
//---------------------------------------------------------------------------//
  }
}