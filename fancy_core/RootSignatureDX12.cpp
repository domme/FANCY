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
    for (uint iParam = 0u; iParam < (uint) myRootParameters.size(); ++iParam)
    {
      if (myRootParameters[iParam].myIsDescriptorTable)
      {
        DescriptorTable& table = myRootParameters[iParam].myDescriptorTable;
        table.myIsDirty = true;
        table.myConstantTableStartDescriptor.ptr = UINT64_MAX;
        for (uint iRange = 0u; iRange < (uint) table.myRanges.size(); ++iRange)
        {
          for (uint iDesc = 0u; iDesc < (uint) table.myRanges[iRange].myDescriptors.size(); ++iDesc)
            table.myRanges[iRange].myDescriptors[iDesc] = { 0ull };
        }
      }
      else
      {
        myRootParameters[iParam].myRootDescriptor.myType = ShaderResourceTypeDX12::None;
        myRootParameters[iParam].myRootDescriptor.myGpuVirtualAddress = 0ull;
        myRootParameters[iParam].myRootDescriptor.myIsDirty = true;
      }
    }

    myIsDirty = true;
  }
//---------------------------------------------------------------------------//
  RootSignatureLayoutDX12::RootSignatureLayoutDX12(const D3D12_ROOT_SIGNATURE_DESC1& aRootSigDesc)
  {
    myRootParameters.resize(aRootSigDesc.NumParameters);
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
          dstTable.myRanges.resize(srcTable.NumDescriptorRanges);

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
  uint64 RootSignatureLayoutDX12::GetHash() const
  {
    if (myRootParameters.empty())
      return 0ull;

    MathUtil::Hasher hasher;

    for (const RootParameter& rootParam : myRootParameters)
    {
      hasher.Add(rootParam.myType);
      hasher.Add(rootParam.myVisiblity);
      if (rootParam.myType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
        hasher.Add(rootParam.myDescriptorTable.myRanges.data(), DYN_ARRAY_BYTESIZE(rootParam.myDescriptorTable.myRanges));
      else if (rootParam.myType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
        hasher.Add(rootParam.myRootConstants);
      else
        hasher.Add(rootParam.myRootDescriptor);
    }

    return hasher.GetHashValue();
  }
//---------------------------------------------------------------------------//
  RootSignatureBindingsDX12::RootSignatureBindingsDX12(const RootSignatureLayoutDX12& aLayout)
  {
    myRootParameters.resize((uint)aLayout.myRootParameters.size());

    for (uint iParam = 0u; iParam < (uint) myRootParameters.size(); ++iParam)
    {
      const RootSignatureLayoutDX12::RootParameter& srcParam = aLayout.myRootParameters[iParam];
      RootParameter& dstParam = myRootParameters[iParam];
      ASSERT(srcParam.myType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, "Nu support for root constants implemented yet");

      dstParam.myIsDescriptorTable = srcParam.myType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
      if (dstParam.myIsDescriptorTable)
      {
        myDescriptorTables.push_back(&dstParam.myDescriptorTable);

        dstParam.myDescriptorTable.myRanges.resize((uint)srcParam.myDescriptorTable.myRanges.size());
        dstParam.myDescriptorTable.myHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
        if (!srcParam.myDescriptorTable.myRanges.empty())
        {
          const D3D12_DESCRIPTOR_RANGE1& srcRange = srcParam.myDescriptorTable.myRanges.front();
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
          
          bool hasUnboundedRanges = false;
          for (uint iRange = 0u; iRange < (uint) dstParam.myDescriptorTable.myRanges.size(); ++iRange)
          {
            const D3D12_DESCRIPTOR_RANGE1& srcRange = srcParam.myDescriptorTable.myRanges[iRange];
            DescriptorRange& dstRange = dstParam.myDescriptorTable.myRanges[iRange];

            dstRange.myType = srcRange.RangeType;

            if (srcRange.NumDescriptors == UINT_MAX)  
            {
              dstRange.myIsUnbounded = true;
              hasUnboundedRanges = true;
            }
            else
            {
              ASSERT(!hasUnboundedRanges, "Fixed-size descriptor arrays can't follow on unbounded arrays");

              dstRange.myIsUnbounded = false;
              dstRange.myDescriptors.resize(srcRange.NumDescriptors);

              for (D3D12_CPU_DESCRIPTOR_HANDLE descriptor : dstRange.myDescriptors)
                descriptor = { 0ull };
            }
          }

          dstParam.myDescriptorTable.myBoundedNumDescriptors = 0u;
          dstParam.myDescriptorTable.myHasUnboundedRanges = hasUnboundedRanges;
          if (!hasUnboundedRanges)
          {
            for (const DescriptorRange& range : dstParam.myDescriptorTable.myRanges)
              dstParam.myDescriptorTable.myBoundedNumDescriptors += (uint) range.myDescriptors.size();
          }
        }
      }
    }
  }
//---------------------------------------------------------------------------//
}

#endif  // FANCYFANCY_ENABLE_DX12