#pragma once

#include "FrameGraphTypes.h"
#include "Common/LinearAllocator.h"
#include "Rendering/GpuResourceView.h"
#include "Rendering/RendererPrerequisites.h"
#include "Rendering/ResourceHandle.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  class RenderOutput;
  class TextureView;
  class GpuBufferView;
  //---------------------------------------------------------------------------//
  struct FrameGraphCompiledTask {
    void * myRenderData = nullptr;
    bool myAccessesBackBuffer = false;
    eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumBarriersPerTask > myBarriers;
  };
  //---------------------------------------------------------------------------//
  /// The kinds of views a resource can have in the compiled graph
  enum FgViewKind { FG_VIEW_KIND_SRV, FG_VIEW_KIND_UAV, FG_VIEW_KIND_RTV, FG_VIEW_KIND_DSV, FG_VIEW_KIND_READ_ONLY_DSV, FG_VIEW_KIND_NUM };
  //---------------------------------------------------------------------------//
  /// One view of a resource in the compiled graph, linked to the next view of the same resource
  struct FrameGraphCompiledView {
    SubresourceRange mySubresources;
    TextureViewHandle myTextureView;
    GpuBufferViewHandle myBufferView;
    FgViewKind myKind = FG_VIEW_KIND_NUM;  // TODO: Add the format once accesses can request one (e.g. a float and a uint view)
    uint myNextViewIndex = kFgInvalidIndex;
  };
  //---------------------------------------------------------------------------//
  /// One resource (FrameGraphResourceBase) in the compiled graph
  struct FrameGraphCompiledResource {
    uint myFirstViewIndex = kFgInvalidIndex;  // Into the compiled views
    uint myHandleIndex = kFgInvalidIndex;
    uint myHandleGeneration = 0u;
  };
  //---------------------------------------------------------------------------//
  /// Resolves a task's handles to concrete resources and views in its Compile().
  /// The compiled graph, including the tasks' render data, is reused as long as the declaration hash doesn't change, so a task's Compile() may only
  /// depend on its declared accesses: views of narrowed handles have to be derived from the declared ranges alone. Everything else that can change
  /// from frame to frame belongs in Execute(), which gets the current task instance.
  class FrameGraphCompileContext {
  public:
    FrameGraphCompileContext( FrameGraphBuilder * aBuilder, uint aTaskIndex );

    TextureView * GetRtv( FgResourceHandle aHandle ) const;
    TextureView * GetDsv( FgResourceHandle aHandle ) const;
    TextureView * GetSrv( FgResourceHandle aHandle ) const;
    TextureView * GetUav( FgResourceHandle aHandle ) const;
    GpuBufferView * GetBufferSrv( FgResourceHandle aHandle ) const;
    GpuBufferView * GetBufferUav( FgResourceHandle aHandle ) const;
    GpuResource * GetResource( FgResourceHandle aHandle ) const;

  private:
    const FrameGraphTaskAccess * FindAccess( FgResourceHandle aHandle ) const;
    GpuResourceView * GetView( FgResourceHandle aHandle, GpuResourceViewType aViewType, bool anIsTexture ) const;

    FrameGraphBuilder * myBuilder;
    uint myTaskIndex;
  };
  //---------------------------------------------------------------------------//
  /// Render graph describing a GPU frame as a DAG of tasks, derived from the tasks' resource accesses.
  /// Declared every frame, compiled only when its structure changes.
  class FrameGraphBuilder {
    friend class FrameGraphCompileContext;

  public:
    explicit FrameGraphBuilder( RenderOutput * aRenderOutput );
    ~FrameGraphBuilder();

    FgResourceHandle RegisterExisting( TextureHandle aTexture );
    FgResourceHandle RegisterExisting( GpuBufferHandle aBuffer );
    FgResourceHandle RegisterBackBuffer();

    /// Starts a new declaration: clears the setup data. Handles of the previous declaration become invalid. The build data is kept and reused
    /// by BuildFrameGraph() if the new declaration is equal.
    void ResetAndBeginSetup();

    /// Only clears the build data, so BuildFrameGraph() rebuilds the graph from the declaration of the last setup (e.g. in a frame without setup)
    void DiscardBuildResults();

    /// Returns the index that identifies the task in Read() and Write(), and the uninitialized memory for the task instance in aTaskMemoryOut (see
    /// FgAddTask() in FrameGraphTaskHelper.h). The order of the tasks follows from the data flow: a task executes after the producers of what it
    /// reads and before the tasks that overwrite it later. The order the tasks were added in only decides between tasks that are otherwise unordered.
    uint AddTask( const FgTaskCreateInfo & aCreateInfo, void ** aTaskMemoryOut );

    /// Only record the access of a task, at any time before BuildFrameGraph(). Reads the contents of the handle's version, which can be an old one:
    /// the task is then ordered before the tasks that overwrite the read subresources later.
    void Read( uint aTaskIndex, FgResourceHandle aHandle, ResourceAccess anAccess );

    /// Only records the access of a task. Writes the subresources of the handle on top of the handle's version and returns the new latest version of
    /// the resource (whole resource), which also contains the changes of the versions in between. Writing through an older version is allowed as long
    /// as none of the later versions changed the written subresources, otherwise this asserts and returns an invalid handle.
    FgResourceHandle Write( uint aTaskIndex, FgResourceHandle aHandle, ResourceAccess anAccess );

    bool BuildFrameGraph();

    void Execute();

    void ReleaseCompiledGraph();

    bool IsDeclarationBuilt() const;
    uint64 GetDeclarationHash() const;
    uint GetNumCompiles() const;
    const eastl::fixed_vector< FrameGraphTask, kFgMaxNumTasks > & GetTasks() const;
    const eastl::fixed_vector< FrameGraphResourceBase, kFgMaxNumResources > & GetResourceBases() const;
    const eastl::fixed_vector< FrameGraphResourceVersion, kFgMaxNumResourceVersions > & GetResourceVersions() const;
    const eastl::fixed_vector< uint, kFgMaxNumResourceVersions > & GetVersionProducerTasks() const;
    const eastl::fixed_vector< uint, kFgMaxNumResources > & GetLatestVersionIndices() const;
    const eastl::fixed_vector< FrameGraphDependency, kFgMaxNumTasks > & GetDependencies() const;
    const eastl::fixed_vector< uint, kFgMaxNumTasks > & GetTaskOrder() const;
    /// The registered resource. For the backbuffer, the current swap chain image.
    GpuResource * GetResource( uint aBaseIndex ) const;

  private:
    struct SetupData;
    struct BuildData;

    static bool Build( const SetupData & aSetupData, BuildData & aBuildData );
#if FANCY_RENDERER_DEBUG_FRAMEGRAPH
    static void VerifyReusedBuild( const SetupData & aSetupData, const BuildData & aBuildData );
#endif
    static uint GetSubresourceIndex( const SetupData & aSetupData, const BuildData & aBuildData, uint aBaseIndex,
                                     const SubresourceLocation & aLocation );

    FgResourceHandle RegisterResource( GpuResource * aResource, const char * aName, uint aHandleIndex, uint aHandleGeneration,
                                       bool anIsBackBuffer );
    void ValidateHandle( FgResourceHandle aHandle ) const;
    bool AddAccess( uint aTaskIndex, FgResourceHandle aHandle, ResourceAccess anAccess );

    uint64 ComputeDeclarationHash() const;
    void Compile();
    void CompileTask( uint aTaskIndex );
    GpuResourceView * GetCreateView( uint aBaseIndex, const SubresourceRange & aRange, ResourceAccess anAccess );
    const FrameGraphResourceBase & GetBase( FgResourceHandle aHandle ) const;
    uint GetBaseIndex( FgResourceHandle aHandle ) const;

    // Each phase only writes its own data and reads the data of the phases before it, so each can be redone on its own

    /// Written by Register...(), AddTask(), Read() and Write(). Cleared by ResetAndBeginSetup().
    struct SetupData {
      LinearAllocator< kFgTaskMemorySize > myTaskAllocator;
      eastl::fixed_vector< FrameGraphTask, kFgMaxNumTasks > myTasks;
      eastl::fixed_vector< FrameGraphResourceBase, kFgMaxNumResources > myResourceBases;
      eastl::fixed_vector< FrameGraphResourceVersion, kFgMaxNumResourceVersions > myResourceVersions;
      eastl::fixed_vector< uint, kFgMaxNumResources > myLatestVersionIndices;          // Per resource base
      eastl::fixed_vector< uint, kFgMaxNumResourceVersions > myVersionProducerTasks;  // Per version. Invalid for version 0.
      eastl::fixed_vector< uint, kFgMaxNumResourceVersions > myVersionNextIndices;    // Per version: the next version of the same resource
      uint myGenerationIndex = 0u;
    };

    /// Written by BuildFrameGraph(), only when the declaration hash changes. Cleared by DiscardBuildResults().
    struct BuildData {
      eastl::fixed_vector< uint, kFgMaxNumResources > myFirstSubresourceIndices;  // Per resource base, into the per-subresource arrays
      eastl::fixed_vector< FgSubresourceState, kFgMaxNumSubresources > mySubresourceStates;
      eastl::fixed_vector< FrameGraphDependency, kFgMaxNumTasks > myDependencies;
      eastl::fixed_vector< uint, kFgMaxNumTasks > myTaskOrder;
      uint64 myDeclarationHash = 0u;
      bool myHasResults = false;  // The results were built successfully from the declaration with myDeclarationHash
    };

    /// Written by Compile(), only when the declaration hash changes. Cleared by ReleaseCompiledGraph().
    struct CompiledData {
      LinearAllocator< kFgTaskMemorySize > myRenderDataAllocator;
      eastl::fixed_vector< FrameGraphCompiledTask, kFgMaxNumTasks > myTasks;
      eastl::fixed_vector< FrameGraphCompiledResource, kFgMaxNumResources > myResources;
      eastl::fixed_vector< FrameGraphCompiledView, kFgMaxNumResourceViewsPerFrame, false > myViews;
      eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumSubresources > myInitialAccesses;  // mySrcAccess is unused
      eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumSubresources > myFinalBarriers;
      uint64 myDeclarationHash = 0u;  // Of the declaration the results were compiled from
      bool myHasResults = false;
    };

    RenderOutput * myRenderOutput = nullptr;
    SetupData mySetupData;
    BuildData myBuildData;
    CompiledData myCompiledData;
    // The current declaration was built: it can't change anymore and can be executed. Written by BuildFrameGraph(), cleared by
    // ResetAndBeginSetup() and DiscardBuildResults(), so it belongs to none of the phases.
    bool myIsDeclarationBuilt = false;
    uint myNumCompiles = 0u;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy
