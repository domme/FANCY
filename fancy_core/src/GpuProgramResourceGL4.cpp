#include "GpuProgramResourceGL4.h"
#include "Serializer.h"

#if defined (RENDERER_OPENGL4)

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//  
void GpuProgramResourceGL4::Serialize(IO::Serializer* aSerializer)
{
  aSerializer->Serialize(&bindingTargetGL, "GLbindingTarget");
  aSerializer->Serialize(&dataFormatGL, "GLdataFormat");
}
//---------------------------------------------------------------------------//  
} } }  // end of namespace Fancy::Rendering::GL4

#endif