#include "MeshDesc.h"
#include "Serializer.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  void MeshDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&myVertexAndIndexHash, "myVertexAndIndexHash");
    aSerializer->Serialize(&myVertexLayouts, "myVertexLayouts");
  }
//---------------------------------------------------------------------------//
} }
