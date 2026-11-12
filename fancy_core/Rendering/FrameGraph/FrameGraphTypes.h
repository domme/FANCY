#pragma once

#include "Common/FancyCoreDefines.h"
#include "Rendering/RenderEnums.h"
#include "Rendering/TextureData.h"

#include "EASTL/fixed_vector.h"
#include "EASTL/fixed_string.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  class CommandList;
  class GpuResource;
  class FrameGraphBuilder;
  class FrameGraphCompileContext;
  //---------------------------------------------------------------------------//
  /// The kFgMaxNum... values are inline capacities that may grow on the heap, except kFgMaxNumResourceViewsPerFrame, which is a hard limit
  enum FrameGraphConstants : uint {
    kFgInvalidIndex = UINT_MAX,

    kFgMaxNumTasks = 1024u,
    kFgMaxNumResources = 1024u,
    kFgMaxNumResourcesPerTask = 8u,
    kFgMaxNumBarriersPerTask = 16u,
    kFgMaxNumSubresources = 1024u,
    kFgMaxNumReadersPerSubresource = 4u,
    kFgMaxNumResourceViewsPerFrame = kFgMaxNumResources * 8u,
    kFgTaskMemorySize = 64u * 1024u,
  };
  //---------------------------------------------------------------------------//
  /// Handle to a range of subresources of a registered resource, only valid in the generation (frame) of the builder it was created in
  struct FgResourceHandle {
    SubresourceRange mySubresources;
    uint myBaseIndex = kFgInvalidIndex;
    uint myFrameGraphGenerationIndex = 0u;

    bool IsValid() const;
    FgResourceHandle Mips( uint aFirstMip, uint aNumMips = 1u ) const;
    FgResourceHandle Planes( uint aFirstPlane, uint aNumPlanes = 1u ) const;
    FgResourceHandle ArraySlices( uint aFirstIndex, uint aNumIndices = 1u ) const;
  };
  //---------------------------------------------------------------------------//
  /// A resource registered in the builder (not owning)
  struct FrameGraphResourceBase {
    eastl::fixed_string< char, 64 > myName;
    GpuResource * myResource = nullptr;
    uint myFirstSubresourceState = kFgInvalidIndex;
    // The engine handle the resource was registered with. Invalid for the backbuffer.
    uint myHandleIndex = kFgInvalidIndex;
    uint myHandleGeneration = 0u;
  };
  //---------------------------------------------------------------------------//
  /// Hazard state of one subresource while BuildFrameGraph() derives the dependencies
  struct FgSubresourceState {
    uint myWriterTask = kFgInvalidIndex;                                        // Invalid if written before the graph
    eastl::fixed_vector< uint, kFgMaxNumReadersPerSubresource > myReaderTasks;  // Readers since the last write
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
  /// A barrier issued before a task executes, between two accesses that are both known at compile time
  struct FrameGraphBarrier {
    SubresourceRange mySubresources;
    uint myBaseIndex = kFgInvalidIndex;
    ResourceAccess mySrcAccess = RESOURCE_ACCESS_NONE;
    ResourceAccess myDstAccess = RESOURCE_ACCESS_NONE;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy
