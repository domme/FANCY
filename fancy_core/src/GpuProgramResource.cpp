#include "GpuProgramResource.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//  
  void GpuProgramResourceInfo::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&u32RegisterIndex, "registerIndex");
    aSerializer->Serialize(&name, "name");
    aSerializer->Serialize(&eAccessType, "accessType");
    aSerializer->Serialize(&eResourceType, "resourceType");
  }
//---------------------------------------------------------------------------//
  void ConstantBufferElement::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&name, "name");
    aSerializer->Serialize(&uOffsetBytes, "OffsetBytes");
    aSerializer->Serialize(&uSizeBytes, "SizeBytes");
    aSerializer->Serialize(&eFormat, "dataFormat");
    aSerializer->Serialize(&uFormatComponentCount, "formatComponentCount");
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

