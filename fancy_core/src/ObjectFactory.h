#pragma once

#include "ObjectName.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class ObjectFactory
  {
    public:
      static void* create(const ObjectName& aTypeName, const ObjectName& anInstanceName = ObjectName::blank);
  };
//---------------------------------------------------------------------------//
} }