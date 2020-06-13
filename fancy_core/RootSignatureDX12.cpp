#include "fancy_core_precompile.h"
#include "RootSignatureDX12.h"
#include "FancyCoreDefines.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
//---------------------------------------------------------------------------//
  void RootSignatureBindingsDX12::Clear()
  {
    // Clear bound resources but don't change parameters or descriptor table info
    for (uint iParam = 0u; iParam < myRootParameters.Size(); ++iParam)
    {
      if (myRootParameters[iParam].myIsDescriptorTable)
      {
        DescriptorTable& table = myRootParameters[iParam].myDescriptorTable;
        for (uint iRange = 0u; iRange < table.myRanges.Size(); ++iRange)
        {
          for (uint iDesc = 0u; iDesc < table.myRanges[iRange].myDescriptors.Size(); ++iDesc)
            table.myRanges[iRange].myDescriptors[iDesc] = { 0ull };
        }
      }
      else
      {
        myRootParameters[iParam].myRootDescriptor.myType = ShaderResourceTypeDX12::None;
        myRootParameters[iParam].myRootDescriptor.myGpuVirtualAddress = 0ull;
      }
    }
  }
//---------------------------------------------------------------------------//
  RootSignatureLayoutDX12::RootSignatureLayoutDX12(const D3D12_ROOT_SIGNATURE_DESC1& aRootSigDesc)
  {
    myRootParameters.GrowToSize(aRootSigDesc.NumParameters);
    for (uint iRootParam = 0u; iRootParam < aRootSigDesc.NumParameters; ++iRootParam)
    {
      const D3D12_ROOT_PARAMETER1& srcParam = aRootSigDesc.pParameters[iRootParam];
      RootParameter& dstParam = myRootParameters[iRootParam];

      dstParam.myType = srcParam.ParameterType;
      dstParam.myVisiblity = srcParam.ShaderVisibility;
      switch (srcParam.ParameterType)
      {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: 
        {
          const D3D12_ROOT_DESCRIPTOR_TABLE1& srcTable = srcParam.DescriptorTable;
          DescriptorTable& dstTable = dstParam.myDescriptorTable;
          dstTable.myRanges.GrowToSize(srcTable.NumDescriptorRanges);

          for (uint iRange = 0u; iRange < srcTable.NumDescriptorRanges; ++iRange)
            dstTable.myRanges[iRange] = srcTable.pDescriptorRanges[iRange];
        }
        break;
        case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
          dstParam.myRootConstants = srcParam.Constants;
        break;
        case D3D12_ROOT_PARAMETER_TYPE_CBV: 
        case D3D12_ROOT_PARAMETER_TYPE_SRV: 
        case D3D12_ROOT_PARAMETER_TYPE_UAV: 
          dstParam.myRootDescriptor = srcParam.Descriptor;
          break;
        default: ASSERT(false);
      }
    }
  }
//---------------------------------------------------------------------------//
  RootSignatureBindingsDX12::RootSignatureBindingsDX12(const RootSignatureLayoutDX12& aLayout)
  {
    myRootParameters.GrowToSize(aLayout.myRootParameters.Size());

    for (uint iParam = 0u; iParam < myRootParameters.Size(); ++iParam)
    {
      const RootSignatureLayoutDX12::RootParameter& srcParam = aLayout.myRootParameters[iParam];
      RootParameter& dstParam = myRootParameters[iParam];
      ASSERT(srcParam.myType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, "Nu support for root constants implemented yet");

      dstParam.myIsDescriptorTable = srcParam.myType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
      if (dstParam.myIsDescriptorTable)
      {
        dstParam.myDescriptorTable.myRanges.GrowToSize(srcParam.myDescriptorTable.myRanges.Size());
        dstParam.myDescriptorTable.myHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
        if (!srcParam.myDescriptorTable.myRanges.IsEmpty())
        {
          const D3D12_DESCRIPTOR_RANGE1& srcRange = srcParam.myDescriptorTable.myRanges.GetFirst();
          switch (srcRange.RangeType)
          {
          case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
          case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
          case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
            dstParam.myDescriptorTable.myHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            break;
          case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
            dstParam.myDescriptorTable.myHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            break;
          default: ASSERT(false);
          }

          dstParam.myDescriptorTable.myNumDescriptors = 0u;
          for (uint iRange = 0u; iRange < dstParam.myDescriptorTable.myRanges.Size(); ++iRange)
          {
            const D3D12_DESCRIPTOR_RANGE1& srcRange = srcParam.myDescriptorTable.myRanges[iRange];
            DescriptorRange& dstRange = dstParam.myDescriptorTable.myRanges[iRange];

            dstRange.myType = srcRange.RangeType;

            dstRange.myDescriptors.GrowToSize(glm::min(srcRange.NumDescriptors, dstRange.myDescriptors.Capacity()));

            for (uint iDesc = 0u; iDesc < dstRange.myDescriptors.Size(); ++iDesc)
              dstRange.myDescriptors[iDesc] = { 0ull };

            dstParam.myDescriptorTable.myNumDescriptors += dstRange.myDescriptors.Size();
          }
        }
      }
    }
  }
//---------------------------------------------------------------------------//
}

#endif  // FANCYFANCY_ENABLE_DX12