#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include "d3dx12.h"
#include <dxgiformat.h>

namespace {
  void CheckD3Dcall(HRESULT aResult)
  {
    if (aResult != S_OK)
      throw;
  }
}

const D3D12_RESOURCE_STATES kResourceStateMask_ComputeContex = D3D12_RESOURCE_STATE_UNORDERED_ACCESS | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT | D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_COPY_SOURCE;
const D3D12_RESOURCE_STATES kResourceStateMask_GraphicsContext = static_cast<D3D12_RESOURCE_STATES>(~0u);
const D3D12_RESOURCE_STATES kResourceStateMask_Copy = D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_COPY_SOURCE;
const D3D12_RESOURCE_STATES kResourceStateMask_Read = D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;




const D3D12_RESOURCE_STATES kDx12ResourceStateMask_UAVRead = 
