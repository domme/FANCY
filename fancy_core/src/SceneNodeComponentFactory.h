#ifndef INCLUDE_SCENENODECOMPONENTFACTORY_H
#define INCLUDE_SCENENODECOMPONENTFACTORY_H

#include "FancyCorePrerequisites.h"
#include "SceneNodeComponent.h"
#include "ObjectName.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  class SceneNodeComponentFactory
  {
    public:
      typedef std::function<SceneNodeComponentPtr(SceneNode*)> CreateFunction;
      static void registerFactory(const ObjectName& typeName, CreateFunction creator);
      static CreateFunction getFactoryMethod(const ObjectName& typeName);
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene

#endif   // INCLUDE_SCENENODECOMPONENTFACTORY_H