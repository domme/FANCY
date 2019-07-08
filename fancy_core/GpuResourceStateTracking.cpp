#include "fancy_core_precompile.h"
#include "GpuResourceStateTracking.h"

namespace Fancy
{
  bool GpuResourceStateTracking::QueueCanTransitionFrom(CommandListType aQueue, CommandListType aSrcQueue, GpuResourceUsageState aSrcState)
  {
    if (aSrcQueue == aQueue)
      return true;

    const bool copy = aQueue == CommandListType::DMA;
    const bool compute = aQueue == CommandListType::Compute;
    const bool graphics = aQueue == CommandListType::Graphics;
    const bool queueIsLessSpecificOrEqual = aQueue <= aSrcQueue;

    switch (aSrcState)
    {
      case GpuResourceUsageState::COMMON: return true; // This should mean "idle" more or less so it could be supported on any queue-type
      case GpuResourceUsageState::READ_INDIRECT_ARGUMENT: return (graphics || compute) && queueIsLessSpecificOrEqual;
      case GpuResourceUsageState::READ_VERTEX_BUFFER: return graphics;
      case GpuResourceUsageState::READ_INDEX_BUFFER: return graphics;
      case GpuResourceUsageState::READ_VERTEX_SHADER_CONSTANT_BUFFER: return graphics;
      case GpuResourceUsageState::READ_VERTEX_SHADER_RESOURCE: return graphics;
      case GpuResourceUsageState::READ_PIXEL_SHADER_CONSTANT_BUFFER: return graphics;
      case GpuResourceUsageState::READ_PIXEL_SHADER_RESOURCE: return graphics;
      case GpuResourceUsageState::READ_COMPUTE_SHADER_CONSTANT_BUFFER: return graphics || compute;
      case GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE: return graphics || compute;
      case GpuResourceUsageState::READ_ANY_SHADER_CONSTANT_BUFFER: return (graphics || compute) && queueIsLessSpecificOrEqual;
      case GpuResourceUsageState::READ_ANY_SHADER_RESOURCE:  return (graphics || compute) && queueIsLessSpecificOrEqual;
      case GpuResourceUsageState::READ_COPY_SOURCE: return graphics || compute;
      case GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH: return (graphics || compute) && queueIsLessSpecificOrEqual;
      case GpuResourceUsageState::READ_DEPTH: return graphics;
      case GpuResourceUsageState::READ_PRESENT: return graphics;
      case GpuResourceUsageState::WRITE_VERTEX_SHADER_UAV: return graphics;
      case GpuResourceUsageState::WRITE_PIXEL_SHADER_UAV: return graphics;
      case GpuResourceUsageState::WRITE_COMPUTE_SHADER_UAV: return graphics || compute;
      case GpuResourceUsageState::WRITE_ANY_SHADER_UAV: return (graphics || compute) && queueIsLessSpecificOrEqual;
      case GpuResourceUsageState::WRITE_RENDER_TARGET: return graphics;
      case GpuResourceUsageState::WRITE_COPY_DEST: return graphics || compute;
      case GpuResourceUsageState::WRITE_DEPTH: return graphics;
      case GpuResourceUsageState::UNKNOWN:
      case GpuResourceUsageState::NUM:
      default: ASSERT(false); return false;
    }
  }
//---------------------------------------------------------------------------//
  bool GpuResourceStateTracking::IsBarrierNeeded(CommandListType aSrcQueue, GpuResourceUsageState aSrcState, CommandListType aDstQueue, GpuResourceUsageState aDstState)
  {
    if (aSrcQueue == aDstQueue && aSrcState == aDstState)
      return false;

    const bool srcIsRead = aSrcState >= GpuResourceUsageState::FIRST_READ_STATE && aSrcState <= GpuResourceUsageState::LAST_READ_STATE;
    const bool dstIsRead = aDstState >= GpuResourceUsageState::FIRST_READ_STATE && aDstState <= GpuResourceUsageState::LAST_READ_STATE;

    // Read-Write or Write-Read transitions always need a barrier
    if (srcIsRead != dstIsRead)
      return true;

    // Write-Write transitions also need a barrier because most write-states are also read-states (UAVs, Depthbuffer for depth-testing,...)
    if (!srcIsRead && !dstIsRead)
      return true;

    // Read-read only need a barrier if the src-state doesn't already cover the dst-state. 
    // We don't allow read-read transitions to less specific states, only transitions to more general states are allowed and those don't need a barrier.
    // However, some cases do require a barrier:
    // 1) From/To copy-states (maybe)
    // 2) From/To indirect argument (maybe)
    // 3) From/To depth
    // 4) From/To present
    if (aSrcState == GpuResourceUsageState::READ_COPY_SOURCE || aDstState == GpuResourceUsageState::READ_COPY_SOURCE)
      return true;
    if (aSrcState == GpuResourceUsageState::READ_INDIRECT_ARGUMENT || aDstState == GpuResourceUsageState::READ_INDIRECT_ARGUMENT)
      return true;
    if (aSrcState == GpuResourceUsageState::READ_PRESENT || aDstState == GpuResourceUsageState::READ_PRESENT)
      return true;

    // In all other cases, no actual barrier is required. States can 
    return false;
  }
//---------------------------------------------------------------------------//
  GpuResourceUsageState GpuResourceStateTracking::CompareGetMoreGeneralState(GpuResourceUsageState aState1, GpuResourceUsageState aState2)
  {
    if (aState1 == aState2)
      return aState1;

    auto StateIsContainedIn = [](GpuResourceUsageState aLowerState, GpuResourceUsageState aHigherState)
    {
        switch (aLowerState) 
      { 
        case GpuResourceUsageState::COMMON: 
          return false;

        case GpuResourceUsageState::READ_INDIRECT_ARGUMENT: 
          return false;

        case GpuResourceUsageState::READ_VERTEX_BUFFER: 
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH;

        case GpuResourceUsageState::READ_INDEX_BUFFER:
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH;

        case GpuResourceUsageState::READ_VERTEX_SHADER_CONSTANT_BUFFER:
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH
            || aHigherState == GpuResourceUsageState::READ_ANY_SHADER_CONSTANT_BUFFER;

        case GpuResourceUsageState::READ_VERTEX_SHADER_RESOURCE:
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH
            || aHigherState == GpuResourceUsageState::READ_ANY_SHADER_RESOURCE;

        case GpuResourceUsageState::READ_PIXEL_SHADER_CONSTANT_BUFFER: 
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH
            || aHigherState == GpuResourceUsageState::READ_ANY_SHADER_CONSTANT_BUFFER;

        case GpuResourceUsageState::READ_PIXEL_SHADER_RESOURCE:
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH
            || aHigherState == GpuResourceUsageState::READ_ANY_SHADER_RESOURCE;

        case GpuResourceUsageState::READ_COMPUTE_SHADER_CONSTANT_BUFFER:
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH
            || aHigherState == GpuResourceUsageState::READ_ANY_SHADER_CONSTANT_BUFFER;

        case GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE:
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH
            || aHigherState == GpuResourceUsageState::READ_ANY_SHADER_RESOURCE;

        case GpuResourceUsageState::READ_ANY_SHADER_CONSTANT_BUFFER:
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH
            || aHigherState == GpuResourceUsageState::READ_ANY_SHADER_CONSTANT_BUFFER;

        case GpuResourceUsageState::READ_ANY_SHADER_RESOURCE:
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH;

        case GpuResourceUsageState::READ_COPY_SOURCE:
          return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH;

        case GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH:
          return false;

        case GpuResourceUsageState::READ_DEPTH:
          return false;

        case GpuResourceUsageState::READ_PRESENT: break;
        case GpuResourceUsageState::WRITE_VERTEX_SHADER_UAV: break;
        case GpuResourceUsageState::WRITE_PIXEL_SHADER_UAV: break;
        case GpuResourceUsageState::WRITE_COMPUTE_SHADER_UAV: break;
        case GpuResourceUsageState::WRITE_ANY_SHADER_UAV: break;
        case GpuResourceUsageState::WRITE_RENDER_TARGET: break;
        case GpuResourceUsageState::WRITE_COPY_DEST: break;
        case GpuResourceUsageState::WRITE_DEPTH: break;
        case GpuResourceUsageState::UNKNOWN: break;
        case GpuResourceUsageState::NUM: break;


      case GpuResourceUsageState::COMMON:
        return false;
      case GpuResourceUsageState::READ_INDIRECT_ARGUMENT:
        return false;
      case GpuResourceUsageState::READ_VERTEX_BUFFER:
      case GpuResourceUsageState::READ_INDEX_BUFFER:
        case GpuResourceUsageState::READ_ANY_SHADER_CONSTANT_BUFFER:
        
      case GpuResourceUsageState::READ_VERTEX_SHADER_RESOURCE:
      case GpuResourceUsageState::READ_PIXEL_SHADER_RESOURCE:
      case GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE:
        return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH
            || aHigherState == GpuResourceUsageState::READ_ANY_SHADER_RESOURCE;
      case GpuResourceUsageState::READ_VERTEX_SHADER_CONSTANT_BUFFER:
      case GpuResourceUsageState::READ_PIXEL_SHADER_CONSTANT_BUFFER:
      case GpuResourceUsageState::READ_COMPUTE_SHADER_CONSTANT_BUFFER:
        return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH
          || aHigherState == GpuResourceUsageState::READ_ANY_SHADER_CONSTANT_BUFFER;
      case GpuResourceUsageState::READ_ANY_SHADER_RESOURCE:
        return aHigherState == GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH;
      case GpuResourceUsageState::READ_COPY_SOURCE: break;
      case 
      case GpuResourceUsageState::READ_DEPTH: break;
      case GpuResourceUsageState::READ_PRESENT: break;
      case GpuResourceUsageState::LAST_READ_STATE: break;
      case GpuResourceUsageState::WRITE_VERTEX_SHADER_UAV: break;
      case GpuResourceUsageState::FIRST_WRITE_STATE: break;
      case GpuResourceUsageState::WRITE_PIXEL_SHADER_UAV: break;
      case GpuResourceUsageState::WRITE_COMPUTE_SHADER_UAV: break;
      case GpuResourceUsageState::WRITE_ANY_SHADER_UAV: break;
      case GpuResourceUsageState::WRITE_RENDER_TARGET: break;
      case GpuResourceUsageState::WRITE_COPY_DEST: break;
      case GpuResourceUsageState::WRITE_DEPTH: break;
      case GpuResourceUsageState::LAST_WRITE_STATE: break;
      case GpuResourceUsageState::UNKNOWN: break;
      case GpuResourceUsageState::NUM: break;
      default: ;
      }
};
  }
//---------------------------------------------------------------------------//




}