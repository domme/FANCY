#pragma once

#include "Serializer.h"
#include "Json/json.h"

namespace Fancy { namespace Rendering {
  class MaterialPassInstance;
  class MaterialPass;
  class GpuProgram;
} }

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class JSONreader : public Serializer
  {
  public:
    JSONreader(const String& anArchivePath);
    virtual ~JSONreader() override;

    const uint32 myVersion = 0;

  protected:

    struct RootHeader
    {
      uint32 myVersion;
      std::vector<Rendering::GpuProgram*> myGpuPrograms;
      std::vector<Rendering::MaterialPass*> myMaterialPasses;
      std::vector<Rendering::Material*> myMaterials;
      std::vector<Geometry::Mesh*> myMeshes;
      std::vector<Geometry::SubModel*> mySubModels;
      std::vector<Geometry::Model*> myModels;
    };

    virtual bool serializeImpl(DataType aDataType, void* anObject, const char* aName) override;

    virtual void beginName(const char* aName, bool anIsArray) override;
    virtual void endName() override;

    void loadHeader();

    RootHeader myHeader;
    Json::Value myDocumentVal;

    std::stack<Json::ArrayIndex> myArrayIndexStack;
    std::stack<Json::Value*> myTypeStack;
  };
//---------------------------------------------------------------------------//  
} }