#include "ObjectFactory.h"
#include "GpuProgram.h"
#include "MaterialPass.h"
#include "MaterialPassInstance.h"
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"
#include "Model.h"
#include "SubModel.h"

#include "BinaryCache.h"
#include "SceneNode.h"
#include "SceneNodeComponentFactory.h"
#include "Renderer.h"

namespace Fancy { namespace IO {
  
/*

//---------------------------------------------------------------------------//
  template<class T, MemoryCategory eMemoryCategory>
  void* locCreateManaged(uint64 aHash, bool& aWasCreated)
  {
    T* object = T::Find(aHash);

    if (object)
    {
      aWasCreated = false;
      return object;
    }
     
    aWasCreated = true;
    object = FANCY_NEW(T, eMemoryCategory);
    
    // TODO: This is bad design... we should set the whole description here. 
    // We need to do this during a refactoring of the serialization-system...
    T::Register(object, aHash);  
    
    return object;
  }
//---------------------------------------------------------------------------//
  template<>
  void* locCreateManaged<Geometry::Mesh, MemoryCategory::GEOMETRY>
    (uint64 aHash, bool& aWasCreated)
  {
    Geometry::Mesh* object = Geometry::Mesh::Find(aHash);

    if (object)
    {
      aWasCreated = false;
      return object;
    }

    aWasCreated = true;
    BinaryCache::read(&object, aHash, 0u);

    return object;
  }
//---------------------------------------------------------------------------//
  template<class T, MemoryCategory eMemoryCategory>
  void* locCreate(uint64, bool& aWasCreated)
  {
    aWasCreated = true;
    return FANCY_NEW(T, eMemoryCategory);
  }
//---------------------------------------------------------------------------//
  typedef void* (*CreateFunc)(uint64, bool&);
  std::pair<ObjectName, CreateFunc> locResourceCreateFunctions[] =
  {
    // Managed:
    { _N(MaterialPass), &locCreateManaged<Rendering::MaterialPass, MemoryCategory::MATERIALS> },
    { _N(Material), &locCreateManaged<Rendering::Material, MemoryCategory::MATERIALS> },
    { _N(Mesh), &locCreateManaged<Geometry::Mesh, MemoryCategory::GEOMETRY> },
    { _N(SubModel), &locCreateManaged<Geometry::SubModel, MemoryCategory::GEOMETRY> },
    { _N(Model), &locCreateManaged<Geometry::Model, MemoryCategory::GEOMETRY> },

    // Non-managed
    { _N(MaterialPassInstance), &locCreate<Rendering::MaterialPassInstance, MemoryCategory::MATERIALS> },
    { _N(SceneNode), &locCreate<Scene::SceneNode, MemoryCategory::GENERAL> },
  };
//---------------------------------------------------------------------------//
  void* ObjectFactory::create(const ObjectName& aTypeName, bool& aWasCreated, uint64 aHash)
  {
    Scene::SceneNodeComponentFactory::CreateFunction createFunc = 
      Scene::SceneNodeComponentFactory::getFactoryMethod(aTypeName);

    if (createFunc != nullptr)
    {
      aWasCreated = true;
      return createFunc(nullptr);
    }
    
    for (std::pair<ObjectName, CreateFunc>& createFuncEntry : locResourceCreateFunctions)
      if (createFuncEntry.first == aTypeName)
        return createFuncEntry.second(aHash, aWasCreated);

    ASSERT(false, "Unknown typename");
    return nullptr;
  }
*/

  void* ObjectFactory::create(const ObjectName& aTypeName, bool& aWasCreated, uint64 aHash)
  {
    // TODO: We have to redesign the whole functionality how "managed" objects are stored. Instead of the hash we need to serialize real descriptions from which to create objects properly!
    return nullptr;
  }




//---------------------------------------------------------------------------//
} }
