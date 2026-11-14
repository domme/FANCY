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
    kFgMaxNumResourceVersions = kFgMaxNumResources * 2u,
    kFgMaxNumReadsPerBuild = 256u,
    kFgMaxNumResourcesPerTask = 8u,
    kFgMaxNumBarriersPerTask = 16u,
    kFgMaxNumSubresources = 1024u,
    kFgMaxNumReadersPerSubresource = 4u,
    kFgMaxNumResourceViewsPerFrame = kFgMaxNumResources * 8u,
    kFgTaskMemorySize = 64u * 1024u,
  };
  //---------------------------------------------------------------------------//
  /// Handle to a range of subresources of one version of a registered resource, only valid in the generation (frame) of the builder it was
  /// created in. A version always describes the whole resource, the range only selects what is accessed.
  struct FgResourceHandle {
    SubresourceRange mySubresources;
    uint myVersionIndex = kFgInvalidIndex;  // Into FrameGraphBuilder's resource versions
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
    GpuResource * myResource = nullptr;  // Null for the backbuffer, whose swap chain image changes every frame
    SubresourceRange mySubresources;     // All subresources of the resource
    // The engine handle the resource was registered with. Invalid for the backbuffer.
    uint myHandleIndex = kFgInvalidIndex;
    uint myHandleGeneration = 0u;
    bool myIsBackBuffer = false;
  };
  //---------------------------------------------------------------------------//
  /// The contents of a whole resource after a write. Versions of a resource form a linear chain, version 0 is the state before the graph.
  struct FrameGraphResourceVersion {
    SubresourceRange myWrittenRange;  // The subresources changed compared to the previous version. Empty for version 0.
    uint myBaseIndex = kFgInvalidIndex;
    uint myVersion = 0u;  // Number within the resource, for display
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
    uint myWrittenVersionIndex = kFgInvalidIndex;  // The version a write created
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
