#include "SceneNodeComponentFactory.h"

namespace Fancy { namespace Scene {
  //---------------------------------------------------------------------------//
 namespace internal {
    typedef std::map<ObjectName, SceneNodeComponentFactory::CreateFunction> ComponentCreatorMap;
    ComponentCreatorMap mapComponentCreators;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void SceneNodeComponentFactory::
    registerFactory( const ObjectName& typeName, CreateFunction creator )
  {
    internal::ComponentCreatorMap::iterator it = internal::mapComponentCreators.find(typeName);
    ASSERT(it == internal::mapComponentCreators.end());

    internal::mapComponentCreators[typeName] = creator;
  }
//---------------------------------------------------------------------------//
  SceneNodeComponentFactory::CreateFunction 
    SceneNodeComponentFactory::getFactoryMethod( const ObjectName& typeName )
  {
    internal::ComponentCreatorMap::iterator it = internal::mapComponentCreators.find(typeName);

    if (it == internal::mapComponentCreators.end())
    {
      return nullptr;
    }

    return it->second;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene