#pragma once

#include "Common/FancyCoreDefines.h"
#include "Rendering/VertexInputLayoutProperties.h"
#include "Rendering/ResourceHandle.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  struct VertexInputLayout;
  class GpuBuffer;
  //---------------------------------------------------------------------------//
  struct MeshPartData {
    eastl::vector< uint8 >      myVertexData;
    eastl::vector< uint8 >      myIndexData;
    VertexInputLayoutProperties myVertexLayoutProperties;
  };
  //---------------------------------------------------------------------------//
  struct MeshPart {
    GpuBufferHandle         myVertexBuffer;
    GpuBufferHandle         myIndexBuffer;
    VertexInputLayoutHandle myVertexInputLayout;
  };
  //---------------------------------------------------------------------------//
  struct MeshDesc {
    eastl::string myName;
    uint64        myHash = 0ull;

    static uint64 Hash( const MeshDesc & aDesc ) {
      return aDesc.myHash;
    }
  };
  //---------------------------------------------------------------------------//
  struct MeshData {
    MeshDesc                      myDesc;
    eastl::vector< MeshPartData > myParts;
  };
  //---------------------------------------------------------------------------//
  struct Mesh {
    MeshDesc                               myDesc;
    eastl::vector< SharedPtr< MeshPart > > myParts;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy