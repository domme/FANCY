#pragma once

#if FANCY_ENABLE_DX12

#pragma warning( disable : 26812 )  // Disable "Prefer enum class over enum"

#include "directx/d3d12.h"
#include "directx/d3dx12_barriers.h"
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include "directx/dxgiformat.h"
#include "Rendering/RendererPrerequisites.h"

namespace {
  void ASSERT_HRESULT( HRESULT aResult ) {
    assert( aResult == S_OK );
  }
}  // namespace

#endif