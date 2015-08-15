#pragma once

#include "ObjectName.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class ObjectFactory
  {
    public:
      static void* create(const ObjectName& aTypeName, bool& aWasCreated, const ObjectName& anInstanceName = ObjectName::blank);
  };
//---------------------------------------------------------------------------//
} }