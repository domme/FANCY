#pragma once

#include "Serializer.h"
#include "Json/json.h"

namespace Fancy {
  class GraphicsWorld;
}

namespace Fancy { namespace Rendering {
  class MaterialPassInstance;
  class MaterialPass;
  class GpuProgram;
  class Texture;
} }

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class JSONreader : public Serializer
  {
  public:
    JSONreader(const String& anArchivePath, GraphicsWorld& aGraphicsWorld);
    virtual ~JSONreader() override;

    const uint32 myVersion = 0;

    void SerializeDescription(DescriptionBase* aDescription) override;

  protected:

    struct RootHeader
    {
      uint32 myVersion;
      
      std::vector<Rendering::MaterialPass*> myMaterialPasses;
      std::vector<Rendering::MaterialPassInstance*> myMaterialPassInstances;
      std::vector<Rendering::Material*> myMaterials;
      std::vector<Geometry::SubModel*> mySubModels;
      std::vector<Geometry::Model*> myModels;
      
      std::vector<SharedPtr<Geometry::Mesh>> myMeshes;
      std::vector<SharedPtr<Rendering::GpuProgramPipeline>> myGpuProgramPipelines;
      std::vector<SharedPtr<Rendering::GpuProgram>> myGpuPrograms;
      std::vector<SharedPtr<Rendering::Texture>> myTextures;

      std::vector<SharedPtr<DescriptionBase>> myResourceDependencies;
    };

    virtual bool serializeImpl(DataType aDataType, void* anObject, const char* aName) override;

    virtual void beginName(const char* aName, bool anIsArray) override;
    virtual void endName() override;

    void loadHeader();

    GraphicsWorld& myGraphicsWorld;

    RootHeader myHeader;
    Json::Value myDocumentVal;

    std::stack<Json::ArrayIndex> myArrayIndexStack;
    std::stack<Json::Value*> myTypeStack;
  };
//---------------------------------------------------------------------------//  
} }