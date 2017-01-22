#pragma once

#include "ObjectName.h"

namespace Fancy {
class GraphicsWorld;
struct DescriptionBase;
}

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class ObjectFactory
  {
    public:
      static void* create(const ObjectName& aTypeName, GraphicsWorld* aGraphicsWorld, bool& aWasCreated, uint64 anInstanceHash = 0u);
      static SharedPtr<void> Create(const ObjectName& aTypeName, const DescriptionBase& aDesc, GraphicsWorld* aGraphicsWorld);
  };
//---------------------------------------------------------------------------//
} }
