#include "fancy_core_precompile.h"
#include "FrameGraphBuilder.h"

#include "Common/MathUtil.h"
#include "Rendering/CommandList.h"
#include "Rendering/GpuBuffer.h"
#include "Rendering/RenderCore.h"
#include "Rendering/RenderOutput.h"
#include "Rendering/Texture.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  namespace Priv_FrameGraphBuilder {
    FgViewKind locGetViewKind( ResourceAccess anAccess ) {
      switch ( RenderCore::GetResourceAccessViewType( anAccess ) ) {
        case GpuResourceViewType::SRV:
          return FG_VIEW_KIND_SRV;
        case GpuResourceViewType::UAV:
          return FG_VIEW_KIND_UAV;
        case GpuResourceViewType::RTV:
          return FG_VIEW_KIND_RTV;
        case GpuResourceViewType::DSV:
          return anAccess == RESOURCE_ACCESS_DEPTH_READ ? FG_VIEW_KIND_READ_ONLY_DSV : FG_VIEW_KIND_DSV;
        default:
          return FG_VIEW_KIND_NUM;
      }
    }
  }  // namespace Priv_FrameGraphBuilder
  //---------------------------------------------------------------------------//
  FrameGraphCompileContext::FrameGraphCompileContext( FrameGraphBuilder * aBuilder, uint aTaskIndex )
      : myBuilder( aBuilder ), myTaskIndex( aTaskIndex ) {}
  //---------------------------------------------------------------------------//
  const FrameGraphTaskAccess * FrameGraphCompileContext::FindAccess( FgResourceHandle aHandle ) const {
    myBuilder->ValidateHandle( aHandle );

    const uint baseIndex = myBuilder->myResources[ aHandle.myResourceIndex ].myBaseIndex;
    for ( const FrameGraphTaskAccess & access : myBuilder->myTasks[ myTaskIndex ].myAccesses ) {
      if ( myBuilder->myResources[ access.myHandle.myResourceIndex ].myBaseIndex == baseIndex ) {
        return &access;
      }
    }
    return nullptr;
  }
  //---------------------------------------------------------------------------//
  GpuResourceView * FrameGraphCompileContext::GetView( FgResourceHandle aHandle, GpuResourceViewType aViewType, bool anIsTexture ) const {
    const FrameGraphTaskAccess * access = FindAccess( aHandle );
    const FrameGraphResourceBase & base = myBuilder->GetBase( aHandle );
    ASSERT( access != nullptr, "Task %s didn't declare an access of resource %s", myBuilder->myTasks[ myTaskIndex ].myName.c_str(),
            base.myName.c_str() );
    if ( access == nullptr ) {
      return nullptr;
    }
    ASSERT( RenderCore::GetResourceAccessViewType( access->myAccess ) == aViewType, "Resource %s is accessed as %s", base.myName.c_str(),
            RenderCore::GetResourceAccessName( access->myAccess ) );

    if ( myBuilder->IsBackBuffer( base.myResource ) ) {
      ASSERT( aViewType == GpuResourceViewType::RTV, "The backbuffer only has an RTV" );
      return myBuilder->myRenderOutput->GetBackbufferRtv();
    }

    ASSERT( base.myResource->IsTexture() == anIsTexture, "Resource %s is a %s", base.myName.c_str(), anIsTexture ? "buffer" : "texture" );
    return myBuilder->GetCreateView( myBuilder->myResources[ aHandle.myResourceIndex ].myBaseIndex, access->myAccess );
  }
  //---------------------------------------------------------------------------//
  TextureView * FrameGraphCompileContext::GetRtv( FgResourceHandle aHandle ) const {
    return static_cast< TextureView * >( GetView( aHandle, GpuResourceViewType::RTV, true ) );
  }
  //---------------------------------------------------------------------------//
  TextureView * FrameGraphCompileContext::GetDsv( FgResourceHandle aHandle ) const {
    return static_cast< TextureView * >( GetView( aHandle, GpuResourceViewType::DSV, true ) );
  }
  //---------------------------------------------------------------------------//
  TextureView * FrameGraphCompileContext::GetSrv( FgResourceHandle aHandle ) const {
    return static_cast< TextureView * >( GetView( aHandle, GpuResourceViewType::SRV, true ) );
  }
  //---------------------------------------------------------------------------//
  TextureView * FrameGraphCompileContext::GetUav( FgResourceHandle aHandle ) const {
    return static_cast< TextureView * >( GetView( aHandle, GpuResourceViewType::UAV, true ) );
  }
  //---------------------------------------------------------------------------//
  GpuBufferView * FrameGraphCompileContext::GetBufferSrv( FgResourceHandle aHandle ) const {
    return static_cast< GpuBufferView * >( GetView( aHandle, GpuResourceViewType::SRV, false ) );
  }
  //---------------------------------------------------------------------------//
  GpuBufferView * FrameGraphCompileContext::GetBufferUav( FgResourceHandle aHandle ) const {
    return static_cast< GpuBufferView * >( GetView( aHandle, GpuResourceViewType::UAV, false ) );
  }
  //---------------------------------------------------------------------------//
  GpuResource * FrameGraphCompileContext::GetResource( FgResourceHandle aHandle ) const {
    const FrameGraphResourceBase & base = myBuilder->GetBase( aHandle );
    ASSERT( FindAccess( aHandle ) != nullptr, "Task %s didn't declare an access of resource %s", myBuilder->myTasks[ myTaskIndex ].myName.c_str(),
            base.myName.c_str() );

    return base.myResource;
  }
  //---------------------------------------------------------------------------//
  FrameGraphBuilder::FrameGraphBuilder( RenderOutput * aRenderOutput ) : myRenderOutput( aRenderOutput ) {
    ASSERT( myRenderOutput != nullptr );
  }
  //---------------------------------------------------------------------------//
  FrameGraphBuilder::~FrameGraphBuilder() {
    Reset();
    ReleaseCompiledGraph();
  }
  //---------------------------------------------------------------------------//
  const FrameGraphResourceBase & FrameGraphBuilder::GetBase( FgResourceHandle aHandle ) const {
    return myResourceBases[ myResources[ aHandle.myResourceIndex ].myBaseIndex ];
  }
  //---------------------------------------------------------------------------//
  bool FrameGraphBuilder::IsBackBuffer( const GpuResource * aResource ) const {
    return aResource == myRenderOutput->GetBackbuffer();
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::RegisterExisting( TextureHandle aTexture ) {
    Texture * texture = RenderCore::GetTexture( aTexture );
    ASSERT( texture != nullptr, "Invalid or outdated texture handle" );
    if ( texture == nullptr ) {
      return FgResourceHandle();
    }
    return RegisterResource( texture, texture->GetName(), aTexture.myIndex, aTexture.myGeneration );
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::RegisterExisting( GpuBufferHandle aBuffer ) {
    GpuBuffer * buffer = RenderCore::GetBuffer( aBuffer );
    ASSERT( buffer != nullptr, "Invalid or outdated buffer handle" );
    if ( buffer == nullptr ) {
      return FgResourceHandle();
    }
    return RegisterResource( buffer, buffer->GetName(), aBuffer.myIndex, aBuffer.myGeneration );
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::RegisterBackBuffer() {
    return RegisterResource( myRenderOutput->GetBackbuffer(), "Backbuffer", kFgInvalidIndex, 0u );
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::RegisterResource( GpuResource * aResource, const char * aName, uint aHandleIndex, uint aHandleGeneration ) {
    ASSERT( aResource != nullptr );
    ASSERT( !myIsBuilt, "Can't register resources after the graph has been built. Call Reset() first" );
    for ( const FrameGraphResourceBase & base : myResourceBases ) {
      ASSERT( base.myResource != aResource, "Resource %s is already registered", aName );
    }

    FrameGraphResourceBase base;
    base.myName = aName;
    base.myResource = aResource;
    base.myHandleIndex = aHandleIndex;
    base.myHandleGeneration = aHandleGeneration;
    myResourceBases.push_back( base );

    return AddVersion( ( uint ) myResourceBases.size() - 1u, kFgInvalidIndex );
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::Reset() {
    myTasks.clear();
    myTaskAllocator.Reset();

    myResourceBases.clear();
    myResources.clear();
    myDependencies.clear();
    myTaskOrder.clear();
    myIsBuilt = false;
    myDeclarationHash = 0u;
    ++myFrameGraphGenerationIndex;
  }
  //---------------------------------------------------------------------------//
  void * FrameGraphBuilder::AddTask( const FgTaskCreateInfo & aCreateInfo ) {
    ASSERT( !myIsBuilt, "Can't add tasks after the graph has been built. Call Reset() first" );
    ASSERT( aCreateInfo.myExecuteFunction != nullptr && aCreateInfo.myCompileFunction != nullptr );

    void * memory = myTaskAllocator.Allocate( aCreateInfo.myTaskSize, aCreateInfo.myTaskAlignment );
    ASSERT( memory != nullptr, "Out of task memory. Increase kFgTaskMemorySize" );
    if ( memory == nullptr ) {
      return nullptr;
    }

    FrameGraphTask task;
    task.myName = aCreateInfo.myName;
    task.myQueueType = aCreateInfo.myQueueType;
    task.myInstance = memory;
    task.myRenderDataSize = aCreateInfo.myRenderDataSize;
    task.myRenderDataAlignment = aCreateInfo.myRenderDataAlignment;
    task.myExecuteFunction = aCreateInfo.myExecuteFunction;
    task.myCompileFunction = aCreateInfo.myCompileFunction;
    myTasks.push_back( task );
    return memory;
  }
  //---------------------------------------------------------------------------//
  FrameGraphTask & FrameGraphBuilder::GetCurrentTask() {
    ASSERT( !myTasks.empty(), "Resource accesses need to be declared after adding a task" );
    ASSERT( !myIsBuilt, "Can't change tasks after the graph has been built. Call Reset() first" );
    return myTasks.back();
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::ValidateHandle( FgResourceHandle aHandle ) const {
    ASSERT( aHandle.IsValid() );
    ASSERT( aHandle.myFrameGraphGenerationIndex == myFrameGraphGenerationIndex, "Resource handle from generation %d used in generation %d",
            aHandle.myFrameGraphGenerationIndex, myFrameGraphGenerationIndex );
    ASSERT( aHandle.myResourceIndex < myResources.size() );
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::AddVersion( uint aBaseIndex, uint aProducerTask ) {
    FrameGraphResourceBase & base = myResourceBases[ aBaseIndex ];

    FrameGraphResource resource;
    resource.myBaseIndex = aBaseIndex;
    resource.myProducerTask = aProducerTask;

    const uint resourceIndex = ( uint ) myResources.size();
    if ( base.myLatestResourceIndex != kFgInvalidIndex ) {
      FrameGraphResource & previousResource = myResources[ base.myLatestResourceIndex ];
      resource.myVersion = previousResource.myVersion + 1u;
      previousResource.myNextVersionResourceIndex = resourceIndex;
    }
    myResources.push_back( resource );
    base.myLatestResourceIndex = resourceIndex;

    FgResourceHandle handle;
    handle.myResourceIndex = resourceIndex;
    handle.myFrameGraphGenerationIndex = myFrameGraphGenerationIndex;
    return handle;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::AddAccess( FgResourceHandle aHandle, ResourceAccess anAccess ) {
    FrameGraphTask & task = GetCurrentTask();

    // TODO: Allow this with non-overlapping subresource ranges
    for ( const FrameGraphTaskAccess & access : task.myAccesses ) {
      ASSERT( myResources[ access.myHandle.myResourceIndex ].myBaseIndex != myResources[ aHandle.myResourceIndex ].myBaseIndex,
              "Task %s accesses resource %s more than once", task.myName.c_str(), GetBase( aHandle ).myName.c_str() );
    }

    FrameGraphTaskAccess access;
    access.myHandle = aHandle;
    access.myAccess = anAccess;
    task.myAccesses.push_back( access );
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::Read( FgResourceHandle aHandle, ResourceAccess anAccess ) {
    ValidateHandle( aHandle );
    ASSERT( !RenderCore::IsWriteAccess( anAccess ), "Read() needs a read access, use Write() for %s", RenderCore::GetResourceAccessName( anAccess ) );

    AddAccess( aHandle, anAccess );
    myResources[ aHandle.myResourceIndex ].myConsumerTasks.push_back( ( uint ) myTasks.size() - 1u );
    return aHandle;
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::Write( FgResourceHandle aHandle, ResourceAccess anAccess ) {
    ValidateHandle( aHandle );
    ASSERT( RenderCore::IsWriteAccess( anAccess ), "Write() needs a write access, use Read() for %s", RenderCore::GetResourceAccessName( anAccess ) );
    ASSERT( myResources[ aHandle.myResourceIndex ].myNextVersionResourceIndex == kFgInvalidIndex,
            "Resource %s: only its latest version can be written (version %d is outdated)", GetBase( aHandle ).myName.c_str(),
            myResources[ aHandle.myResourceIndex ].myVersion );

    const FgResourceHandle newHandle = AddVersion( myResources[ aHandle.myResourceIndex ].myBaseIndex, ( uint ) myTasks.size() - 1u );
    AddAccess( newHandle, anAccess );
    return newHandle;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::AddDependency( uint aSrcTask, uint aDstTask ) {
    // Versions produced before the graph have no producer. A task can also consume the version it overwrites itself.
    if ( aSrcTask == kFgInvalidIndex || aDstTask == kFgInvalidIndex || aSrcTask == aDstTask ) {
      return;
    }

    for ( const FrameGraphDependency & dependency : myDependencies ) {
      if ( dependency.mySrcTask == aSrcTask && dependency.myDstTask == aDstTask ) {
        return;
      }
    }

    FrameGraphDependency dependency;
    dependency.mySrcTask = aSrcTask;
    dependency.myDstTask = aDstTask;
    myDependencies.push_back( dependency );
  }
  //---------------------------------------------------------------------------//
  bool FrameGraphBuilder::BuildFrameGraph() {
    ASSERT( !myIsBuilt, "The graph has already been built. Call Reset() first" );

    // For each version v of a resource:
    // - Read after write: all consumers of v depend on the producer of v
    // - Write after write: the producer of v + 1 depends on the producer of v
    // - Write after read: the producer of v + 1 depends on all consumers of v
    myDependencies.clear();
    for ( const FrameGraphResource & resource : myResources ) {
      for ( uint consumer : resource.myConsumerTasks ) {
        AddDependency( resource.myProducerTask, consumer );
      }

      if ( resource.myNextVersionResourceIndex != kFgInvalidIndex ) {
        const uint nextProducer = myResources[ resource.myNextVersionResourceIndex ].myProducerTask;
        AddDependency( resource.myProducerTask, nextProducer );
        for ( uint consumer : resource.myConsumerTasks ) {
          AddDependency( consumer, nextProducer );
        }
      }
    }

    // Kahn's algorithm, picking the earliest added ready task. Reading an outdated version moves the consumer before
    // the task that overwrote it.
    const uint numTasks = ( uint ) myTasks.size();
    eastl::fixed_vector< uint, kFgMaxNumTasks > numIncomingEdges( numTasks, 0u );
    for ( const FrameGraphDependency & dependency : myDependencies ) {
      ++numIncomingEdges[ dependency.myDstTask ];
    }

    eastl::fixed_vector< bool, kFgMaxNumTasks > isScheduled( numTasks, false );
    myTaskOrder.clear();

    for ( uint numScheduled = 0u; numScheduled < numTasks; ++numScheduled ) {
      uint nextTask = kFgInvalidIndex;
      for ( uint taskIdx = 0u; taskIdx < numTasks && nextTask == kFgInvalidIndex; ++taskIdx ) {
        if ( !isScheduled[ taskIdx ] && numIncomingEdges[ taskIdx ] == 0u ) {
          nextTask = taskIdx;
        }
      }

      if ( nextTask == kFgInvalidIndex ) {
        LOG_ERROR( "FrameGraph contains a cycle" );
        myTaskOrder.clear();
        return false;
      }

      isScheduled[ nextTask ] = true;
      myTaskOrder.push_back( nextTask );

      for ( const FrameGraphDependency & dependency : myDependencies ) {
        if ( dependency.mySrcTask == nextTask ) {
          --numIncomingEdges[ dependency.myDstTask ];
        }
      }
    }

    myIsBuilt = true;
    return true;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::ReleaseCompiledGraph() {
    if ( !myCompiledResources.empty() ) {
      // Recompiling is rare (structure or resource changes), so simply make sure no frame in flight uses the views
      RenderCore::WaitForIdle( CommandListType::Graphics );
      for ( const FrameGraphCompiledResource & compiledResource : myCompiledResources ) {
        for ( const FrameGraphCompiledView & view : compiledResource.myViews ) {
          if ( view.myTextureView.IsValid() ) {
            RenderCore::DeleteTextureView( view.myTextureView );
          }
          if ( view.myBufferView.IsValid() ) {
            RenderCore::DeleteBufferView( view.myBufferView );
          }
        }
      }
      myCompiledResources.clear();
    }

    myCompiledTasks.clear();
    myCompiledRenderDataAllocator.Reset();
    myCompiledIsValid = false;
  }
  //---------------------------------------------------------------------------//
  GpuResourceView * FrameGraphBuilder::GetCreateView( uint aBaseIndex, ResourceAccess anAccess ) {
    using namespace Priv_FrameGraphBuilder;

    const FrameGraphResourceBase & base = myResourceBases[ aBaseIndex ];
    const GpuResourceViewType viewType = RenderCore::GetResourceAccessViewType( anAccess );
    const FgViewKind viewKind = locGetViewKind( anAccess );
    const bool isReadOnlyDepth = viewKind == FG_VIEW_KIND_READ_ONLY_DSV;
    const bool isTexture = base.myResource->IsTexture();
    ASSERT( viewKind != FG_VIEW_KIND_NUM, "Access %s doesn't need a view", RenderCore::GetResourceAccessName( anAccess ) );
    ASSERT( aBaseIndex < myCompiledResources.size() );

    // At most kFgMaxNumViewsPerResource views per resource, so this search is bounded
    FrameGraphCompiledResource & compiledResource = myCompiledResources[ aBaseIndex ];
    ASSERT( compiledResource.myHandleIndex == base.myHandleIndex && compiledResource.myHandleGeneration == base.myHandleGeneration,
            "Resource %s changed since the graph was compiled", base.myName.c_str() );
    for ( const FrameGraphCompiledView & existingView : compiledResource.myViews ) {
      if ( existingView.myKind == viewKind ) {
        return isTexture ? static_cast< GpuResourceView * >( RenderCore::GetTextureView( existingView.myTextureView ) )
                         : RenderCore::GetBufferView( existingView.myBufferView );
      }
    }

    ASSERT( !compiledResource.myViews.full(), "Too many views of resource %s. Increase kFgMaxNumViewsPerResource", base.myName.c_str() );
    if ( compiledResource.myViews.full() ) {
      return nullptr;
    }

    FrameGraphCompiledView view;
    view.myKind = viewKind;

    const char * viewTypeName = "SRV";
    if ( isTexture ) {
      Texture * texture = static_cast< Texture * >( base.myResource );
      const TextureProperties & textureProps = texture->GetProperties();

      // TODO: Subresource ranges per access. Until then, SRVs see all subresources and the other views mip 0.
      TextureViewProperties props;
      props.myDimension = textureProps.myDimension;
      props.myFormat = textureProps.myFormat;
      switch ( viewType ) {
        case GpuResourceViewType::SRV:
          props.mySubresourceRange = texture->GetSubresources();
          break;
        case GpuResourceViewType::UAV:
          props.myIsShaderWritable = true;
          viewTypeName = "UAV";
          break;
        case GpuResourceViewType::RTV:
          props.myIsRenderTarget = true;
          viewTypeName = "RTV";
          break;
        case GpuResourceViewType::DSV:
          props.myIsDepthReadOnly = isReadOnlyDepth;
          props.myIsStencilReadOnly = isReadOnlyDepth;
          viewTypeName = isReadOnlyDepth ? "Read-only DSV" : "DSV";
          break;
        default:
          ASSERT( false, "View type not supported for textures" );
          return nullptr;
      }

      eastl::string name( eastl::string::CtorSprintf(), "%s (FrameGraph %s)", base.myName.c_str(), viewTypeName );
      view.myTextureView = RenderCore::CreateTextureView( texture, props, name.c_str() );
      ASSERT( view.myTextureView.IsValid(), "Failed creating view %s", name.c_str() );
      if ( !view.myTextureView.IsValid() ) {
        return nullptr;
      }
      compiledResource.myViews.push_back( view );
      return RenderCore::GetTextureView( view.myTextureView );
    }

    GpuBuffer * buffer = static_cast< GpuBuffer * >( base.myResource );

    // TODO: Structured and typed buffer views. Until then, buffers are accessed as (RW)ByteAddressBuffers.
    GpuBufferViewProperties props;
    props.myIsRaw = true;
    props.mySize = buffer->GetByteSize();
    switch ( viewType ) {
      case GpuResourceViewType::SRV:
        break;
      case GpuResourceViewType::UAV:
        props.myIsShaderWritable = true;
        viewTypeName = "UAV";
        break;
      default:
        ASSERT( false, "View type not supported for buffers" );
        return nullptr;
    }

    eastl::string name( eastl::string::CtorSprintf(), "%s (FrameGraph %s)", base.myName.c_str(), viewTypeName );
    view.myBufferView = RenderCore::CreateBufferView( buffer, props, name.c_str() );
    ASSERT( view.myBufferView.IsValid(), "Failed creating view %s", name.c_str() );
    if ( !view.myBufferView.IsValid() ) {
      return nullptr;
    }
    compiledResource.myViews.push_back( view );
    return RenderCore::GetBufferView( view.myBufferView );
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::ComputeDeclarationHash() {
    uint64 hash = 0u;

    // The compiled views belong to the registered resources, identified by their engine handles. Recreating a resource changes the handle's
    // generation. The backbuffer has no handle, its swap chain image changes every frame and is resolved when executing instead.
    for ( const FrameGraphResourceBase & base : myResourceBases ) {
      MathUtil::hash_combine( hash, ( uint64 ) base.myResource->IsTexture() );
      MathUtil::hash_combine( hash, ( uint64 ) base.myHandleIndex );
      MathUtil::hash_combine( hash, ( uint64 ) base.myHandleGeneration );
    }

    for ( const FrameGraphTask & task : myTasks ) {
      MathUtil::hash_combine( hash, MathUtil::Hash( task.myName.c_str() ) );
      MathUtil::hash_combine( hash, ( uint64 ) task.myQueueType );
      MathUtil::hash_combine( hash, reinterpret_cast< uint64 >( task.myExecuteFunction ) );
      MathUtil::hash_combine( hash, ( uint64 ) task.myAccesses.size() );

      for ( const FrameGraphTaskAccess & access : task.myAccesses ) {
        MathUtil::hash_combine( hash, ( uint64 ) myResources[ access.myHandle.myResourceIndex ].myBaseIndex );
        MathUtil::hash_combine( hash, ( uint64 ) access.myHandle.myResourceIndex );
        MathUtil::hash_combine( hash, ( uint64 ) access.myAccess );
      }
    }

    myDeclarationHash = hash;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::Compile() {
    ComputeDeclarationHash();
    if ( myCompiledIsValid && myCompiledHash == myDeclarationHash ) {
      return;
    }

    ReleaseCompiledGraph();

    myCompiledTasks.resize( myTasks.size() );
    myCompiledResources.resize( myResourceBases.size() );
    for ( uint i = 0u, e = ( uint ) myResourceBases.size(); i < e; ++i ) {
      myCompiledResources[ i ].myHandleIndex = myResourceBases[ i ].myHandleIndex;
      myCompiledResources[ i ].myHandleGeneration = myResourceBases[ i ].myHandleGeneration;
    }

    eastl::fixed_vector< ResourceAccess, kFgMaxNumResources > lastAccess( myResourceBases.size(), RESOURCE_ACCESS_NONE );

    for ( uint taskIdx : myTaskOrder ) {
      const FrameGraphTask & task = myTasks[ taskIdx ];
      FrameGraphCompiledTask & compiledTask = myCompiledTasks[ taskIdx ];

      compiledTask.myRenderData = myCompiledRenderDataAllocator.Allocate( task.myRenderDataSize, task.myRenderDataAlignment );
      ASSERT( compiledTask.myRenderData != nullptr, "Out of render data memory. Increase kFgTaskMemorySize" );
      CompileTask( taskIdx );

      for ( const FrameGraphTaskAccess & access : task.myAccesses ) {
        const uint baseIndex = myResources[ access.myHandle.myResourceIndex ].myBaseIndex;
        compiledTask.myAccessesBackBuffer |= IsBackBuffer( myResourceBases[ baseIndex ].myResource );

        // First accesses are patched from the global state at submit time. Writes always need one (e.g. UAV -> UAV).
        const ResourceAccess previousAccess = lastAccess[ baseIndex ];
        if ( previousAccess != access.myAccess || RenderCore::IsWriteAccess( access.myAccess ) ) {
          FrameGraphBarrier barrier;
          barrier.myBaseIndex = baseIndex;
          barrier.myAccess = access.myAccess;
          compiledTask.myBarriers.push_back( barrier );
        }
        lastAccess[ baseIndex ] = access.myAccess;
      }
    }

    myCompiledHash = myDeclarationHash;
    myCompiledIsValid = true;
    ++myNumCompiles;

    uint numViews = 0u;
    for ( const FrameGraphCompiledResource & compiledResource : myCompiledResources ) {
      numViews += ( uint ) compiledResource.myViews.size();
    }
    LOG_INFO( "FrameGraph compiled (%d tasks, %d views)", ( int ) myTasks.size(), ( int ) numViews );
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::CompileTask( uint aTaskIndex ) {
    const FrameGraphTask & task = myTasks[ aTaskIndex ];
    const FrameGraphCompileContext context( this, aTaskIndex );
    task.myCompileFunction( task.myInstance, context, myCompiledTasks[ aTaskIndex ].myRenderData );
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::Execute() {
    ASSERT( myIsBuilt, "BuildFrameGraph() needs to succeed before Execute()" );
    if ( !myIsBuilt || myTasks.empty() ) {
      return;
    }

    Compile();

    // The swap chain image changes every frame
    for ( uint taskIdx : myTaskOrder ) {
      if ( myCompiledTasks[ taskIdx ].myAccessesBackBuffer ) {
        CompileTask( taskIdx );
      }
    }

    CommandList * ctx = RenderCore::BeginCommandList( CommandListType::Graphics, false );

    for ( uint taskIdx : myTaskOrder ) {
      const FrameGraphTask & task = myTasks[ taskIdx ];
      const FrameGraphCompiledTask & compiledTask = myCompiledTasks[ taskIdx ];

      ctx->BeginMarkerRegion( task.myName.c_str() );

      for ( const FrameGraphBarrier & barrier : compiledTask.myBarriers ) {
        ctx->TransitionResourceForAccess( myResourceBases[ barrier.myBaseIndex ].myResource, barrier.myAccess );
      }

      task.myExecuteFunction( ctx, task.myInstance, compiledTask.myRenderData );

      ctx->EndMarkerRegion();
    }

    RenderCore::ExecuteAndFreeCommandList( ctx );
  }
  //---------------------------------------------------------------------------//
  bool FrameGraphBuilder::IsBuilt() const {
    return myIsBuilt;
  }
  //---------------------------------------------------------------------------//
  uint64 FrameGraphBuilder::GetDeclarationHash() const {
    return myDeclarationHash;
  }
  //---------------------------------------------------------------------------//
  uint FrameGraphBuilder::GetNumCompiles() const {
    return myNumCompiles;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< FrameGraphTask, kFgMaxNumTasks > & FrameGraphBuilder::GetTasks() const {
    return myTasks;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< FrameGraphResourceBase, kFgMaxNumResources > & FrameGraphBuilder::GetResourceBases() const {
    return myResourceBases;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< FrameGraphResource, kFgMaxNumResources > & FrameGraphBuilder::GetResources() const {
    return myResources;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< FrameGraphDependency, kFgMaxNumTasks > & FrameGraphBuilder::GetDependencies() const {
    return myDependencies;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< uint, kFgMaxNumTasks > & FrameGraphBuilder::GetTaskOrder() const {
    return myTaskOrder;
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy
