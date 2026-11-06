#pragma once

#include "Common/FancyCoreDefines.h"
#include "Rendering/RenderEnums.h"

#include "EASTL/fixed_vector.h"
#include "EASTL/fixed_string.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  class CommandList;
  class GpuResource;
  class FrameGraphBuilder;
  class FrameGraphCompileContext;
  //---------------------------------------------------------------------------//
  enum FrameGraphConstants : uint {
    kFgInvalidIndex = UINT_MAX,

    kFgMaxNumTasks = 1024u,
    kFgMaxNumResources = 1024u,
    kFgMaxNumResourcesPerTask = 8u,
    kFgMaxNumConsumersPerResource = 16u,
    kFgMaxNumViewsPerResource = 8u,
    kFgTaskMemorySize = 64u * 1024u,
  };
  //---------------------------------------------------------------------------//
  /// Handle to one version of a resource (an index into FrameGraphBuilder::GetResources()), only valid in the generation (frame) of the builder
  /// it was created in
  struct FgResourceHandle {
    FgResourceHandle() : myResourceIndex( kFgInvalidIndex ), myFrameGraphGenerationIndex( 0u ) {}

    bool IsValid() const {
      return myResourceIndex != kFgInvalidIndex;
    }

    uint myResourceIndex;
    uint myFrameGraphGenerationIndex;
  };
  //---------------------------------------------------------------------------//
  /// The data that exists once per resource, shared by all of its versions (not owning)
  struct FrameGraphResourceBase {
    eastl::fixed_string< char, 64 > myName;
    GpuResource * myResource = nullptr;
    uint myLatestResourceIndex = kFgInvalidIndex;
    // The engine handle the resource was registered with. Invalid for the backbuffer.
    uint myHandleIndex = kFgInvalidIndex;
    uint myHandleGeneration = 0u;
  };
  //---------------------------------------------------------------------------//
  /// One version of a resource: produced once, consumed by any number of tasks
  struct FrameGraphResource {
    uint myBaseIndex = kFgInvalidIndex;
    uint myVersion = 0u;                    // 0 = registered, +1 for each write
    uint myProducerTask = kFgInvalidIndex;  // Invalid if produced before the graph
    uint myNextVersionResourceIndex = kFgInvalidIndex;
    eastl::fixed_vector< uint, kFgMaxNumConsumersPerResource > myConsumerTasks;
  };
  //---------------------------------------------------------------------------//
  struct FrameGraphTaskAccess {
    FgResourceHandle myHandle;
    ResourceAccess myAccess = RESOURCE_ACCESS_NONE;
  };
  //---------------------------------------------------------------------------//
  /// Type-erased calls of a task class' static functions
  typedef void ( *FrameGraphTaskExecuteFunction )( CommandList * aCommandList, const void * aTask, const void * someRenderData );
  typedef void ( *FrameGraphTaskCompileFunction )( const void * aTask, const FrameGraphCompileContext & aContext, void * someRenderData );
  //---------------------------------------------------------------------------//
  /// Everything the builder needs to know about a task class. Usually created with FgMakeTaskCreateInfo() (FrameGraphTaskHelper.h).
  /// Tasks need to be trivially destructible, they're never destructed.
  struct FgTaskCreateInfo {
    const char * myName = nullptr;
    FrameGraphTaskExecuteFunction myExecuteFunction = nullptr;
    FrameGraphTaskCompileFunction myCompileFunction = nullptr;
    CommandListType myQueueType = CommandListType::Graphics;
    uint myTaskSize = 0u;
    uint myTaskAlignment = 0u;
    uint myRenderDataSize = 0u;
    uint myRenderDataAlignment = 0u;
  };
  //---------------------------------------------------------------------------//
  /// A task instance added with FrameGraphBuilder::AddTask()
  struct FrameGraphTask {
    eastl::fixed_string< char, 64 > myName;
    CommandListType myQueueType = CommandListType::Graphics;
    void * myInstance = nullptr;
    uint myRenderDataSize = 0u;
    uint myRenderDataAlignment = 0u;
    FrameGraphTaskExecuteFunction myExecuteFunction = nullptr;
    FrameGraphTaskCompileFunction myCompileFunction = nullptr;

    eastl::fixed_vector< FrameGraphTaskAccess, kFgMaxNumResourcesPerTask > myAccesses;
  };
  //---------------------------------------------------------------------------//
  /// mySrcTask has to be executed before myDstTask
  struct FrameGraphDependency {
    uint mySrcTask = kFgInvalidIndex;
    uint myDstTask = kFgInvalidIndex;
  };
  //---------------------------------------------------------------------------//
  /// A transition issued before a task executes
  struct FrameGraphBarrier {
    uint myBaseIndex = kFgInvalidIndex;
    ResourceAccess myAccess = RESOURCE_ACCESS_NONE;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy
