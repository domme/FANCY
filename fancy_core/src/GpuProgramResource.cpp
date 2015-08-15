#include "GpuProgramResource.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//  
  void GpuProgramResourceInfo::serialize(IO::Serializer* aSerializer)
  {
    PLATFORM_DEPENDENT_NAME(GpuProgramResource)::serialize(aSerializer);
    
    aSerializer->serialize(&u32RegisterIndex, "registerIndex");
    aSerializer->serialize(&name, "name");
    aSerializer->serialize(&eAccessType, "accessType");
    aSerializer->serialize(&eResourceType, "resourceType");
  }
//---------------------------------------------------------------------------//
  void ConstantBufferElement::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&name, "name");
    aSerializer->serialize(&uOffsetBytes, "OffsetBytes");
    aSerializer->serialize(&uSizeBytes, "SizeBytes");
    aSerializer->serialize(&eFormat, "dataFormat");
    aSerializer->serialize(&uFormatComponentCount, "formatComponentCount");
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

