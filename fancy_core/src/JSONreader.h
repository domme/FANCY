#pragma once

#include "Serializer.h"
#include "Json/json.h"

namespace Fancy {
  class GraphicsWorld;
}

namespace Fancy { namespace Rendering {
  struct MaterialDesc;
  struct TextureDesc;
  struct GpuProgramDesc;
  struct GpuProgramPipelineDesc;
} }

namespace Fancy { namespace Geometry {
  struct MeshDesc;
  struct ModelDesc;
  struct SubModelDesc;
} }

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class JSONreader : public Serializer
  {
  public:
    JSONreader(const String& anArchivePath, GraphicsWorld& aGraphicsWorld);
    virtual ~JSONreader() override;

    const uint myVersion = 0;

    void SerializeDescription(DescriptionBase* aDescription);

  protected:

    struct RootHeader
    {
      uint myVersion;

      Json::Value* myResources;
      std::vector<SharedPtr<DescriptionBase>> myCreatedDescs;
      std::vector<DescriptionBase*> myLoadedDecscs;
    };

    DescriptionBase* GetResourceDesc(uint64 aHash);
    Json::Value* GetResourceVal(const ObjectName& aTypeName, uint64 aHash) const;

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