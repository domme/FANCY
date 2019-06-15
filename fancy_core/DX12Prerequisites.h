#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include "d3dx12.h"
#include <dxgiformat.h>
#include "RendererPrerequisites.h"

namespace {
  void CheckD3Dcall(HRESULT aResult)
  {
    if (aResult != S_OK)
      throw;
  }
}

const D3D12_RESOURCE_STATES kResourceStateMask_ComputeContext = D3D12_RESOURCE_STATE_UNORDERED_ACCESS | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT | D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_COPY_SOURCE;
const D3D12_RESOURCE_STATES kResourceStateMask_GraphicsContext = static_cast<D3D12_RESOURCE_STATES>(~0u);
