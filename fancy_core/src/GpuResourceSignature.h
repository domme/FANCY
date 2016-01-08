#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class GpuResSignatureEntryType
  {
    Constants,          // (DX12:) Root Constants
    GpuVirtualAddress,  // (DX12:) Root Descriptor
    Table               // (DX12:) Descriptor Table
  };
//---------------------------------------------------------------------------//
  struct GpuResSignatureEntry
  {
    GpuResSignatureEntryType myType;
    uint32 myNumElements;
    uint32 myRegisterIndex;
  };
//---------------------------------------------------------------------------//
  /// Platform-independent abstraction of the "RootSignature" concept in DX12. Not used in GL4 currently
  class GpuResourceSignature
  {
    // TODO: Implement with data-layout resembling the native DX12-RootSignature Desc. 
    // Use a hash to quickly lookup the corresponding native RootSignature object
    // We might also need a conversion between D3D12's native RootSignature-descs and this GpuResourceSignature
    // In order to simply load the signature from a compiled HLSL-header that contains the rootSignature-string
    // This would avoid having to maintain code-side signature definitions.

    std::vector<GpuResSignatureEntry> myEntries;
    uint myHash;
  };
//---------------------------------------------------------------------------//
} }


