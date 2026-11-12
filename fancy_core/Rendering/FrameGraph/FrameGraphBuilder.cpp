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
    //---------------------------------------------------------------------------//
    /// The accesses of one subresource so far while compiling
    struct FgSubresourceAccesses {
      ResourceAccess myFirstAccess = RESOURCE_ACCESS_NONE;
      ResourceAccess myLastAccess = RESOURCE_ACCESS_NONE;
    };
    //---------------------------------------------------------------------------//
    /// Merges anOther into aRange if the result is a contiguous range again. anOther has to come after aRange in iteration order.
    bool locTryMergeRanges( SubresourceRange & aRange, const SubresourceRange & anOther ) {
      const bool sameMips = aRange.myFirstMipLevel == anOther.myFirstMipLevel && aRange.myNumMipLevels == anOther.myNumMipLevels;
      const bool sameSlices = aRange.myFirstArrayIndex == anOther.myFirstArrayIndex && aRange.myNumArrayIndices == anOther.myNumArrayIndices;
      const bool samePlanes = aRange.myFirstPlane == anOther.myFirstPlane && aRange.myNumPlanes == anOther.myNumPlanes;

      if ( sameSlices && samePlanes && aRange.myFirstMipLevel + aRange.myNumMipLevels == anOther.myFirstMipLevel ) {
        aRange.myNumMipLevels += anOther.myNumMipLevels;
        return true;
      }
      if ( sameMips && samePlanes && aRange.myFirstArrayIndex + aRange.myNumArrayIndices == anOther.myFirstArrayIndex ) {
        aRange.myNumArrayIndices += anOther.myNumArrayIndices;
        return true;
      }
      if ( sameMips && sameSlices && aRange.myFirstPlane + aRange.myNumPlanes == anOther.myFirstPlane ) {
        aRange.myNumPlanes += anOther.myNumPlanes;
        return true;
      }
      return false;
    }
    //---------------------------------------------------------------------------//
    /// Merges barriers (in subresource iteration order) with the same accesses into contiguous ranges
    void locMergeBarriers( eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumBarriersPerTask > & someBarriers ) {
      bool hasMerged = true;
      while ( hasMerged ) {
        hasMerged = false;
        for ( uint i = 0u; i < ( uint ) someBarriers.size(); ++i ) {
          for ( uint k = i + 1u; k < ( uint ) someBarriers.size(); ) {
            if ( someBarriers[ i ].mySrcAccess == someBarriers[ k ].mySrcAccess && someBarriers[ i ].myDstAccess == someBarriers[ k ].myDstAccess &&
                 locTryMergeRanges( someBarriers[ i ].mySubresources, someBarriers[ k ].mySubresources ) ) {
              someBarriers.erase( someBarriers.begin() + k );
              hasMerged = true;
            } else {
              ++k;
            }
          }
        }
      }
    }
  }  // namespace Priv_FrameGraphBuilder
  //---------------------------------------------------------------------------//
  bool FgResourceHandle::IsValid() const {
    return myBaseIndex != kFgInvalidIndex;
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FgResourceHandle::Mips( uint aFirstMip, uint aNumMips ) const {
    FgResourceHandle result = *this;
    result.mySubresources.myFirstMipLevel = aFirstMip;
    result.mySubresources.myNumMipLevels = aNumMips;
    ASSERT( mySubresources.Contains( result.mySubresources ), "Mips %u-%u are outside of the handle's subresources", aFirstMip,
            aFirstMip + aNumMips - 1u );
    return result;
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FgResourceHandle::Planes( uint aFirstPlane, uint aNumPlanes ) const {
    FgResourceHandle result = *this;
    result.mySubresources.myFirstPlane = aFirstPlane;
    result.mySubresources.myNumPlanes = aNumPlanes;
    ASSERT( mySubresources.Contains( result.mySubresources ), "Planes %u-%u are outside of the handle's subresources", aFirstPlane,
            aFirstPlane + aNumPlanes - 1u );
    return result;
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FgResourceHandle::ArraySlices( uint aFirstIndex, uint aNumIndices ) const {
    FgResourceHandle result = *this;
    result.mySubresources.myFirstArrayIndex = aFirstIndex;
    result.mySubresources.myNumArrayIndices = aNumIndices;
    ASSERT( mySubresources.Contains( result.mySubresources ), "Array slices %u-%u are outside of the handle's subresources", aFirstIndex,
            aFirstIndex + aNumIndices - 1u );
    return result;
  }
  //---------------------------------------------------------------------------//
  FrameGraphCompileContext::FrameGraphCompileContext( FrameGraphBuilder * aBuilder, uint aTaskIndex )
      : myBuilder( aBuilder ), myTaskIndex( aTaskIndex ) {}
  //---------------------------------------------------------------------------//
  const FrameGraphTaskAccess * FrameGraphCompileContext::FindAccess( FgResourceHandle aHandle ) const {
    myBuilder->ValidateHandle( aHandle );

    for ( const FrameGraphTaskAccess & access : myBuilder->myTasks[ myTaskIndex ].myAccesses ) {
      if ( access.myHandle.myBaseIndex == aHandle.myBaseIndex && access.myHandle.mySubresources == aHandle.mySubresources ) {
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
    return myBuilder->GetCreateView( aHandle.myBaseIndex, aHandle.mySubresources, access->myAccess );
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
    return myResourceBases[ aHandle.myBaseIndex ];
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

    FgResourceHandle handle;
    handle.mySubresources = aResource->GetSubresources();
    handle.myBaseIndex = ( uint ) myResourceBases.size() - 1u;
    handle.myFrameGraphGenerationIndex = myFrameGraphGenerationIndex;
    return handle;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::Reset() {
    myTasks.clear();
    myTaskAllocator.Reset();

    myResourceBases.clear();
    mySubresourceStates.clear();
    myDependencies.clear();
    myTaskOrder.clear();
    myIsBuilt = false;
    myDeclarationHash = 0u;
    ++myFrameGraphGenerationIndex;
  }
  //---------------------------------------------------------------------------//
  uint FrameGraphBuilder::AddTask( const FgTaskCreateInfo & aCreateInfo, void ** aTaskMemoryOut ) {
    ASSERT( !myIsBuilt, "Can't add tasks after the graph has been built. Call Reset() first" );
    ASSERT( aCreateInfo.myExecuteFunction != nullptr && aCreateInfo.myCompileFunction != nullptr );

    void * memory = myTaskAllocator.Allocate( aCreateInfo.myTaskSize, aCreateInfo.myTaskAlignment );
    *aTaskMemoryOut = memory;
    ASSERT( memory != nullptr, "Out of task memory. Increase kFgTaskMemorySize" );
    if ( memory == nullptr ) {
      return kFgInvalidIndex;
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
    return ( uint ) myTasks.size() - 1u;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::ValidateHandle( FgResourceHandle aHandle ) const {
    ASSERT( aHandle.IsValid() );
    ASSERT( aHandle.myFrameGraphGenerationIndex == myFrameGraphGenerationIndex, "Resource handle from generation %d used in generation %d",
            aHandle.myFrameGraphGenerationIndex, myFrameGraphGenerationIndex );
    ASSERT( aHandle.myBaseIndex < myResourceBases.size() );
    ASSERT( GetBase( aHandle ).myResource->GetSubresources().Contains( aHandle.mySubresources ), "Handle to %s lies outside of the resource",
            GetBase( aHandle ).myName.c_str() );
  }
  //---------------------------------------------------------------------------//
  FgSubresourceState & FrameGraphBuilder::GetSubresourceState( const FrameGraphResourceBase & aBase, const SubresourceLocation & aLocation ) {
    return mySubresourceStates[ aBase.myFirstSubresourceState + aBase.myResource->GetSubresourceIndex( aLocation ) ];
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::AddAccess( uint aTaskIndex, FgResourceHandle aHandle, ResourceAccess anAccess ) {
    ASSERT( !myIsBuilt, "Can't change tasks after the graph has been built. Call Reset() first" );
    ASSERT( aTaskIndex < myTasks.size(), "Invalid task index %u", aTaskIndex );
    ASSERT( anAccess != RESOURCE_ACCESS_NONE, "Missing access for resource %s", GetBase( aHandle ).myName.c_str() );
    if ( aTaskIndex >= myTasks.size() || anAccess == RESOURCE_ACCESS_NONE ) {
      return;
    }

    FrameGraphTask & task = myTasks[ aTaskIndex ];

    for ( const FrameGraphTaskAccess & access : task.myAccesses ) {
      ASSERT( access.myHandle.myBaseIndex != aHandle.myBaseIndex || !access.myHandle.mySubresources.Overlaps( aHandle.mySubresources ),
              "Task %s accesses overlapping subresources of %s more than once", task.myName.c_str(), GetBase( aHandle ).myName.c_str() );
    }

    if ( anAccess == RESOURCE_ACCESS_DEPTH_WRITE || anAccess == RESOURCE_ACCESS_DEPTH_READ ) {
      const SubresourceRange & resourceRange = GetBase( aHandle ).myResource->GetSubresources();
      ASSERT( aHandle.mySubresources.myFirstPlane == 0u && aHandle.mySubresources.myNumPlanes == resourceRange.myNumPlanes,
              "Depth access to %s needs to cover all planes", GetBase( aHandle ).myName.c_str() );
    }

    FrameGraphTaskAccess access;
    access.myHandle = aHandle;
    access.myAccess = anAccess;
    task.myAccesses.push_back( access );
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::Read( uint aTaskIndex, FgResourceHandle aHandle, ResourceAccess anAccess ) {
    ValidateHandle( aHandle );
    ASSERT( !RenderCore::IsWriteAccess( anAccess ), "Read() needs a read access, use Write() for %s", RenderCore::GetResourceAccessName( anAccess ) );
    AddAccess( aTaskIndex, aHandle, anAccess );
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::Write( uint aTaskIndex, FgResourceHandle aHandle, ResourceAccess anAccess ) {
    ValidateHandle( aHandle );
    ASSERT( RenderCore::IsWriteAccess( anAccess ), "Write() needs a write access, use Read() for %s", RenderCore::GetResourceAccessName( anAccess ) );
    AddAccess( aTaskIndex, aHandle, anAccess );
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::AddDependency( uint aSrcTask, uint aDstTask ) {
    // Subresources written before the graph have no writer
    if ( aSrcTask == kFgInvalidIndex ) {
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

    // Replay all accesses in setup order. Per subresource, a read depends on the last writer (read after write), a write on the last
    // writer (write after write) and on all readers since (write after read).
    myDependencies.clear();
    mySubresourceStates.clear();
    for ( FrameGraphResourceBase & base : myResourceBases ) {
      base.myFirstSubresourceState = ( uint ) mySubresourceStates.size();
      mySubresourceStates.resize( mySubresourceStates.size() + base.myResource->GetSubresources().GetNumSubresources() );
    }

    for ( uint taskIdx = 0u, numTasks = ( uint ) myTasks.size(); taskIdx < numTasks; ++taskIdx ) {
      for ( const FrameGraphTaskAccess & access : myTasks[ taskIdx ].myAccesses ) {
        const FrameGraphResourceBase & base = GetBase( access.myHandle );
        const bool isWrite = RenderCore::IsWriteAccess( access.myAccess );
        for ( SubresourceIterator it = access.myHandle.mySubresources.Begin(); it != access.myHandle.mySubresources.End(); ++it ) {
          FgSubresourceState & state = GetSubresourceState( base, *it );
          AddDependency( state.myWriterTask, taskIdx );
          if ( !isWrite ) {
            state.myReaderTasks.push_back( taskIdx );
            continue;
          }

          for ( uint reader : state.myReaderTasks ) {
            AddDependency( reader, taskIdx );
          }
          state.myWriterTask = taskIdx;
          state.myReaderTasks.clear();
        }
      }
    }

    // Kahn's algorithm, picking the earliest added ready task
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
      for ( const FrameGraphCompiledView & view : myCompiledViews ) {
        if ( view.myTextureView.IsValid() ) {
          RenderCore::DeleteTextureView( view.myTextureView );
        }
        if ( view.myBufferView.IsValid() ) {
          RenderCore::DeleteBufferView( view.myBufferView );
        }
      }
      myCompiledViews.clear();
      myCompiledResources.clear();
    }

    myCompiledTasks.clear();
    myCompiledInitialAccesses.clear();
    myCompiledFinalBarriers.clear();
    myCompiledRenderDataAllocator.Reset();
    myCompiledIsValid = false;
  }
  //---------------------------------------------------------------------------//
  GpuResourceView * FrameGraphBuilder::GetCreateView( uint aBaseIndex, const SubresourceRange & aRange, ResourceAccess anAccess ) {
    using namespace Priv_FrameGraphBuilder;

    const FrameGraphResourceBase & base = myResourceBases[ aBaseIndex ];
    const GpuResourceViewType viewType = RenderCore::GetResourceAccessViewType( anAccess );
    const FgViewKind viewKind = locGetViewKind( anAccess );
    const bool isReadOnlyDepth = viewKind == FG_VIEW_KIND_READ_ONLY_DSV;
    const bool isTexture = base.myResource->IsTexture();
    ASSERT( viewKind != FG_VIEW_KIND_NUM, "Access %s doesn't need a view", RenderCore::GetResourceAccessName( anAccess ) );
    ASSERT( aBaseIndex < myCompiledResources.size() );

    FrameGraphCompiledResource & compiledResource = myCompiledResources[ aBaseIndex ];
    ASSERT( compiledResource.myHandleIndex == base.myHandleIndex && compiledResource.myHandleGeneration == base.myHandleGeneration,
            "Resource %s changed since the graph was compiled", base.myName.c_str() );
    for ( uint viewIndex = compiledResource.myFirstViewIndex; viewIndex != kFgInvalidIndex;
          viewIndex = myCompiledViews[ viewIndex ].myNextViewIndex ) {
      const FrameGraphCompiledView & existingView = myCompiledViews[ viewIndex ];
      if ( existingView.myKind == viewKind && existingView.mySubresources == aRange ) {
        return isTexture ? static_cast< GpuResourceView * >( RenderCore::GetTextureView( existingView.myTextureView ) )
                         : RenderCore::GetBufferView( existingView.myBufferView );
      }
    }

    ASSERT( !myCompiledViews.full(), "Too many views in the graph. Increase kFgMaxNumResourceViewsPerFrame" );
    if ( myCompiledViews.full() ) {
      return nullptr;
    }

    FrameGraphCompiledView view;
    view.myKind = viewKind;
    view.mySubresources = aRange;
    view.myNextViewIndex = compiledResource.myFirstViewIndex;

    const char * viewTypeName = "SRV";
    if ( isTexture ) {
      Texture * texture = static_cast< Texture * >( base.myResource );
      const TextureProperties & textureProps = texture->GetProperties();

      const bool isMultiPlane = texture->GetSubresources().myNumPlanes > 1u;
      TextureViewProperties props;
      props.myDimension = textureProps.myDimension;
      props.myFormat = textureProps.myFormat;
      props.mySubresourceRange = aRange;
      switch ( viewType ) {
        case GpuResourceViewType::SRV:
          ASSERT( !isMultiPlane || aRange.myNumPlanes == 1u, "SRV of %s needs to be a single plane", base.myName.c_str() );
          break;
        case GpuResourceViewType::UAV:
          ASSERT( aRange.myNumMipLevels == 1u, "UAV of %s needs to be a single mip", base.myName.c_str() );
          ASSERT( !isMultiPlane || aRange.myNumPlanes == 1u, "UAV of %s needs to be a single plane", base.myName.c_str() );
          props.myIsShaderWritable = true;
          viewTypeName = "UAV";
          break;
        case GpuResourceViewType::RTV:
          ASSERT( aRange.myNumMipLevels == 1u, "RTV of %s needs to be a single mip", base.myName.c_str() );
          props.myIsRenderTarget = true;
          viewTypeName = "RTV";
          break;
        case GpuResourceViewType::DSV:
          ASSERT( aRange.myNumMipLevels == 1u, "DSV of %s needs to be a single mip", base.myName.c_str() );
          ASSERT( aRange.myFirstPlane == 0u && aRange.myNumPlanes == texture->GetSubresources().myNumPlanes, "DSV of %s needs to cover all planes",
                  base.myName.c_str() );
          props.myIsRenderTarget = true;
          props.myIsDepthReadOnly = isReadOnlyDepth;
          props.myIsStencilReadOnly = isReadOnlyDepth;
          viewTypeName = isReadOnlyDepth ? "Read-only DSV" : "DSV";
          break;
        default:
          ASSERT( false, "View type not supported for textures" );
          return nullptr;
      }

      eastl::string name( eastl::string::CtorSprintf(), "%s (FrameGraph %s", base.myName.c_str(), viewTypeName );
      if ( !( aRange == texture->GetSubresources() ) ) {
        name.append_sprintf( " mips %u-%u planes %u-%u slices %u-%u", aRange.myFirstMipLevel, aRange.myFirstMipLevel + aRange.myNumMipLevels - 1u,
                             aRange.myFirstPlane, aRange.myFirstPlane + aRange.myNumPlanes - 1u, aRange.myFirstArrayIndex,
                             aRange.myFirstArrayIndex + aRange.myNumArrayIndices - 1u );
      }
      name.append( ")" );
      view.myTextureView = RenderCore::CreateTextureView( texture, props, name.c_str() );
      ASSERT( view.myTextureView.IsValid(), "Failed creating view %s", name.c_str() );
      if ( !view.myTextureView.IsValid() ) {
        return nullptr;
      }
      compiledResource.myFirstViewIndex = ( uint ) myCompiledViews.size();
      myCompiledViews.push_back( view );
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
    compiledResource.myFirstViewIndex = ( uint ) myCompiledViews.size();
    myCompiledViews.push_back( view );
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
        const SubresourceRange & range = access.myHandle.mySubresources;
        MathUtil::hash_combine( hash, ( uint64 ) access.myHandle.myBaseIndex );
        MathUtil::hash_combine( hash, ( uint64 ) range.myFirstMipLevel );
        MathUtil::hash_combine( hash, ( uint64 ) range.myNumMipLevels );
        MathUtil::hash_combine( hash, ( uint64 ) range.myFirstArrayIndex );
        MathUtil::hash_combine( hash, ( uint64 ) range.myNumArrayIndices );
        MathUtil::hash_combine( hash, ( uint64 ) range.myFirstPlane );
        MathUtil::hash_combine( hash, ( uint64 ) range.myNumPlanes );
        MathUtil::hash_combine( hash, ( uint64 ) access.myAccess );
      }
    }

    myDeclarationHash = hash;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::Compile() {
    using namespace Priv_FrameGraphBuilder;

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

    eastl::fixed_vector< FgSubresourceAccesses, kFgMaxNumSubresources > subresourceAccesses( mySubresourceStates.size() );

    for ( uint taskIdx : myTaskOrder ) {
      const FrameGraphTask & task = myTasks[ taskIdx ];
      FrameGraphCompiledTask & compiledTask = myCompiledTasks[ taskIdx ];

      compiledTask.myRenderData = myCompiledRenderDataAllocator.Allocate( task.myRenderDataSize, task.myRenderDataAlignment );
      ASSERT( compiledTask.myRenderData != nullptr, "Out of render data memory. Increase kFgTaskMemorySize" );
      CompileTask( taskIdx );

      for ( const FrameGraphTaskAccess & access : task.myAccesses ) {
        const uint baseIndex = access.myHandle.myBaseIndex;
        const FrameGraphResourceBase & base = myResourceBases[ baseIndex ];
        const bool isWrite = RenderCore::IsWriteAccess( access.myAccess );
        compiledTask.myAccessesBackBuffer |= IsBackBuffer( base.myResource );

        eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumBarriersPerTask > barriers;
        for ( SubresourceIterator it = access.myHandle.mySubresources.Begin(); it != access.myHandle.mySubresources.End(); ++it ) {
          FgSubresourceAccesses & accesses = subresourceAccesses[ base.myFirstSubresourceState + base.myResource->GetSubresourceIndex( *it ) ];

          if ( accesses.myLastAccess == RESOURCE_ACCESS_NONE ) {
            // No barrier for the first access: Execute() brings the subresource into this state before the graph starts
            accesses.myFirstAccess = access.myAccess;
          } else if ( isWrite || accesses.myLastAccess != access.myAccess ) {
            FrameGraphBarrier barrier;
            barrier.mySubresources = SubresourceRange( *it );
            barrier.myBaseIndex = baseIndex;
            barrier.mySrcAccess = accesses.myLastAccess;
            barrier.myDstAccess = access.myAccess;
            barriers.push_back( barrier );
          }

          accesses.myLastAccess = access.myAccess;
        }

        locMergeBarriers( barriers );
        compiledTask.myBarriers.insert( compiledTask.myBarriers.end(), barriers.begin(), barriers.end() );
      }
    }

    // The graph expects every subresource in the state of its first access and restores that state at its end
    for ( uint baseIndex = 0u, numBases = ( uint ) myResourceBases.size(); baseIndex < numBases; ++baseIndex ) {
      const FrameGraphResourceBase & base = myResourceBases[ baseIndex ];
      const uint numSubresources = base.myResource->GetSubresources().GetNumSubresources();

      eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumBarriersPerTask > initialAccesses;
      eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumBarriersPerTask > finalBarriers;
      for ( uint subresourceIndex = 0u; subresourceIndex < numSubresources; ++subresourceIndex ) {
        const FgSubresourceAccesses & accesses = subresourceAccesses[ base.myFirstSubresourceState + subresourceIndex ];
        if ( accesses.myFirstAccess == RESOURCE_ACCESS_NONE ) {
          continue;
        }

        FrameGraphBarrier barrier;
        barrier.mySubresources = SubresourceRange( base.myResource->GetSubresourceLocation( subresourceIndex ) );
        barrier.myBaseIndex = baseIndex;
        barrier.myDstAccess = accesses.myFirstAccess;
        initialAccesses.push_back( barrier );

        if ( accesses.myLastAccess != accesses.myFirstAccess ) {
          barrier.mySrcAccess = accesses.myLastAccess;
          finalBarriers.push_back( barrier );
        }
      }

      locMergeBarriers( initialAccesses );
      locMergeBarriers( finalBarriers );
      myCompiledInitialAccesses.insert( myCompiledInitialAccesses.end(), initialAccesses.begin(), initialAccesses.end() );
      myCompiledFinalBarriers.insert( myCompiledFinalBarriers.end(), finalBarriers.begin(), finalBarriers.end() );
    }

    myCompiledHash = myDeclarationHash;
    myCompiledIsValid = true;
    ++myNumCompiles;

    LOG_INFO( "FrameGraph compiled (%d tasks, %d views)", ( int ) myTasks.size(), ( int ) myCompiledViews.size() );
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

    // Hand the resources over from the legacy hazard tracking: afterwards, each subresource is in the state of its first access in the graph.
    // Since the graph restores that state at its end, the tracked state stays correct without the graph's list having to report anything.
    CommandList * handoverCtx = RenderCore::BeginCommandList( CommandListType::Graphics );
    for ( const FrameGraphBarrier & initialAccess : myCompiledInitialAccesses ) {
      handoverCtx->TrackResourceAccess( myResourceBases[ initialAccess.myBaseIndex ].myResource, initialAccess.mySubresources,
                                        initialAccess.myDstAccess );
    }
    RenderCore::ExecuteAndFreeCommandList( handoverCtx );

    CommandList * ctx = RenderCore::BeginCommandList( CommandListType::Graphics, false );

    for ( uint taskIdx : myTaskOrder ) {
      const FrameGraphTask & task = myTasks[ taskIdx ];
      const FrameGraphCompiledTask & compiledTask = myCompiledTasks[ taskIdx ];

      ctx->BeginMarkerRegion( task.myName.c_str() );

      for ( const FrameGraphBarrier & barrier : compiledTask.myBarriers ) {
        ctx->TransitionResourceForAccess( myResourceBases[ barrier.myBaseIndex ].myResource, barrier.mySubresources, barrier.mySrcAccess,
                                          barrier.myDstAccess );
      }
      // The compiled barriers belong to this task and are issued right before its work. Flushing them here keeps them apart from barriers the
      // task records itself (e.g. Vulkan clears) and from the next task's barriers on the same subresources.
      ctx->FlushBarriers();

      task.myExecuteFunction( ctx, task.myInstance, compiledTask.myRenderData );

      ctx->EndMarkerRegion();
    }

    for ( const FrameGraphBarrier & barrier : myCompiledFinalBarriers ) {
      ctx->TransitionResourceForAccess( myResourceBases[ barrier.myBaseIndex ].myResource, barrier.mySubresources, barrier.mySrcAccess,
                                        barrier.myDstAccess );
    }
    ctx->FlushBarriers();

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
  const eastl::fixed_vector< FrameGraphDependency, kFgMaxNumTasks > & FrameGraphBuilder::GetDependencies() const {
    return myDependencies;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< uint, kFgMaxNumTasks > & FrameGraphBuilder::GetTaskOrder() const {
    return myTaskOrder;
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy
