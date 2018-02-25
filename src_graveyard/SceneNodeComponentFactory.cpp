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
    RegisterFactory( const ObjectName& typeName, CreateFunction creator )
  {
    internal::mapComponentCreators[typeName] = creator;
  }
//---------------------------------------------------------------------------//
  SceneNodeComponentFactory::CreateFunction 
    SceneNodeComponentFactory::GetFactoryFunction( const ObjectName& typeName )
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