#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include "d3dx12.h"

using namespace Microsoft::WRL;  // Too ugly?!

void CheckD3Dcall(HRESULT aResult)
{
  if (aResult != S_OK)
    throw;
}
