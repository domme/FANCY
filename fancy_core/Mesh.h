#pragma once

#include "FancyCoreDefines.h"
#include "DynamicArray.h"
#include "VertexInputLayoutProperties.h"

namespace Fancy 
{
//---------------------------------------------------------------------------//
  struct VertexInputLayout;
  class GpuBuffer;
//---------------------------------------------------------------------------//
  struct MeshPartData
  {
    DynamicArray<uint8> myVertexData;
    DynamicArray<uint8> myIndexData;
    VertexInputLayoutProperties myVertexLayoutProperties;
  };
//---------------------------------------------------------------------------//
  struct MeshPart
  {
    SharedPtr<GpuBuffer> myVertexBuffer;
    SharedPtr<GpuBuffer> myIndexBuffer;
    SharedPtr<VertexInputLayout> myVertexInputLayout;
  };
//---------------------------------------------------------------------------//
  struct MeshDesc
  {
    String myName;
    uint64 myHash = 0ull;
  };
//---------------------------------------------------------------------------//
  struct MeshData
  {
    MeshDesc myDesc;
    DynamicArray<MeshPartData> myParts;
  };
//---------------------------------------------------------------------------//
  struct Mesh
  {
    MeshDesc myDesc;
    DynamicArray<SharedPtr<MeshPart>> myParts;
  };
  //---------------------------------------------------------------------------//
}