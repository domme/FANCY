#pragma once

namespace Fancy
{
//---------------------------------------------------------------------------//
  class ShaderResourceInterfaceDX12
  {
  public:
    //---------------------------------------------------------------------------//
    enum class RootParameterType
    {
      Constants = 0,
      Descriptor,
      DescriptorSet
    };
    //---------------------------------------------------------------------------//
    enum class SriResourceType
    {
      ConstantBuffer = 0,
      BufferOrTexture,
      BufferOrTextureRW,
      Sampler,
      Float,
    };
    //---------------------------------------------------------------------------//
    struct SriDescriptorSetElement
    {
      uint myNumElements;
      SriResourceType myResourceType;
      uint myBindingSlot;
    };
    //---------------------------------------------------------------------------//
    struct RootDescriptorTable
    {
      enum { MAX_DESCRIPTOR_SET_ELEMENTS = 64 };
      uint myNumElements;
      SriDescriptorSetElement myRangeElements[MAX_DESCRIPTOR_SET_ELEMENTS];
    };
    //---------------------------------------------------------------------------//
    struct RootDescriptor
    {
      SriResourceType myResourceType;
      uint myBindingSlot;
    };
    //---------------------------------------------------------------------------//
    struct RootConstant
    {
      D3D12_ROOT_CONSTANTS myConstant;
      String myName;
      uint64 myNameHash;
    };
    //---------------------------------------------------------------------------//
    struct RootParameter
    {
      RootParameterType myType;

      union
      {
        RootDescriptorTable myDescriptorSet;
        RootDescriptor myDescriptor;
        RootConstant myConstant;
      };
    };
    //---------------------------------------------------------------------------//

    DynamicArray<RootParameter> myElements;
  };
//---------------------------------------------------------------------------//
}


