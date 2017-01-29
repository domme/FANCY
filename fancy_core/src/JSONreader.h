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

    void SerializeDescription(DescriptionBase* aDescription);

  protected:

    struct RootHeader
    {
      uint32 myVersion;
      
      std::vector<SharedPtr<Geometry::Mesh>> myMeshes;
      std::vector<SharedPtr<Rendering::Texture>> myTextures;
      std::vector<SharedPtr<Rendering::GpuProgram>> myGpuPrograms;
      std::vector<SharedPtr<Rendering::GpuProgramPipeline>> myGpuProgramPipelines;
      std::vector<SharedPtr<Rendering::MaterialPass>> myMaterialPasses;
      std::vector<SharedPtr<Rendering::MaterialPassInstance>> myMaterialPassInstances;
      std::vector<SharedPtr<Rendering::Material>> myMaterials;
      std::vector<SharedPtr<Geometry::SubModel>> mySubModels;
      std::vector<SharedPtr<Geometry::Model>> myModels;

      std::vector<SharedPtr<DescriptionBase>> myResourceDependencies;
    };

    bool serializeImpl(DataType aDataType, void* anObject, const char* aName) override;

    void beginName(const char* aName, bool anIsArray) override;
    void endName() override;

    void loadHeader();

    GraphicsWorld& myGraphicsWorld;

    RootHeader myHeader;
    Json::Value myDocumentVal;

    std::stack<Json::ArrayIndex> myArrayIndexStack;
    std::stack<Json::Value*> myTypeStack;
  };
//---------------------------------------------------------------------------//  
} }