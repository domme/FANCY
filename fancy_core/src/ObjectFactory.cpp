#include "ObjectFactory.h"
#include "GpuProgram.h"
#include "MaterialPass.h"
#include "MaterialPassInstance.h"
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"
#include "Model.h"
#include "SubModel.h"

#include "GpuProgramPipelineDesc.h"

#include "BinaryCache.h"
#include "SceneNode.h"
#include "SceneNodeComponentFactory.h"
#include "Renderer.h"

namespace Fancy { namespace IO {

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
//---------------------------------------------------------------------------//
  SharedPtr<void> ObjectFactory::Create(const ObjectName& aTypeName, const DescriptionBase& aDesc)
  {
    if (aTypeName == _N(Texture))
    {
      const Rendering::TextureDesc& desc = static_cast<const Rendering::TextureDesc&>(aDesc);
      return Rendering::RenderCore::CreateTexture(desc);
    }
    else if (aTypeName == _N(GpuProgram))
    {
      const Rendering::GpuProgramDesc& desc = static_cast<const Rendering::GpuProgramDesc&>(aDesc);
      return Rendering::RenderCore::CreateGpuProgram(desc);
    }
    else if (aTypeName == _N(GpuProgramPipeline))
    {
      const Rendering::GpuProgramPipelineDesc& desc = static_cast<const Rendering::GpuProgramPipelineDesc&>(aDesc);
      return Rendering::RenderCore::CreateGpuProgramPipeline(desc);
    }
    
    ASSERT(false, "Unknown typename");
    return nullptr;
  }
//---------------------------------------------------------------------------//
} }
