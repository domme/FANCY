#include "stdafx.h"
#include "DX12Prerequisites.h"
#include "CommandQueueDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy
{
  CommandQueueDX12::CommandQueueDX12(CommandListType aType)
    : CommandQueue(aType)
  {
	  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	  switch (aType) 
	  { 
		  case CommandListType::Graphics: 
			  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; 
		  break;
		  case CommandListType::Compute: 
			  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			break;
		  case CommandListType::DMA: 
			  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY; 
			break;
		  default: ASSERT(false);
	  }
	  
	  CheckD3Dcall(RenderCore::GetPlatformDX12()->GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&myQueue)));

	  myFence.reset(new GpuFenceDX12(aType));
  }
}


