#include "fancy_core_precompile.h"
#include "FrameGraphBuilder.h"

#include "Common/MathUtil.h"
#include "Common/Ptr.h"
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
    /// A read of a resource version while building the graph, linked to the next read of the same version
    struct FgVersionRead {
      uint myTaskIndex = kFgInvalidIndex;
      SubresourceRange myRange;
      uint myNextRead = kFgInvalidIndex;
    };
    //---------------------------------------------------------------------------//
    void locAddDependency( eastl::fixed_vector< FrameGraphDependency, kFgMaxNumTasks > & someDependencies, uint aSrcTask, uint aDstTask ) {
      // Subresources written before the graph have no writer
      if ( aSrcTask == kFgInvalidIndex ) {
        return;
      }

      for ( const FrameGraphDependency & dependency : someDependencies ) {
        if ( dependency.mySrcTask == aSrcTask && dependency.myDstTask == aDstTask ) {
          return;
        }
      }

      FrameGraphDependency dependency;
      dependency.mySrcTask = aSrcTask;
      dependency.myDstTask = aDstTask;
      someDependencies.push_back( dependency );
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
    return myVersionIndex != kFgInvalidIndex;
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

    for ( const FrameGraphTaskAccess & access : myBuilder->mySetupData.myTasks[ myTaskIndex ].myAccesses ) {
      if ( access.myHandle.myVersionIndex == aHandle.myVersionIndex && access.myHandle.mySubresources.Contains( aHandle.mySubresources ) ) {
        return &access;
      }
    }
    return nullptr;
  }
  //---------------------------------------------------------------------------//
  GpuResourceView * FrameGraphCompileContext::GetView( FgResourceHandle aHandle, GpuResourceViewType aViewType, bool anIsTexture ) const {
    const FrameGraphTaskAccess * access = FindAccess( aHandle );
    const FrameGraphResourceBase & base = myBuilder->GetBase( aHandle );
    ASSERT( access != nullptr, "Task %s didn't declare an access of resource %s", myBuilder->mySetupData.myTasks[ myTaskIndex ].myName.c_str(),
            base.myName.c_str() );
    if ( access == nullptr ) {
      return nullptr;
    }
    ASSERT( RenderCore::GetResourceAccessViewType( access->myAccess ) == aViewType, "Resource %s is accessed as %s", base.myName.c_str(),
            RenderCore::GetResourceAccessName( access->myAccess ) );

    if ( base.myIsBackBuffer ) {
      ASSERT( aViewType == GpuResourceViewType::RTV, "The backbuffer only has an RTV" );
      return myBuilder->myRenderOutput->GetBackbufferRtv();
    }

    const uint baseIndex = myBuilder->GetBaseIndex( aHandle );
    ASSERT( myBuilder->GetResource( baseIndex )->IsTexture() == anIsTexture, "Resource %s is a %s", base.myName.c_str(),
            anIsTexture ? "buffer" : "texture" );
    return myBuilder->GetCreateView( baseIndex, aHandle.mySubresources, access->myAccess );
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
    ASSERT( FindAccess( aHandle ) != nullptr, "Task %s didn't declare an access of resource %s",
            myBuilder->mySetupData.myTasks[ myTaskIndex ].myName.c_str(), myBuilder->GetBase( aHandle ).myName.c_str() );

    return myBuilder->GetResource( myBuilder->GetBaseIndex( aHandle ) );
  }
  //---------------------------------------------------------------------------//
  FrameGraphBuilder::FrameGraphBuilder( RenderOutput * aRenderOutput ) : myRenderOutput( aRenderOutput ) {
    ASSERT( myRenderOutput != nullptr );
  }
  //---------------------------------------------------------------------------//
  FrameGraphBuilder::~FrameGraphBuilder() {
    ResetAndBeginSetup();
    ReleaseCompiledGraph();
  }
  //---------------------------------------------------------------------------//
  uint FrameGraphBuilder::GetBaseIndex( FgResourceHandle aHandle ) const {
    return mySetupData.myResourceVersions[ aHandle.myVersionIndex ].myBaseIndex;
  }
  //---------------------------------------------------------------------------//
  const FrameGraphResourceBase & FrameGraphBuilder::GetBase( FgResourceHandle aHandle ) const {
    return mySetupData.myResourceBases[ GetBaseIndex( aHandle ) ];
  }
  //---------------------------------------------------------------------------//
  GpuResource * FrameGraphBuilder::GetResource( uint aBaseIndex ) const {
    const FrameGraphResourceBase & base = mySetupData.myResourceBases[ aBaseIndex ];
    return base.myIsBackBuffer ? myRenderOutput->GetBackbuffer() : base.myResource;
  }
  //---------------------------------------------------------------------------//
  uint FrameGraphBuilder::GetSubresourceIndex( const SetupData & aSetupData, const BuildData & aBuildData, uint aBaseIndex,
                                                const SubresourceLocation & aLocation ) {
    const SubresourceRange & subresources = aSetupData.myResourceBases[ aBaseIndex ].mySubresources;
    return aBuildData.myFirstSubresourceIndices[ aBaseIndex ] +
           GpuResource::CalcSubresourceIndex( aLocation.myMipLevel, subresources.myNumMipLevels, aLocation.myArrayIndex,
                                              subresources.myNumArrayIndices, aLocation.myPlaneIndex );
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::RegisterExisting( TextureHandle aTexture ) {
    Texture * texture = RenderCore::GetTexture( aTexture );
    ASSERT( texture != nullptr, "Invalid or outdated texture handle" );
    if ( texture == nullptr ) {
      return FgResourceHandle();
    }
    return RegisterResource( texture, texture->GetName(), aTexture.myIndex, aTexture.myGeneration, false );
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::RegisterExisting( GpuBufferHandle aBuffer ) {
    GpuBuffer * buffer = RenderCore::GetBuffer( aBuffer );
    ASSERT( buffer != nullptr, "Invalid or outdated buffer handle" );
    if ( buffer == nullptr ) {
      return FgResourceHandle();
    }
    return RegisterResource( buffer, buffer->GetName(), aBuffer.myIndex, aBuffer.myGeneration, false );
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::RegisterBackBuffer() {
    return RegisterResource( myRenderOutput->GetBackbuffer(), "Backbuffer", kFgInvalidIndex, 0u, true );
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::RegisterResource( GpuResource * aResource, const char * aName, uint aHandleIndex, uint aHandleGeneration,
                                                        bool anIsBackBuffer ) {
    ASSERT( aResource != nullptr );
    ASSERT( !myIsDeclarationBuilt, "Can't register resources after the graph has been built. Call ResetAndBeginSetup() first" );
    for ( uint baseIndex = 0u, numBases = ( uint ) mySetupData.myResourceBases.size(); baseIndex < numBases; ++baseIndex ) {
      ASSERT( GetResource( baseIndex ) != aResource, "Resource %s is already registered", aName );
    }

    FrameGraphResourceBase base;
    base.myName = aName;
    base.myResource = anIsBackBuffer ? nullptr : aResource;
    base.mySubresources = aResource->GetSubresources();
    base.myHandleIndex = aHandleIndex;
    base.myHandleGeneration = aHandleGeneration;
    base.myIsBackBuffer = anIsBackBuffer;
    mySetupData.myResourceBases.push_back( base );

    // Version 0 is the state before the graph: nothing written, no producer
    const uint versionIndex = ( uint ) mySetupData.myResourceVersions.size();
    FrameGraphResourceVersion version;
    version.myWrittenRange = SubresourceRange( 0u, 0u, 0u, 0u, 0u, 0u );
    version.myBaseIndex = ( uint ) mySetupData.myResourceBases.size() - 1u;
    mySetupData.myResourceVersions.push_back( version );
    mySetupData.myVersionProducerTasks.push_back( kFgInvalidIndex );
    mySetupData.myVersionNextIndices.push_back( kFgInvalidIndex );
    mySetupData.myLatestVersionIndices.push_back( versionIndex );

    FgResourceHandle handle;
    handle.mySubresources = base.mySubresources;
    handle.myVersionIndex = versionIndex;
    handle.myFrameGraphGenerationIndex = mySetupData.myGenerationIndex;
    return handle;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::ResetAndBeginSetup() {
    mySetupData.myTaskAllocator.Reset();
    mySetupData.myTasks.clear();
    mySetupData.myResourceBases.clear();
    mySetupData.myResourceVersions.clear();
    mySetupData.myLatestVersionIndices.clear();
    mySetupData.myVersionProducerTasks.clear();
    mySetupData.myVersionNextIndices.clear();
    ++mySetupData.myGenerationIndex;

    // The build results are kept: BuildFrameGraph() reuses them if the new declaration is equal
    myIsDeclarationBuilt = false;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::DiscardBuildResults() {
    myBuildData.myFirstSubresourceIndices.clear();
    myBuildData.mySubresourceStates.clear();
    myBuildData.myDependencies.clear();
    myBuildData.myTaskOrder.clear();
    myBuildData.myDeclarationHash = 0u;
    myBuildData.myHasResults = false;
    myIsDeclarationBuilt = false;
  }
  //---------------------------------------------------------------------------//
  uint FrameGraphBuilder::AddTask( const FgTaskCreateInfo & aCreateInfo, void ** aTaskMemoryOut ) {
    ASSERT( !myIsDeclarationBuilt, "Can't add tasks after the graph has been built. Call ResetAndBeginSetup() first" );
    ASSERT( aCreateInfo.myExecuteFunction != nullptr && aCreateInfo.myCompileFunction != nullptr );

    void * memory = mySetupData.myTaskAllocator.Allocate( aCreateInfo.myTaskSize, aCreateInfo.myTaskAlignment );
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
    mySetupData.myTasks.push_back( task );
    return ( uint ) mySetupData.myTasks.size() - 1u;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::ValidateHandle( FgResourceHandle aHandle ) const {
    ASSERT( aHandle.IsValid() );
    ASSERT( aHandle.myFrameGraphGenerationIndex == mySetupData.myGenerationIndex, "Resource handle from generation %d used in generation %d",
            aHandle.myFrameGraphGenerationIndex, mySetupData.myGenerationIndex );
    ASSERT( aHandle.myVersionIndex < mySetupData.myResourceVersions.size() );
    if ( !aHandle.IsValid() || aHandle.myVersionIndex >= mySetupData.myResourceVersions.size() ) {
      return;
    }
    ASSERT( GetBase( aHandle ).mySubresources.Contains( aHandle.mySubresources ), "Handle to %s lies outside of the resource",
            GetBase( aHandle ).myName.c_str() );
  }
  //---------------------------------------------------------------------------//
  // A task has at most one access per subresource. The graph only synchronizes between tasks: a subresource can be in only one
  // state while a task runs, a read and a write of it would make the task depend on itself, and FindAccess() needs a unique match.
  // A task that reads back what it writes (e.g. a mip chain in one task) declares a single write and synchronizes internally.
  // Depth accesses bind a DSV, which covers all planes, so they have to cover all planes as well. Shader reads can use single planes.
  //---------------------------------------------------------------------------//
  bool FrameGraphBuilder::AddAccess( uint aTaskIndex, FgResourceHandle aHandle, ResourceAccess anAccess ) {
    ASSERT( !myIsDeclarationBuilt, "Can't change tasks after the graph has been built. Call ResetAndBeginSetup() first" );
    ASSERT( aTaskIndex < mySetupData.myTasks.size(), "Invalid task index %u", aTaskIndex );
    ASSERT( anAccess != RESOURCE_ACCESS_NONE, "Missing access for resource %s", GetBase( aHandle ).myName.c_str() );
    if ( aTaskIndex >= mySetupData.myTasks.size() || anAccess == RESOURCE_ACCESS_NONE ) {
      return false;
    }

    FrameGraphTask & task = mySetupData.myTasks[ aTaskIndex ];

    for ( const FrameGraphTaskAccess & access : task.myAccesses ) {
      ASSERT( GetBaseIndex( access.myHandle ) != GetBaseIndex( aHandle ) || !access.myHandle.mySubresources.Overlaps( aHandle.mySubresources ),
              "Task %s accesses overlapping subresources of %s more than once", task.myName.c_str(), GetBase( aHandle ).myName.c_str() );
    }

    if ( anAccess == RESOURCE_ACCESS_DEPTH_WRITE || anAccess == RESOURCE_ACCESS_DEPTH_READ ) {
      const SubresourceRange & resourceRange = GetBase( aHandle ).mySubresources;
      ASSERT( aHandle.mySubresources.myFirstPlane == 0u && aHandle.mySubresources.myNumPlanes == resourceRange.myNumPlanes,
              "Depth access to %s needs to cover all planes", GetBase( aHandle ).myName.c_str() );
    }

    FrameGraphTaskAccess access;
    access.myHandle = aHandle;
    access.myAccess = anAccess;
    task.myAccesses.push_back( access );
    return true;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::Read( uint aTaskIndex, FgResourceHandle aHandle, ResourceAccess anAccess ) {
    ValidateHandle( aHandle );
    if ( !aHandle.IsValid() || aHandle.myVersionIndex >= mySetupData.myResourceVersions.size() ) {
      return;
    }
    ASSERT( !RenderCore::IsWriteAccess( anAccess ), "Read() needs a read access, use Write() for %s", RenderCore::GetResourceAccessName( anAccess ) );
    AddAccess( aTaskIndex, aHandle, anAccess );
  }
  //---------------------------------------------------------------------------//
  FgResourceHandle FrameGraphBuilder::Write( uint aTaskIndex, FgResourceHandle aHandle, ResourceAccess anAccess ) {
    ValidateHandle( aHandle );
    if ( !aHandle.IsValid() || aHandle.myVersionIndex >= mySetupData.myResourceVersions.size() ) {
      return FgResourceHandle();
    }
    ASSERT( RenderCore::IsWriteAccess( anAccess ), "Write() needs a write access, use Read() for %s", RenderCore::GetResourceAccessName( anAccess ) );

    // Writing on top of an older version is fine as long as no later version changed the written subresources
    const uint baseIndex = GetBaseIndex( aHandle );
    for ( uint versionIndex = mySetupData.myVersionNextIndices[ aHandle.myVersionIndex ]; versionIndex != kFgInvalidIndex;
          versionIndex = mySetupData.myVersionNextIndices[ versionIndex ] ) {
      const FrameGraphResourceVersion & laterVersion = mySetupData.myResourceVersions[ versionIndex ];
      const bool hasConflict = laterVersion.myWrittenRange.Overlaps( aHandle.mySubresources );
      ASSERT( !hasConflict, "Task %u writes subresources of %s that a later version (v%u, written by task %u) already changed", aTaskIndex,
              mySetupData.myResourceBases[ baseIndex ].myName.c_str(), laterVersion.myVersion, mySetupData.myVersionProducerTasks[ versionIndex ] );
      if ( hasConflict ) {
        return FgResourceHandle();
      }
    }

    if ( !AddAccess( aTaskIndex, aHandle, anAccess ) ) {
      return FgResourceHandle();
    }

    const uint latestVersionIndex = mySetupData.myLatestVersionIndices[ baseIndex ];
    const uint newVersionIndex = ( uint ) mySetupData.myResourceVersions.size();

    FrameGraphResourceVersion newVersion;
    newVersion.myWrittenRange = aHandle.mySubresources;
    newVersion.myBaseIndex = baseIndex;
    newVersion.myVersion = mySetupData.myResourceVersions[ latestVersionIndex ].myVersion + 1u;
    mySetupData.myResourceVersions.push_back( newVersion );
    mySetupData.myVersionProducerTasks.push_back( aTaskIndex );
    mySetupData.myVersionNextIndices.push_back( kFgInvalidIndex );
    mySetupData.myVersionNextIndices[ latestVersionIndex ] = newVersionIndex;
    mySetupData.myLatestVersionIndices[ baseIndex ] = newVersionIndex;

    mySetupData.myTasks[ aTaskIndex ].myAccesses.back().myWrittenVersionIndex = newVersionIndex;

    FgResourceHandle result;
    result.mySubresources = mySetupData.myResourceBases[ baseIndex ].mySubresources;
    result.myVersionIndex = newVersionIndex;
    result.myFrameGraphGenerationIndex = mySetupData.myGenerationIndex;
    return result;
  }
  //---------------------------------------------------------------------------//
  bool FrameGraphBuilder::BuildFrameGraph() {
    ASSERT( !myIsDeclarationBuilt, "The graph has already been built. Call ResetAndBeginSetup() or DiscardBuildResults() first" );

    // The build results only depend on the declaration, so the ones of an equal declaration can be kept
    const uint64 declarationHash = ComputeDeclarationHash();
    if ( myBuildData.myHasResults && myBuildData.myDeclarationHash == declarationHash ) {
#if FANCY_RENDERER_DEBUG_FRAMEGRAPH
      VerifyReusedBuild( mySetupData, myBuildData );
#endif
      myIsDeclarationBuilt = true;
      return true;
    }

    myBuildData.myDeclarationHash = declarationHash;
    myBuildData.myHasResults = Build( mySetupData, myBuildData );
    myIsDeclarationBuilt = myBuildData.myHasResults;
    return myBuildData.myHasResults;
  }
  //---------------------------------------------------------------------------//
  bool FrameGraphBuilder::Build( const SetupData & aSetupData, BuildData & aBuildData ) {
    using namespace Priv_FrameGraphBuilder;

    const eastl::fixed_vector< FrameGraphTask, kFgMaxNumTasks > & tasks = aSetupData.myTasks;
    const eastl::fixed_vector< FrameGraphResourceVersion, kFgMaxNumResourceVersions > & versions = aSetupData.myResourceVersions;
    const uint numTasks = ( uint ) tasks.size();
    const uint numVersions = ( uint ) versions.size();
    const uint numBases = ( uint ) aSetupData.myResourceBases.size();

    eastl::fixed_vector< FrameGraphDependency, kFgMaxNumTasks > & dependencies = aBuildData.myDependencies;
    dependencies.clear();
    aBuildData.myFirstSubresourceIndices.resize( numBases );
    uint numSubresources = 0u;
    for ( uint baseIndex = 0u; baseIndex < numBases; ++baseIndex ) {
      aBuildData.myFirstSubresourceIndices[ baseIndex ] = numSubresources;
      numSubresources += aSetupData.myResourceBases[ baseIndex ].mySubresources.GetNumSubresources();
    }
    aBuildData.mySubresourceStates.clear();
    aBuildData.mySubresourceStates.resize( numSubresources );

    // The reads of each version, as linked lists in a flat array
    eastl::fixed_vector< FgVersionRead, kFgMaxNumReadsPerBuild > versionReads;
    eastl::fixed_vector< uint, kFgMaxNumResourceVersions > firstReadOfVersion( numVersions, kFgInvalidIndex );
    for ( uint taskIdx = 0u; taskIdx < numTasks; ++taskIdx ) {
      for ( const FrameGraphTaskAccess & access : tasks[ taskIdx ].myAccesses ) {
        if ( RenderCore::IsWriteAccess( access.myAccess ) ) {
          continue;
        }

        const uint versionIndex = access.myHandle.myVersionIndex;
        FgVersionRead read;
        read.myTaskIndex = taskIdx;
        read.myRange = access.myHandle.mySubresources;
        read.myNextRead = firstReadOfVersion[ versionIndex ];
        firstReadOfVersion[ versionIndex ] = ( uint ) versionReads.size();
        versionReads.push_back( read );
      }
    }

    // Replay the accesses of each resource in version order: for each version, first the write that created it, then all reads of it. A version
    // is the whole resource, so at that point the per-subresource state holds exactly the contents the version's readers see. Per subresource, a
    // read depends on the last writer (read after write), a write on the last writer (write after write) and on all readers since (write after
    // read). The versions of a resource are appended in chain order, so the order of the versions array is the version order of every resource.
    for ( uint versionIndex = 0u; versionIndex < numVersions; ++versionIndex ) {
      const FrameGraphResourceVersion & version = versions[ versionIndex ];
      const uint producerTask = aSetupData.myVersionProducerTasks[ versionIndex ];

      if ( producerTask != kFgInvalidIndex ) {
        for ( SubresourceIterator it = version.myWrittenRange.Begin(); it != version.myWrittenRange.End(); ++it ) {
          FgSubresourceState & state = aBuildData.mySubresourceStates[ GetSubresourceIndex( aSetupData, aBuildData, version.myBaseIndex, *it ) ];
          locAddDependency( dependencies, state.myWriterTask, producerTask );
          for ( uint reader : state.myReaderTasks ) {
            locAddDependency( dependencies, reader, producerTask );
          }
          state.myWriterTask = producerTask;
          state.myReaderTasks.clear();
        }
      }

      for ( uint readIndex = firstReadOfVersion[ versionIndex ]; readIndex != kFgInvalidIndex; readIndex = versionReads[ readIndex ].myNextRead ) {
        const FgVersionRead & read = versionReads[ readIndex ];
        for ( SubresourceIterator it = read.myRange.Begin(); it != read.myRange.End(); ++it ) {
          FgSubresourceState & state = aBuildData.mySubresourceStates[ GetSubresourceIndex( aSetupData, aBuildData, version.myBaseIndex, *it ) ];
          locAddDependency( dependencies, state.myWriterTask, read.myTaskIndex );
          state.myReaderTasks.push_back( read.myTaskIndex );
        }
      }
    }

    // Kahn's algorithm, picking the earliest added ready task. Dependencies can point backwards in task order, e.g. when a task reads an old version.
    eastl::fixed_vector< uint, kFgMaxNumTasks > & taskOrder = aBuildData.myTaskOrder;
    eastl::fixed_vector< uint, kFgMaxNumTasks > numIncomingEdges( numTasks, 0u );
    for ( const FrameGraphDependency & dependency : dependencies ) {
      ++numIncomingEdges[ dependency.myDstTask ];
    }

    eastl::fixed_vector< bool, kFgMaxNumTasks > isScheduled( numTasks, false );
    taskOrder.clear();

    for ( uint numScheduled = 0u; numScheduled < numTasks; ++numScheduled ) {
      uint nextTask = kFgInvalidIndex;
      for ( uint taskIdx = 0u; taskIdx < numTasks && nextTask == kFgInvalidIndex; ++taskIdx ) {
        if ( !isScheduled[ taskIdx ] && numIncomingEdges[ taskIdx ] == 0u ) {
          nextTask = taskIdx;
        }
      }

      if ( nextTask == kFgInvalidIndex ) {
        LOG_ERROR( "FrameGraph contains a cycle" );
        taskOrder.clear();
        return false;
      }

      isScheduled[ nextTask ] = true;
      taskOrder.push_back( nextTask );

      for ( const FrameGraphDependency & dependency : dependencies ) {
        if ( dependency.mySrcTask == nextTask ) {
          --numIncomingEdges[ dependency.myDstTask ];
        }
      }
    }

    return true;
  }
  //---------------------------------------------------------------------------//
#if FANCY_RENDERER_DEBUG_FRAMEGRAPH
  void FrameGraphBuilder::VerifyReusedBuild( const SetupData & aSetupData, const BuildData & aBuildData ) {
    // Equal declaration hashes have to produce equal build results. The builder is large, so keep it off the stack.
    UniquePtr< BuildData > rebuilt( new BuildData() );
    const bool isBuilt = Build( aSetupData, *rebuilt );
    ASSERT( isBuilt, "Reused FrameGraph build results: the declaration doesn't build anymore" );
    ASSERT( rebuilt->myTaskOrder == aBuildData.myTaskOrder, "Reused FrameGraph build results: the task order differs" );
    ASSERT( rebuilt->myFirstSubresourceIndices == aBuildData.myFirstSubresourceIndices,
            "Reused FrameGraph build results: the subresource layout differs" );

    // The dependencies can be found in a different order, but each one only once
    ASSERT( rebuilt->myDependencies.size() == aBuildData.myDependencies.size(), "Reused FrameGraph build results: the dependencies differ" );
    for ( const FrameGraphDependency & rebuiltDependency : rebuilt->myDependencies ) {
      bool isKnown = false;
      for ( const FrameGraphDependency & dependency : aBuildData.myDependencies ) {
        isKnown |= dependency.mySrcTask == rebuiltDependency.mySrcTask && dependency.myDstTask == rebuiltDependency.myDstTask;
      }
      ASSERT( isKnown, "Reused FrameGraph build results: dependency %u -> %u is missing", rebuiltDependency.mySrcTask, rebuiltDependency.myDstTask );
    }
  }
#endif  // FANCY_RENDERER_DEBUG_FRAMEGRAPH
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::ReleaseCompiledGraph() {
    if ( !myCompiledData.myResources.empty() ) {
      // Recompiling is rare (structure or resource changes), so simply make sure no frame in flight uses the views
      RenderCore::WaitForIdle( CommandListType::Graphics );
      for ( const FrameGraphCompiledView & view : myCompiledData.myViews ) {
        if ( view.myTextureView.IsValid() ) {
          RenderCore::DeleteTextureView( view.myTextureView );
        }
        if ( view.myBufferView.IsValid() ) {
          RenderCore::DeleteBufferView( view.myBufferView );
        }
      }
      myCompiledData.myViews.clear();
      myCompiledData.myResources.clear();
    }

    myCompiledData.myRenderDataAllocator.Reset();
    myCompiledData.myTasks.clear();
    myCompiledData.myInitialAccesses.clear();
    myCompiledData.myFinalBarriers.clear();
    myCompiledData.myDeclarationHash = 0u;
    myCompiledData.myHasResults = false;
  }
  //---------------------------------------------------------------------------//
  GpuResourceView * FrameGraphBuilder::GetCreateView( uint aBaseIndex, const SubresourceRange & aRange, ResourceAccess anAccess ) {
    using namespace Priv_FrameGraphBuilder;

    const FrameGraphResourceBase & base = mySetupData.myResourceBases[ aBaseIndex ];
    GpuResource * resource = GetResource( aBaseIndex );
    eastl::fixed_vector< FrameGraphCompiledView, kFgMaxNumResourceViewsPerFrame, false > & views = myCompiledData.myViews;
    const GpuResourceViewType viewType = RenderCore::GetResourceAccessViewType( anAccess );
    const FgViewKind viewKind = locGetViewKind( anAccess );
    const bool isReadOnlyDepth = viewKind == FG_VIEW_KIND_READ_ONLY_DSV;
    const bool isTexture = resource->IsTexture();
    ASSERT( viewKind != FG_VIEW_KIND_NUM, "Access %s doesn't need a view", RenderCore::GetResourceAccessName( anAccess ) );
    ASSERT( aBaseIndex < myCompiledData.myResources.size() );

    FrameGraphCompiledResource & compiledResource = myCompiledData.myResources[ aBaseIndex ];
    ASSERT( compiledResource.myHandleIndex == base.myHandleIndex && compiledResource.myHandleGeneration == base.myHandleGeneration,
            "Resource %s changed since the graph was compiled", base.myName.c_str() );
    for ( uint viewIndex = compiledResource.myFirstViewIndex; viewIndex != kFgInvalidIndex; viewIndex = views[ viewIndex ].myNextViewIndex ) {
      const FrameGraphCompiledView & existingView = views[ viewIndex ];
      if ( existingView.myKind == viewKind && existingView.mySubresources == aRange ) {
        return isTexture ? static_cast< GpuResourceView * >( RenderCore::GetTextureView( existingView.myTextureView ) )
                         : RenderCore::GetBufferView( existingView.myBufferView );
      }
    }

    ASSERT( !views.full(), "Too many views in the graph. Increase kFgMaxNumResourceViewsPerFrame" );
    if ( views.full() ) {
      return nullptr;
    }

    FrameGraphCompiledView view;
    view.myKind = viewKind;
    view.mySubresources = aRange;
    view.myNextViewIndex = compiledResource.myFirstViewIndex;

    const char * viewTypeName = "SRV";
    if ( isTexture ) {
      Texture * texture = static_cast< Texture * >( resource );
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
      compiledResource.myFirstViewIndex = ( uint ) views.size();
      views.push_back( view );
      return RenderCore::GetTextureView( view.myTextureView );
    }

    GpuBuffer * buffer = static_cast< GpuBuffer * >( resource );

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
    compiledResource.myFirstViewIndex = ( uint ) views.size();
    views.push_back( view );
    return RenderCore::GetBufferView( view.myBufferView );
  }
  //---------------------------------------------------------------------------//
  uint64 FrameGraphBuilder::ComputeDeclarationHash() const {
    const eastl::fixed_vector< FrameGraphResourceVersion, kFgMaxNumResourceVersions > & versions = mySetupData.myResourceVersions;
    uint64 hash = 0u;

    // The counts separate the sequence of resources from the sequence of tasks
    MathUtil::hash_combine( hash, ( uint64 ) mySetupData.myResourceBases.size() );
    MathUtil::hash_combine( hash, ( uint64 ) mySetupData.myTasks.size() );

    // The compiled views belong to the registered resources, identified by their engine handles. Recreating a resource changes the handle's
    // generation. The backbuffer has no handle, its swap chain image changes every frame and is resolved when executing instead.
    for ( uint baseIndex = 0u, numBases = ( uint ) mySetupData.myResourceBases.size(); baseIndex < numBases; ++baseIndex ) {
      const FrameGraphResourceBase & base = mySetupData.myResourceBases[ baseIndex ];
      MathUtil::hash_combine( hash, ( uint64 ) GetResource( baseIndex )->IsTexture() );
      MathUtil::hash_combine( hash, ( uint64 ) base.myIsBackBuffer );
      MathUtil::hash_combine( hash, ( uint64 ) base.myHandleIndex );
      MathUtil::hash_combine( hash, ( uint64 ) base.myHandleGeneration );
    }

    for ( const FrameGraphTask & task : mySetupData.myTasks ) {
      MathUtil::hash_combine( hash, MathUtil::Hash( task.myName.c_str() ) );
      MathUtil::hash_combine( hash, ( uint64 ) task.myQueueType );
      MathUtil::hash_combine( hash, reinterpret_cast< uint64 >( task.myExecuteFunction ) );
      MathUtil::hash_combine( hash, ( uint64 ) task.myAccesses.size() );

      for ( const FrameGraphTaskAccess & access : task.myAccesses ) {
        const SubresourceRange & range = access.myHandle.mySubresources;
        MathUtil::hash_combine( hash, ( uint64 ) GetBaseIndex( access.myHandle ) );
        MathUtil::hash_combine( hash, ( uint64 ) versions[ access.myHandle.myVersionIndex ].myVersion );
        // Which version a write creates depends on the order of the Write() calls
        if ( access.myWrittenVersionIndex != kFgInvalidIndex ) {
          MathUtil::hash_combine( hash, ( uint64 ) versions[ access.myWrittenVersionIndex ].myVersion );
        }
        MathUtil::hash_combine( hash, ( uint64 ) range.myFirstMipLevel );
        MathUtil::hash_combine( hash, ( uint64 ) range.myNumMipLevels );
        MathUtil::hash_combine( hash, ( uint64 ) range.myFirstArrayIndex );
        MathUtil::hash_combine( hash, ( uint64 ) range.myNumArrayIndices );
        MathUtil::hash_combine( hash, ( uint64 ) range.myFirstPlane );
        MathUtil::hash_combine( hash, ( uint64 ) range.myNumPlanes );
        MathUtil::hash_combine( hash, ( uint64 ) access.myAccess );
      }
    }

    return hash;
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::Compile() {
    using namespace Priv_FrameGraphBuilder;

    if ( myCompiledData.myHasResults && myCompiledData.myDeclarationHash == myBuildData.myDeclarationHash ) {
      return;
    }

    ReleaseCompiledGraph();

    const eastl::fixed_vector< FrameGraphTask, kFgMaxNumTasks > & tasks = mySetupData.myTasks;
    const eastl::fixed_vector< FrameGraphResourceBase, kFgMaxNumResources > & bases = mySetupData.myResourceBases;
    const uint numBases = ( uint ) bases.size();

    myCompiledData.myTasks.resize( tasks.size() );
    myCompiledData.myResources.resize( numBases );
    for ( uint baseIndex = 0u; baseIndex < numBases; ++baseIndex ) {
      myCompiledData.myResources[ baseIndex ].myHandleIndex = bases[ baseIndex ].myHandleIndex;
      myCompiledData.myResources[ baseIndex ].myHandleGeneration = bases[ baseIndex ].myHandleGeneration;
    }

    eastl::fixed_vector< FgSubresourceAccesses, kFgMaxNumSubresources > subresourceAccesses( myBuildData.mySubresourceStates.size() );

    for ( uint taskIdx : myBuildData.myTaskOrder ) {
      const FrameGraphTask & task = tasks[ taskIdx ];
      FrameGraphCompiledTask & compiledTask = myCompiledData.myTasks[ taskIdx ];

      compiledTask.myRenderData = myCompiledData.myRenderDataAllocator.Allocate( task.myRenderDataSize, task.myRenderDataAlignment );
      ASSERT( compiledTask.myRenderData != nullptr, "Out of render data memory. Increase kFgTaskMemorySize" );
      CompileTask( taskIdx );

      for ( const FrameGraphTaskAccess & access : task.myAccesses ) {
        const uint baseIndex = GetBaseIndex( access.myHandle );
        const bool isWrite = RenderCore::IsWriteAccess( access.myAccess );
        compiledTask.myAccessesBackBuffer |= bases[ baseIndex ].myIsBackBuffer;

        eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumBarriersPerTask > barriers;
        for ( SubresourceIterator it = access.myHandle.mySubresources.Begin(); it != access.myHandle.mySubresources.End(); ++it ) {
          FgSubresourceAccesses & accesses = subresourceAccesses[ GetSubresourceIndex( mySetupData, myBuildData, baseIndex, *it ) ];

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
    for ( uint baseIndex = 0u; baseIndex < numBases; ++baseIndex ) {
      const GpuResource * resource = GetResource( baseIndex );
      const uint firstSubresourceIndex = myBuildData.myFirstSubresourceIndices[ baseIndex ];
      const uint numSubresources = resource->GetSubresources().GetNumSubresources();

      eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumBarriersPerTask > initialAccesses;
      eastl::fixed_vector< FrameGraphBarrier, kFgMaxNumBarriersPerTask > finalBarriers;
      for ( uint subresourceIndex = 0u; subresourceIndex < numSubresources; ++subresourceIndex ) {
        const FgSubresourceAccesses & accesses = subresourceAccesses[ firstSubresourceIndex + subresourceIndex ];
        if ( accesses.myFirstAccess == RESOURCE_ACCESS_NONE ) {
          continue;
        }

        FrameGraphBarrier barrier;
        barrier.mySubresources = SubresourceRange( resource->GetSubresourceLocation( subresourceIndex ) );
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
      myCompiledData.myInitialAccesses.insert( myCompiledData.myInitialAccesses.end(), initialAccesses.begin(), initialAccesses.end() );
      myCompiledData.myFinalBarriers.insert( myCompiledData.myFinalBarriers.end(), finalBarriers.begin(), finalBarriers.end() );
    }

    myCompiledData.myDeclarationHash = myBuildData.myDeclarationHash;
    myCompiledData.myHasResults = true;
    ++myNumCompiles;

    LOG_INFO( "FrameGraph compiled (%d tasks, %d views)", ( int ) tasks.size(), ( int ) myCompiledData.myViews.size() );
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::CompileTask( uint aTaskIndex ) {
    const FrameGraphTask & task = mySetupData.myTasks[ aTaskIndex ];
    const FrameGraphCompileContext context( this, aTaskIndex );
    task.myCompileFunction( task.myInstance, context, myCompiledData.myTasks[ aTaskIndex ].myRenderData );
  }
  //---------------------------------------------------------------------------//
  void FrameGraphBuilder::Execute() {
    const eastl::fixed_vector< FrameGraphTask, kFgMaxNumTasks > & tasks = mySetupData.myTasks;
    ASSERT( myIsDeclarationBuilt, "BuildFrameGraph() needs to succeed before Execute()" );
    if ( !myIsDeclarationBuilt || tasks.empty() ) {
      return;
    }

    Compile();

    // The swap chain image changes every frame
    for ( uint taskIdx : myBuildData.myTaskOrder ) {
      if ( myCompiledData.myTasks[ taskIdx ].myAccessesBackBuffer ) {
        CompileTask( taskIdx );
      }
    }

    // Hand the resources over from the legacy hazard tracking: afterwards, each subresource is in the state of its first access in the graph.
    // Since the graph restores that state at its end, the tracked state stays correct without the graph's list having to report anything.
    CommandList * handoverCtx = RenderCore::BeginCommandList( CommandListType::Graphics );
    for ( const FrameGraphBarrier & initialAccess : myCompiledData.myInitialAccesses ) {
      handoverCtx->TrackResourceAccess( GetResource( initialAccess.myBaseIndex ), initialAccess.mySubresources, initialAccess.myDstAccess );
    }
    RenderCore::ExecuteAndFreeCommandList( handoverCtx );

    CommandList * ctx = RenderCore::BeginCommandList( CommandListType::Graphics, false );

    for ( uint taskIdx : myBuildData.myTaskOrder ) {
      const FrameGraphTask & task = tasks[ taskIdx ];
      const FrameGraphCompiledTask & compiledTask = myCompiledData.myTasks[ taskIdx ];

      ctx->BeginMarkerRegion( task.myName.c_str() );

      for ( const FrameGraphBarrier & barrier : compiledTask.myBarriers ) {
        ctx->TransitionResourceForAccess( GetResource( barrier.myBaseIndex ), barrier.mySubresources, barrier.mySrcAccess, barrier.myDstAccess );
      }
      // The compiled barriers belong to this task and are issued right before its work. Flushing them here keeps them apart from barriers the
      // task records itself (e.g. Vulkan clears) and from the next task's barriers on the same subresources.
      ctx->FlushBarriers();

      task.myExecuteFunction( ctx, task.myInstance, compiledTask.myRenderData );

      ctx->EndMarkerRegion();
    }

    for ( const FrameGraphBarrier & barrier : myCompiledData.myFinalBarriers ) {
      ctx->TransitionResourceForAccess( GetResource( barrier.myBaseIndex ), barrier.mySubresources, barrier.mySrcAccess, barrier.myDstAccess );
    }
    ctx->FlushBarriers();

    RenderCore::ExecuteAndFreeCommandList( ctx );
  }
  //---------------------------------------------------------------------------//
  bool FrameGraphBuilder::IsDeclarationBuilt() const {
    return myIsDeclarationBuilt;
  }
  //---------------------------------------------------------------------------//
  uint64 FrameGraphBuilder::GetDeclarationHash() const {
    return myBuildData.myDeclarationHash;
  }
  //---------------------------------------------------------------------------//
  uint FrameGraphBuilder::GetNumCompiles() const {
    return myNumCompiles;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< FrameGraphTask, kFgMaxNumTasks > & FrameGraphBuilder::GetTasks() const {
    return mySetupData.myTasks;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< FrameGraphResourceBase, kFgMaxNumResources > & FrameGraphBuilder::GetResourceBases() const {
    return mySetupData.myResourceBases;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< FrameGraphResourceVersion, kFgMaxNumResourceVersions > & FrameGraphBuilder::GetResourceVersions() const {
    return mySetupData.myResourceVersions;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< uint, kFgMaxNumResourceVersions > & FrameGraphBuilder::GetVersionProducerTasks() const {
    return mySetupData.myVersionProducerTasks;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< uint, kFgMaxNumResources > & FrameGraphBuilder::GetLatestVersionIndices() const {
    return mySetupData.myLatestVersionIndices;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< FrameGraphDependency, kFgMaxNumTasks > & FrameGraphBuilder::GetDependencies() const {
    return myBuildData.myDependencies;
  }
  //---------------------------------------------------------------------------//
  const eastl::fixed_vector< uint, kFgMaxNumTasks > & FrameGraphBuilder::GetTaskOrder() const {
    return myBuildData.myTaskOrder;
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy
