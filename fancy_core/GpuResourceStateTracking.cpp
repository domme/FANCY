#include "fancy_core_precompile.h"
#include "GpuResourceHazardTracking.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  bool GpuResourceHazardTracking::QueueUnderstandsState(CommandListType aCurrQueue, CommandListType aQueue, GpuResourceState aState)
  {
    if (aQueue == aCurrQueue)
      return true;

    const bool copy = aCurrQueue == CommandListType::DMA;
    const bool compute = aCurrQueue == CommandListType::Compute;
    const bool graphics = aCurrQueue == CommandListType::Graphics;
    const bool queueIsLessSpecificOrEqual = aCurrQueue <= aQueue;

    switch (aState)
    {
      case GpuResourceState::READ_INDIRECT_ARGUMENT: return (graphics || compute) && queueIsLessSpecificOrEqual;
      case GpuResourceState::READ_VERTEX_BUFFER: return graphics;
      case GpuResourceState::READ_INDEX_BUFFER: return graphics;
      case GpuResourceState::READ_VERTEX_SHADER_CONSTANT_BUFFER: return graphics;
      case GpuResourceState::READ_VERTEX_SHADER_RESOURCE: return graphics;
      case GpuResourceState::READ_PIXEL_SHADER_CONSTANT_BUFFER: return graphics;
      case GpuResourceState::READ_PIXEL_SHADER_RESOURCE: return graphics;
      case GpuResourceState::READ_COMPUTE_SHADER_CONSTANT_BUFFER: return graphics || compute;
      case GpuResourceState::READ_COMPUTE_SHADER_RESOURCE: return graphics || compute;
      case GpuResourceState::READ_ANY_SHADER_CONSTANT_BUFFER: return (graphics || compute) && queueIsLessSpecificOrEqual;
      case GpuResourceState::READ_ANY_SHADER_RESOURCE:  return (graphics || compute) && queueIsLessSpecificOrEqual;
      case GpuResourceState::READ_COPY_SOURCE: return graphics || compute;
      case GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH: return (graphics || compute) && queueIsLessSpecificOrEqual;
      case GpuResourceState::READ_DEPTH: return graphics;
      case GpuResourceState::READ_PRESENT: return graphics;
      case GpuResourceState::WRITE_VERTEX_SHADER_UAV: return graphics;
      case GpuResourceState::WRITE_PIXEL_SHADER_UAV: return graphics;
      case GpuResourceState::WRITE_COMPUTE_SHADER_UAV: return graphics || compute;
      case GpuResourceState::WRITE_ANY_SHADER_UAV: return (graphics || compute) && queueIsLessSpecificOrEqual;
      case GpuResourceState::WRITE_RENDER_TARGET: return graphics;
      case GpuResourceState::WRITE_COPY_DEST: return graphics || compute;
      case GpuResourceState::WRITE_DEPTH: return graphics;
      case GpuResourceState::UNKNOWN:
      case GpuResourceState::NUM:
      default: ASSERT(false); return false;
    }
  }
//---------------------------------------------------------------------------//
  bool GpuResourceHazardTracking::QueueUnderstandsPartsOfState(CommandListType aCurrQueue, GpuResourceState aState)
  {
    if (aCurrQueue == CommandListType::Graphics)
      return true;

    const bool copy = aCurrQueue == CommandListType::DMA;
    const bool compute = aCurrQueue == CommandListType::Compute;

    switch (aState)
    {
      case GpuResourceState::READ_INDIRECT_ARGUMENT: return compute;
      case GpuResourceState::READ_VERTEX_BUFFER: return false;
      case GpuResourceState::READ_INDEX_BUFFER: return false;
      case GpuResourceState::READ_VERTEX_SHADER_CONSTANT_BUFFER: return false;
      case GpuResourceState::READ_VERTEX_SHADER_RESOURCE: return false;
      case GpuResourceState::READ_PIXEL_SHADER_CONSTANT_BUFFER: return false;
      case GpuResourceState::READ_PIXEL_SHADER_RESOURCE: return false;
      case GpuResourceState::READ_COMPUTE_SHADER_CONSTANT_BUFFER: return compute;
      case GpuResourceState::READ_COMPUTE_SHADER_RESOURCE: return compute;
      case GpuResourceState::READ_ANY_SHADER_CONSTANT_BUFFER: return compute;
      case GpuResourceState::READ_ANY_SHADER_RESOURCE:  return compute;
      case GpuResourceState::READ_COPY_SOURCE: return true;
      case GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH: return compute;
      case GpuResourceState::READ_DEPTH: return false;
      case GpuResourceState::READ_PRESENT: return false;
      case GpuResourceState::WRITE_VERTEX_SHADER_UAV: return false;
      case GpuResourceState::WRITE_PIXEL_SHADER_UAV: return false;
      case GpuResourceState::WRITE_COMPUTE_SHADER_UAV: return compute;
      case GpuResourceState::WRITE_ANY_SHADER_UAV: return compute;
      case GpuResourceState::WRITE_RENDER_TARGET: return false;
      case GpuResourceState::WRITE_COPY_DEST: return true;
      case GpuResourceState::WRITE_DEPTH: return false;
      case GpuResourceState::UNKNOWN:
      case GpuResourceState::NUM:
      default: ASSERT(false); return false;
    }
  }
