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
