#include "fancy_core_precompile.h"
#include "CommandQueueDX12.h"

#include "Rendering/RenderCore.h"
#include "Debug/Log.h"

#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "CommandListDX12.h"
#include "GpuResourceDataDX12.h"
#include "DebugUtilsDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
  //---------------------------------------------------------------------------//
  CommandQueueDX12::CommandQueueDX12( CommandListType aType )
      : CommandQueue( aType ), myFenceCompletedEvent( nullptr ) {
    ID3D12Device * device = RenderCore::GetPlatformDX12()->GetDevice();

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = Adapter::ResolveCommandListType( aType );
    ASSERT_HRESULT( device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &myQueue ) ) );

    ASSERT_HRESULT( device->CreateFence( myLastCompletedFenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &myFence ) ) );
  }
  //---------------------------------------------------------------------------//
  bool CommandQueueDX12::IsFenceDone( uint64 aFenceVal ) {
    ASSERT( GetCommandListType( aFenceVal ) == myType, "Can't compare against fence-value from different timeline" );

    if ( myLastCompletedFenceVal < aFenceVal )
      myLastCompletedFenceVal = glm::max( myLastCompletedFenceVal, myFence->GetCompletedValue() );

    return myLastCompletedFenceVal >= aFenceVal;
  }
  //---------------------------------------------------------------------------//
  uint64 CommandQueueDX12::SignalAndIncrementFence() {
    ASSERT_HRESULT( myQueue->Signal( myFence.Get(), myNextFenceVal ) );
    return myNextFenceVal++;
  }
  //---------------------------------------------------------------------------//
  void CommandQueueDX12::WaitForFence( uint64 aFenceVal ) {
    ASSERT( GetCommandListType( aFenceVal ) == myType, "Can't compare against fence-value from different timeline" );

    if ( IsFenceDone( aFenceVal ) )
      return;

    ASSERT_HRESULT( myFence->SetEventOnCompletion( aFenceVal, myFenceCompletedEvent ) );
    WaitForSingleObject( myFenceCompletedEvent, INFINITE );
    myLastCompletedFenceVal = glm::max( myLastCompletedFenceVal, aFenceVal );
  }
  //---------------------------------------------------------------------------//
  void CommandQueueDX12::WaitForIdle() {
    const uint64 fenceVal = SignalAndIncrementFence();
    WaitForFence( fenceVal );
  }
  //---------------------------------------------------------------------------//
  void CommandQueueDX12::StallForQueue( const CommandQueue * aCommandQueue ) {
    const CommandQueueDX12 * otherQueue = static_cast< const CommandQueueDX12 * >( aCommandQueue );
    if ( otherQueue != this )
      ASSERT_HRESULT( myQueue->Wait( otherQueue->myFence.Get(), otherQueue->myNextFenceVal - 1u ) );
  }
  //---------------------------------------------------------------------------//
  void CommandQueueDX12::StallForFence( uint64 aFenceVal ) {
    const CommandQueueDX12 * otherQueue =
        static_cast< const CommandQueueDX12 * >( RenderCore::GetCommandQueue( aFenceVal ) );
    if ( otherQueue != this )
      ASSERT_HRESULT( myQueue->Wait( otherQueue->myFence.Get(), aFenceVal ) );
  }
  //---------------------------------------------------------------------------//
  uint64 CommandQueueDX12::ExecuteCommandListInternal( CommandList * aCommandList,
                                                       SyncMode      aSyncMode /* = SyncMode::ASYNC*/ ) {
    ASSERT( aCommandList->GetType() == myType );
    ASSERT( aCommandList->IsOpen() );

    ResolveResourceHazardData( aCommandList );

    aCommandList->Close();

    CommandListDX12 *   contextDx12 = ( CommandListDX12 * ) aCommandList;
    ID3D12CommandList * cmdList = contextDx12->myCommandList;
    myQueue->ExecuteCommandLists( 1, &cmdList );

    const uint64 fenceVal = SignalAndIncrementFence();
    aCommandList->PostExecute( fenceVal );

    if ( aSyncMode == SyncMode::BLOCKING || RenderCore::ourDebugWaitAfterEachSubmit )
      WaitForFence( fenceVal );

    return fenceVal;
  }
  //---------------------------------------------------------------------------//
  uint64 CommandQueueDX12::ExecuteAndResetCommandListInternal( CommandList * aContext, SyncMode aSyncMode ) {
    const uint64 fenceVal = ExecuteCommandListInternal( aContext, aSyncMode );
    aContext->ResetAndOpen();
    return fenceVal;
  }
  //---------------------------------------------------------------------------//
  void CommandQueueDX12::ResolveResourceHazardData( CommandList * aCommandList ) {
    eastl::fixed_vector< D3D12_TEXTURE_BARRIER, 64 > patchTexBarriers;
    eastl::fixed_vector< D3D12_BUFFER_BARRIER, 64 >  patchBufBarriers;

    CommandListDX12 * cmdListDx12 = static_cast< CommandListDX12 * >( aCommandList );

    for ( auto it : cmdListDx12->myLocalHazardData ) {
      const GpuResource * resource = it.first;
      const bool isTexture = resource->IsTexture();

      GpuResourceBarrierDataDX12 & globalData = resource->GetDX12Data()->myHazardData;
      const CommandListDX12::LocalHazardData & localData = it.second;
      ASSERT( globalData.mySubresources.size() == localData.mySubresources.size() );

      for ( uint subIdx = 0u; subIdx < ( uint ) localData.mySubresources.size(); ++subIdx ) {
        GpuSubresourceBarrierStateDX12 & globalSub = globalData.mySubresources[ subIdx ];
        const CommandListDX12::SubresourceHazardData & localSub = localData.mySubresources[ subIdx ];

        if ( !localSub.myWasUsed ) continue;

        const uint oldGlobalState = globalSub.myState;

        // Update global state tracking
        if ( localSub.myIsSharedReadState )
          globalSub.myContext = CommandListType::SHARED_READ;
        globalSub.myState = localSub.myState;
        if ( localSub.myWasWritten )
          globalSub.myContext = aCommandList->GetType();

        if ( oldGlobalState == localSub.myFirstDstState ) {
#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
          // TODO: Add logging with new state names
#endif
          continue;
        }

        if ( isTexture ) {
          TextureBarrierParams beforeParams = GetTextureBarrierParams( oldGlobalState );
          TextureBarrierParams afterParams  = GetTextureBarrierParams( localSub.myFirstDstState );
          D3D12_TEXTURE_BARRIER barrier = {};
          barrier.SyncBefore   = beforeParams.mySync;
          barrier.AccessBefore = beforeParams.myAccess;
          barrier.LayoutBefore = beforeParams.myLayout;
          barrier.SyncAfter    = afterParams.mySync;
          barrier.AccessAfter  = afterParams.myAccess;
          barrier.LayoutAfter  = afterParams.myLayout;
          barrier.pResource    = resource->GetDX12Data()->myResource.Get();
          barrier.Subresources.IndexOrFirstMipLevel = subIdx;
          barrier.Subresources.NumMipLevels = 1u;
          barrier.Subresources.FirstArraySlice = 0u;
          barrier.Subresources.NumArraySlices = 1u;
          barrier.Subresources.FirstPlane = 0u;
          barrier.Subresources.NumPlanes = 1u;
          barrier.Flags        = D3D12_TEXTURE_BARRIER_FLAG_NONE;
          patchTexBarriers.push_back( barrier );
        } else {
          BufferBarrierParams beforeParams = GetBufferBarrierParams( oldGlobalState );
          BufferBarrierParams afterParams  = GetBufferBarrierParams( localSub.myFirstDstState );
          D3D12_BUFFER_BARRIER barrier = {};
          barrier.SyncBefore   = beforeParams.mySync;
          barrier.AccessBefore = beforeParams.myAccess;
          barrier.SyncAfter    = afterParams.mySync;
          barrier.AccessAfter  = afterParams.myAccess;
          barrier.pResource    = resource->GetDX12Data()->myResource.Get();
          barrier.Offset       = 0;
          barrier.Size         = UINT64_MAX;
          patchBufBarriers.push_back( barrier );
        }
      }

      // Update myAllSubresourcesSameState
      bool allSame = true;
      uint firstState = globalData.mySubresources[ 0 ].myState;
      for ( uint s = 1u; allSame && s < ( uint ) globalData.mySubresources.size(); ++s )
        allSame = globalData.mySubresources[ s ].myState == firstState;
      globalData.myAllSubresourcesSameState = allSame;
    }

    if ( !patchTexBarriers.empty() || !patchBufBarriers.empty() ) {
      CommandList *     ctx = RenderCore::BeginCommandList( myType );
      CommandListDX12 * ctxDx12 = static_cast< CommandListDX12 * >( ctx );
      for ( uint i = 0; i < patchTexBarriers.size(); ++i ) {
        ctxDx12->AddTextureBarrier( patchTexBarriers[ i ] );
      }
      for ( uint i = 0; i < patchBufBarriers.size(); ++i ) {
        ctxDx12->AddBufferBarrier( patchBufBarriers[ i ] );
      }
      ctxDx12->FlushBarriers();
      ExecuteAndFreeCommandList( ctx );
    }
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy

#endif