#include "fancy_core_precompile.h"
#include "CommandQueueDX12.h"

#include "Rendering/RenderCore.h"
#include "Debug/Log.h"

#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "CommandListDX12.h"
#include "GpuResourceDataDX12.h"
#include "DebugUtilsDX12.h"
#include "BarrierUtilsDX12.h"

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
}  // namespace Fancy

#endif