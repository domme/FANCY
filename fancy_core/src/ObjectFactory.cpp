#include "ObjectFactory.h"
#include "GpuProgram.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  void* ObjectFactory::create(const ObjectName& aTypeName, const ObjectName& anInstanceName)
  {
    if (aTypeName == _N(GpuProgram))
    {
      Rendering::GpuProgram* object = Rendering::GpuProgram::getByName(anInstanceName);

      if (object)
        return object;

      object = FANCY_NEW(Rendering::GpuProgram, MemoryCategory::MATERIALS);
      object->m_Name = anInstanceName;
      Rendering::GpuProgram::registerWithName(anInstanceName, object);
      return object;
    }

    ASSERT(false, "Unknown typename");
    return nullptr;
  }
//---------------------------------------------------------------------------//
} }