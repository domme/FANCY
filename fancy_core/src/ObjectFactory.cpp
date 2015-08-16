#include "ObjectFactory.h"
#include "GpuProgram.h"
#include "MaterialPass.h"
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"
#include "Model.h"
#include "SubModel.h"

#include "BinaryCache.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  template<class T, MemoryCategory eMemoryCategory>
  void* locCreateManaged(const ObjectName& anInstanceName, bool& aWasCreated)
  {
    T* object = T::getByName(anInstanceName);

    if (object)
    {
      aWasCreated = false;
      return object;
    }
     
    aWasCreated = true;
    object = FANCY_NEW(T, eMemoryCategory);
    T::registerWithName(anInstanceName, object);
    return object;
  }
//---------------------------------------------------------------------------//
  template<>
  void* locCreateManaged<Rendering::Texture, MemoryCategory::TEXTURES>
    (const ObjectName& anInstanceName, bool& aWasCreated)
  {
    Rendering::Texture* object = Rendering::Texture::getByName(anInstanceName);

    if (object)
    {
      aWasCreated = false;
      return object;
    }

    aWasCreated = true;
    BinaryCache::read(&object, anInstanceName, 0u);

    return object;
  }
//---------------------------------------------------------------------------//
  template<>
  void* locCreateManaged<Geometry::Mesh, MemoryCategory::GEOMETRY>
    (const ObjectName& anInstanceName, bool& aWasCreated)
  {
    Geometry::Mesh* object = Geometry::Mesh::getByName(anInstanceName);

    if (object)
    {
      aWasCreated = false;
      return object;
    }

    aWasCreated = true;
    BinaryCache::read(&object, anInstanceName, 0u);

    return object;
  }
//---------------------------------------------------------------------------//
  template<class T, MemoryCategory eMemoryCategory>
  void* locCreate(const ObjectName& anInstanceName, bool& aWasCreated)
  {
    aWasCreated = true;
    return FANCY_NEW(T, eMemoryCategory);
  }
//---------------------------------------------------------------------------//
  typedef void* (*CreateFunc)(const ObjectName&, bool&);
  std::pair<ObjectName, CreateFunc> locCreateFuncRegistry[] =
  {
    // Managed:
    { _N(GpuProgram), &locCreateManaged<Rendering::GpuProgram, MemoryCategory::MATERIALS> },
    { _N(MaterialPass), &locCreateManaged<Rendering::MaterialPass, MemoryCategory::MATERIALS> },
    { _N(Material), &locCreateManaged<Rendering::Material, MemoryCategory::MATERIALS> },
    { _N(Texture), &locCreateManaged<Rendering::Texture, MemoryCategory::MATERIALS> },
    { _N(Mesh), &locCreateManaged<Geometry::Mesh, MemoryCategory::GEOMETRY> },
    { _N(SubModel), &locCreateManaged<Geometry::SubModel, MemoryCategory::GEOMETRY> },
    { _N(Model), &locCreateManaged<Geometry::Model, MemoryCategory::GEOMETRY> },

    // Non-managed
    { _N(MaterialPassInstance), &locCreate<Rendering::MaterialPassInstance, MemoryCategory::MATERIALS> }


  };
//---------------------------------------------------------------------------//
  void* ObjectFactory::create(const ObjectName& aTypeName, bool& aWasCreated, const ObjectName& anInstanceName)
  {
    for (std::pair<ObjectName, CreateFunc>& createFuncEntry : locCreateFuncRegistry)
      if (createFuncEntry.first == aTypeName)
        return createFuncEntry.second(anInstanceName, aWasCreated);

    ASSERT(false, "Unknown typename");
    return nullptr;
  }
//---------------------------------------------------------------------------//
} }