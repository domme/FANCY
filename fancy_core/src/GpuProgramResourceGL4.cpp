#include "GpuProgramResourceGL4.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//  
void GpuProgramResourceGL4::serialize(IO::Serializer* aSerializer)
{
  aSerializer->serialize(&bindingTargetGL, "GLbindingTarget");
  aSerializer->serialize(&dataFormatGL, "GLdataFormat");
}
//---------------------------------------------------------------------------//  
} } }  // end of namespace Fancy::Rendering::GL4