//---------------------------------------------------------------------------//
  bool GpuResourceHazardTracking::StateIsContainedIn(GpuResourceState aLowerState, GpuResourceState aHigherState)
  {
    if (aLowerState == aHigherState)
      return true;

    switch (aLowerState)
    {
    case GpuResourceState::READ_INDIRECT_ARGUMENT:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH;

    case GpuResourceState::READ_VERTEX_BUFFER:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH;

    case GpuResourceState::READ_INDEX_BUFFER:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH;

    case GpuResourceState::READ_VERTEX_SHADER_CONSTANT_BUFFER:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH
        || aHigherState == GpuResourceState::READ_ANY_SHADER_CONSTANT_BUFFER;

    case GpuResourceState::READ_VERTEX_SHADER_RESOURCE:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH
        || aHigherState == GpuResourceState::READ_ANY_SHADER_RESOURCE;

    case GpuResourceState::READ_PIXEL_SHADER_CONSTANT_BUFFER:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH
        || aHigherState == GpuResourceState::READ_ANY_SHADER_CONSTANT_BUFFER;

    case GpuResourceState::READ_PIXEL_SHADER_RESOURCE:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH
        || aHigherState == GpuResourceState::READ_ANY_SHADER_RESOURCE;

    case GpuResourceState::READ_COMPUTE_SHADER_CONSTANT_BUFFER:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH
        || aHigherState == GpuResourceState::READ_ANY_SHADER_CONSTANT_BUFFER;

    case GpuResourceState::READ_COMPUTE_SHADER_RESOURCE:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH
        || aHigherState == GpuResourceState::READ_ANY_SHADER_RESOURCE;

    case GpuResourceState::READ_ANY_SHADER_CONSTANT_BUFFER:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH
        || aHigherState == GpuResourceState::READ_ANY_SHADER_CONSTANT_BUFFER;

    case GpuResourceState::READ_ANY_SHADER_RESOURCE:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH;

    case GpuResourceState::READ_COPY_SOURCE:
      return aHigherState == GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH;

    case GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH:
      return false;

    case GpuResourceState::READ_DEPTH:
      return false;

    case GpuResourceState::READ_PRESENT:
      return false;

    case GpuResourceState::WRITE_VERTEX_SHADER_UAV:
      return aHigherState == GpuResourceState::WRITE_ANY_SHADER_UAV;

    case GpuResourceState::WRITE_PIXEL_SHADER_UAV:
      return aHigherState == GpuResourceState::WRITE_ANY_SHADER_UAV;

    case GpuResourceState::WRITE_COMPUTE_SHADER_UAV:
      return aHigherState == GpuResourceState::WRITE_ANY_SHADER_UAV;

    case GpuResourceState::WRITE_ANY_SHADER_UAV:
      return false;

    case GpuResourceState::WRITE_RENDER_TARGET:
      return false;

    case GpuResourceState::WRITE_COPY_DEST:
      return false;

    case GpuResourceState::WRITE_DEPTH:
      return false;

    default: return false;
    }
  }
//---------------------------------------------------------------------------//
}