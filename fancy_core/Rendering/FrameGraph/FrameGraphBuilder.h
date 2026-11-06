#pragma once

#include "FrameGraphTypes.h"
#include "Common/LinearAllocator.h"
#include "Rendering/GpuResourceView.h"
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
    eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumResourcesPerTask > myBarriers;
  };
  //---------------------------------------------------------------------------//
  /// The kinds of views a resource can have in the compiled graph
  enum FgViewKind { FG_VIEW_KIND_SRV, FG_VIEW_KIND_UAV, FG_VIEW_KIND_RTV, FG_VIEW_KIND_DSV, FG_VIEW_KIND_READ_ONLY_DSV, FG_VIEW_KIND_NUM };
  //---------------------------------------------------------------------------//
  /// One view of a resource in the compiled graph
  struct FrameGraphCompiledView {
    TextureViewHandle myTextureView;
    GpuBufferViewHandle myBufferView;
    FgViewKind myKind = FG_VIEW_KIND_NUM;  // TODO: Add the format once accesses can request one (e.g. a float and a uint view)
  };
  //---------------------------------------------------------------------------//
  /// The views of one resource (FrameGraphResourceBase) in the compiled graph
  struct FrameGraphCompiledResource {
    eastl::fixed_vector< FrameGraphCompiledView, kFgMaxNumViewsPerResource, false > myViews;
    uint myHandleIndex = kFgInvalidIndex;
    uint myHandleGeneration = 0u;
  };
  //---------------------------------------------------------------------------//
  /// Resolves a task's handles to concrete resources and views in its Compile()
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

    void Reset();

    /// Returns uninitialized memory for the task instance, see FgAddTask() (FrameGraphTaskHelper.h)
    void * AddTask( const FgTaskCreateInfo & aCreateInfo );

    FgResourceHandle Read( FgResourceHandle aHandle, ResourceAccess anAccess );
    FgResourceHandle Write( FgResourceHandle aHandle, ResourceAccess anAccess );

    bool BuildFrameGraph();

    void Execute();

    void ReleaseCompiledGraph();

    bool IsBuilt() const;
    uint64 GetDeclarationHash() const;
    uint GetNumCompiles() const;
    const eastl::fixed_vector< FrameGraphTask, kFgMaxNumTasks > & GetTasks() const;
    const eastl::fixed_vector< FrameGraphResourceBase, kFgMaxNumResources > & GetResourceBases() const;
    const eastl::fixed_vector< FrameGraphResource, kFgMaxNumResources > & GetResources() const;
    const eastl::fixed_vector< FrameGraphDependency, kFgMaxNumTasks > & GetDependencies() const;
    const eastl::fixed_vector< uint, kFgMaxNumTasks > & GetTaskOrder() const;

  private:
    FgResourceHandle RegisterResource( GpuResource * aResource, const char * aName, uint aHandleIndex, uint aHandleGeneration );
    FrameGraphTask & GetCurrentTask();
    void ValidateHandle( FgResourceHandle aHandle ) const;
    FgResourceHandle AddVersion( uint aBaseIndex, uint aProducerTask );
    void AddAccess( FgResourceHandle aHandle, ResourceAccess anAccess );
    void AddDependency( uint aSrcTask, uint aDstTask );

    void ComputeDeclarationHash();
    void Compile();
    void CompileTask( uint aTaskIndex );
    GpuResourceView * GetCreateView( uint aBaseIndex, ResourceAccess anAccess );
    const FrameGraphResourceBase & GetBase( FgResourceHandle aHandle ) const;
    bool IsBackBuffer( const GpuResource * aResource ) const;

    RenderOutput * myRenderOutput = nullptr;

    LinearAllocator< kFgTaskMemorySize > myTaskAllocator;
    LinearAllocator< kFgTaskMemorySize > myCompiledRenderDataAllocator;

    // Declaration, recreated every frame
    eastl::fixed_vector< FrameGraphResourceBase, kFgMaxNumResources > myResourceBases;
    eastl::fixed_vector< FrameGraphResource, kFgMaxNumResources > myResources;
    eastl::fixed_vector< FrameGraphTask, kFgMaxNumTasks > myTasks;
    eastl::fixed_vector< FrameGraphDependency, kFgMaxNumTasks > myDependencies;
    eastl::fixed_vector< uint, kFgMaxNumTasks > myTaskOrder;
    uint64 myDeclarationHash = 0u;
    uint myFrameGraphGenerationIndex = 0u;
    bool myIsBuilt = false;

    // Compiled graph, only recreated when the declaration hash changes
    eastl::fixed_vector< FrameGraphCompiledTask, kFgMaxNumTasks > myCompiledTasks;
    eastl::fixed_vector< FrameGraphCompiledResource, kFgMaxNumResources > myCompiledResources;
    uint64 myCompiledHash = 0u;
    uint myNumCompiles = 0u;
    bool myCompiledIsValid = false;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy
