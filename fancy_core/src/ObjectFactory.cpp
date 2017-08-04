#include "ObjectFactory.h"
#include "GpuProgram.h"
#include "MeshDesc.h"
#include "Material.h"
#include "Model.h"
#include "SubModel.h"

#include "GpuProgramPipelineDesc.h"

#include "BinaryCache.h"
#include "SceneNode.h"
#include "SceneNodeComponentFactory.h"
#include "GraphicsWorld.h"
#include "RenderCore.h"

namespace Fancy { namespace IO {

//---------------------------------------------------------------------------//
  template<class T, MemoryCategory eMemoryCategory>
  void* locCreate(uint64, GraphicsWorld* aGraphicsWorld, bool& aWasCreated)
  {
    aWasCreated = true;
    return FANCY_NEW(T, eMemoryCategory);
  }
//---------------------------------------------------------------------------//
  template<>
  void* locCreate<Scene::SceneNode, MemoryCategory::GENERAL> (uint64, GraphicsWorld* aGraphicsWorld, bool& aWasCreated)
  {
    aWasCreated = true;
    return FANCY_NEW(Scene::SceneNode(aGraphicsWorld->GetScene()), eMemoryCategory);
  }
//---------------------------------------------------------------------------//
  typedef void* (*CreateFunc)(uint64, GraphicsWorld*, bool&);
  std::pair<ObjectName, CreateFunc> locResourceCreateFunctions[] =
  {
    { _N(SceneNode), &locCreate<Scene::SceneNode, MemoryCategory::GENERAL> },
  };
//---------------------------------------------------------------------------//
  void* ObjectFactory::create(const ObjectName& aTypeName, GraphicsWorld* aGraphicsWorld, bool& aWasCreated, uint64 aHash)
  {
    Scene::SceneNodeComponentFactory::CreateFunction createFunc = 
      Scene::SceneNodeComponentFactory::GetFactoryFunction(aTypeName);

    if (createFunc != nullptr)
    {
      aWasCreated = true;
      return createFunc();
    }
    
    for (std::pair<ObjectName, CreateFunc>& createFuncEntry : locResourceCreateFunctions)
      if (createFuncEntry.first == aTypeName)
        return createFuncEntry.second(aHash, aGraphicsWorld, aWasCreated);

    ASSERT(false, "Unknown typename");
    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<void> ObjectFactory::Create(const ObjectName& aTypeName, const DescriptionBase& aDesc, GraphicsWorld* aGraphicsWorld)
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
    else if (aTypeName == _N(Mesh))
    {
      const Geometry::MeshDesc& desc = static_cast<const Geometry::MeshDesc&>(aDesc);
      return Rendering::RenderCore::GetMesh(aDesc.GetHash()); 
    }
    else if (aTypeName == _N(SubModel))
    {
      const Geometry::SubModelDesc& desc = static_cast<const Geometry::SubModelDesc&>(aDesc);
      return aGraphicsWorld->CreateSubModel(desc);
    }
    else if (aTypeName == _N(Material))
    {
      const Rendering::MaterialDesc& desc = static_cast<const Rendering::MaterialDesc&>(aDesc);
      return aGraphicsWorld->CreateMaterial(desc);
    }
    if (aTypeName == _N(Model))
    {
      const Geometry::ModelDesc& desc = static_cast<const Geometry::ModelDesc&>(aDesc);
      return aGraphicsWorld->CreateModel(desc);
    }

    ASSERT(false, "Unknown typename");
    return SharedPtr<void>();
  }
//---------------------------------------------------------------------------//
} }
