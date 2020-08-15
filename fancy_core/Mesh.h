#pragma once

#include "FancyCoreDefines.h"
#include "VertexInputLayoutProperties.h"

namespace Fancy 
{
//---------------------------------------------------------------------------//
  struct VertexInputLayout;
  class GpuBuffer;
//---------------------------------------------------------------------------//
  struct MeshPartData
  {
    eastl::vector<uint8> myVertexData;
    eastl::vector<uint8> myIndexData;
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
    eastl::vector<MeshPartData> myParts;
  };
//---------------------------------------------------------------------------//
  struct Mesh
  {
    MeshDesc myDesc;
    eastl::vector<SharedPtr<MeshPart>> myParts;
  };
  //---------------------------------------------------------------------------//
}