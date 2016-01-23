#pragma once

#include "ObjectName.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class ObjectFactory
  {
    public:
      static void* create(const ObjectName& aTypeName, bool& aWasCreated, uint64 anInstanceHash = 0u);
  };
//---------------------------------------------------------------------------//
} }