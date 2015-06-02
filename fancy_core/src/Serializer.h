#ifndef INCLUDE_SERIALIZER_H
#define INCLUDE_SERIALIZER_H

#include "FancyCorePrerequisites.h"
#include <fstream>
#include "FixedArray.h"

#include "Json/json.h"

#define _VAL(X) X, #X

namespace Fancy { namespace Rendering {
  class MaterialPassInstance;
}}
 
namespace Fancy { namespace Scene {
  class Scene;
  class SceneNode;
  class SceneNodeComponent;

  using SceneNodeComponentPtr = std::shared_ptr<SceneNodeComponent>;
  using SceneNodePtr = std::shared_ptr < SceneNode > ;
}}

namespace Fancy { namespace Geometry {
  class Model;
  class SubModel;

  using SubModelList = FixedArray< SubModel*, kMaxNumSubModelsPerModel > ;
} }

namespace Fancy { namespace IO {
    class SerializerBinary;
} }

namespace Fancy { namespace IO {
  //---------------------------------------------------------------------------//
    enum class ESerializationMode
    {
      STORE,
      LOAD
    };
  //---------------------------------------------------------------------------//
    struct TypeInfo
    {
      String myTypeName;
      uint   myInstanceHash;
    };
  //---------------------------------------------------------------------------//
    struct TocEntry
    {
      TypeInfo myTypeInfo;
      uint32 myArchivePosition;
    };
  //---------------------------------------------------------------------------//
  //---------------------------------------------------------------------------//
    class Serializer
    {
      public:
        Serializer(ESerializationMode aMode);
        virtual ~Serializer();
      //---------------------------------------------------------------------------//
      //---------------------------------------------------------------------------//
        template<class T> void serialize(T& anObject, const char* aName = nullptr)
        {
          serialize(&anObject, aName);
        }
      //---------------------------------------------------------------------------//
        template<class T> void serialize(T* anObject, const char* aName = nullptr)
        {
          if (myMode == ESerializationMode::STORE)
          {
            store(aName, anObject);
          }
          else
          {
            load(aName, anObject);
          }
        }
      //---------------------------------------------------------------------------//
        virtual void* beginType(const String& aTypeName, uint anInstanceHash) = 0;
        virtual void endType() = 0;

        virtual void beginArray(const char* aName, void* aCarray) = 0;
        virtual void endArray() = 0;
        
    protected:
      virtual void store(const char* aName, uint* aValue) = 0;
      virtual void store(const char* aName, float* aValue) = 0;
      virtual void store(const char* aName, String* aValue) = 0;
      virtual void store(const char* aName, std::vector<Scene::SceneNodeComponentPtr>* someValues) = 0;
      virtual void store(const char* aName, std::vector<Scene::SceneNodePtr>* someValues) = 0;
      virtual void store(const char* aName, Geometry::Model** aValue) = 0;
      virtual void store(const char* aName, Geometry::SubModel** aValue) = 0;
      virtual void store(const char* aName, Geometry::SubModelList* someValues) = 0;
      virtual void store(const char* aName, Rendering::Material** aValue) = 0;
      virtual void store(const char* aName, glm::mat3* aValue) = 0;
      virtual void store(const char* aName, glm::mat4* aValue) = 0;
      virtual void store(const char* aName, glm::vec3* aValue) = 0;
      virtual void store(const char* aName, glm::vec4* aValue) = 0;

      ESerializationMode myMode;
      std::fstream myArchive;
    };
  //---------------------------------------------------------------------------//
  //---------------------------------------------------------------------------//
    class SerializerJSON : public Serializer
    {
    public:
      SerializerJSON(ESerializationMode aMode, const String& anArchivePath);
      virtual ~SerializerJSON() override { }

      virtual void* beginType(const String& aTypeName, uint anInstanceHash) override;
      virtual void endType() override;

      virtual void beginArray(const char* aName, void* aCarray) override;
      virtual void endArray() override;

    protected:
      virtual void store(const char* aName, uint* aValue) override;
      virtual void store(const char* aName, float* aValue) override;
      virtual void store(const char* aName, String* aValue) override;
      virtual void store(const char* aName, std::vector<Scene::SceneNodeComponentPtr>* someValues) override;
      virtual void store(const char* aName, std::vector<Scene::SceneNodePtr>* someValues) override;
      virtual void store(const char* aName, Geometry::Model** aValue) override;
      virtual void store(const char* aName, Geometry::SubModel** aValue) override;
      virtual void store(const char* aName, Geometry::SubModelList* someValues) override;
      virtual void store(const char* aName, Rendering::Material** aValue) override;
      virtual void store(const char* aName, glm::mat3* aValue) override;
      virtual void store(const char* aName, glm::mat4* aValue) override;
      virtual void store(const char* aName, glm::vec3* aValue) override;
      virtual void store(const char* aName, glm::vec4* aValue) override;
      void _store(const char* aName, const Json::Value& aValue);

      std::stack<Json::Value> myTypeStack;
      Json::StyledStreamWriter myJsonWriter;
    };

  //---------------------------------------------------------------------------//
    // TODO: Move to somewhere else and let clases register their own create-functions
    //class Factory 
    //{
    //public:
    //  static Scene::SceneNodeComponentPtr construct(const ObjectName& aTypeName)
    //  {
    //    // yeeeey -.-
    //    if (aTypeName == _N(ModelComponent))
    //      return std::make_shared<ModelComponent>()

    //  }
    //};
    
// #define SERIALIZER_BINARY_SUPPORT
#if defined SERIALIZER_BINARY_SUPPORT
  //---------------------------------------------------------------------------//
    class SerializerBinary // : public Serializer
    {
    public:
      SerializerBinary(ESerializationMode _aMode, std::fstream* anArchive, const String& anArchivePath)
      {
        myMode = _aMode; 
        mySceneGraphStr = anArchive;
        myToc.clear();
        myCurrentlyProcessedNode = nullptr;
      }
    //---------------------------------------------------------------------------//
      ~SerializerBinary()
      {
        if (mySceneGraphStr != nullptr && mySceneGraphStr->good())
        {
          mySceneGraphStr->close();
        }
      }
    //---------------------------------------------------------------------------//
      static void writeToArchive(Rendering::Texture* aTexture, void* someData, uint32 aDataCount, std::fstream* anArchive);
    //---------------------------------------------------------------------------//
      static void writeToArchive(Geometry::GeometryData* aGeometry, std::fstream* anArchive);
    //---------------------------------------------------------------------------//
      static void loadFromArchive(Rendering::Texture** aTexture, std::fstream* anArchive);
    //---------------------------------------------------------------------------//
      static void loadFromArchive(Geometry::GeometryData** aTexture, std::fstream* anArchive);
    //---------------------------------------------------------------------------//
      ESerializationMode getMode() { return myMode; }
    //---------------------------------------------------------------------------//
      void beginType(uint aTypeHash, uint anInstanceHash)
      {
        if (myMode == ESerializationMode::STORE)
        {
          (*mySceneGraphStr) << aTypeHash;
          (*mySceneGraphStr) << anInstanceHash;
          
          TocEntry entry;
          entry.myTypeInfo.myTypeHash = aTypeHash;
          entry.myTypeInfo.myInstanceHash = anInstanceHash;
          entry.myArchivePosition = myMode == ESerializationMode::LOAD ? mySceneGraphStr->tellg() : mySceneGraphStr->tellp();
          myToc.push_back(entry);
        }
        else  // LOAD
        {
          uint typeHash;
          uint instanceHash;
          (*mySceneGraphStr) >> typeHash;  // Keep the get-pointer in sync
          (*mySceneGraphStr) >> instanceHash;
        }
      }
    //---------------------------------------------------------------------------//
      template<class T> void operator&(T& anObject)
      {
        serialize(&anObject);
      }
    //---------------------------------------------------------------------------//
      template<class T> void operator&(T* anObject)
      {
        serialize(anObject);
      }
    //---------------------------------------------------------------------------//      
      template<class T> void serialize(T* anObject)
      {
        ASSERT(mySceneGraphStr != nullptr && mySceneGraphStr->good());
        ASSERT(anObject);

        if (myMode == ESerializationMode::STORE)
        {
          store(anObject, std::integral_constant<bool, std::is_fundamental<T>::value || std::is_enum<T>::value>());
        }
        else
        {
          load(anObject, std::integral_constant<bool, std::is_fundamental<T>::value || std::is_enum<T>::value>());
        }
      }
    //---------------------------------------------------------------------------//
    protected:

#pragma region Store
      template<class T> void store(T* anObject, std::false_type isFundamental)
      {
        anObject->serialize(this);
      }
    //---------------------------------------------------------------------------//
      template<class T> void store(T* anObject, std::true_type isFundamental)
      {
        (*mySceneGraphStr) << (*anObject);
      }
    //---------------------------------------------------------------------------//
      template<class T> void storeManaged(T** anObject)
      {
        TypeInfo info = { 0u };
        if ((*anObject) != nullptr)
        {
          info.myTypeHash = (*anObject)->getTypeName();
          info.myInstanceHash = (*anObject)->getName();
        }

        if (!isInstanceStored(info))
        {
          // Instance hasn't been stored yet. Store it here!
          (*anObject)->serialize(*this);
        }
        else
        {
          // Instance was already stored earlier... just store the typeinfo here.
          mySceneGraphStr->write((char*)&info, sizeof(TypeInfo));
        }
      }
    //---------------------------------------------------------------------------//
      template<> void store(std::vector<Scene::SceneNodeComponentPtr>* someComponents, std::false_type isFundamental)
      {
        uint32 numComponents = someComponents->size();
        (*mySceneGraphStr) << numComponents;

        for (uint32 i = 0u; i < numComponents; ++i)
        {
          (*this) & (*someComponents)[i].get();
        }
      }
    //---------------------------------------------------------------------------//
      template<> void store(std::vector<Scene::SceneNodePtr>* someNodes, std::false_type isFundamental)
      {
        uint32 numNodes = someNodes->size();
        (*mySceneGraphStr) << numNodes;

        for (uint32 i = 0u; i < numNodes; ++i)
        {
          (*this) & (*someNodes)[i].get();
        }
      }
    //---------------------------------------------------------------------------//
      template<> void store(Geometry::Model** aModel, std::false_type isFundamental)
      {
        storeManaged<Geometry::Model>(aModel);
      }
    //---------------------------------------------------------------------------//
      template<> void store(Geometry::SubModel** aModel, std::false_type isFundamental)
      {
        storeManaged<Geometry::SubModel>(aModel);
      }
    //---------------------------------------------------------------------------//
      template<> void store(Geometry::SubModelList* someSubmodels, std::false_type isFundamental)
      {
        uint32 numSubmodels = someSubmodels->size();
        (*mySceneGraphStr) << numSubmodels;

        for (uint32 i = 0u; i < numSubmodels; ++i)
        {
          (*this) & (*someSubmodels)[i];
        }
      }
    //---------------------------------------------------------------------------//
      template<> void store(Rendering::Material** aMaterial, std::false_type isFundamental)
      {
        storeManaged<Rendering::Material>(aMaterial);
      }
    //---------------------------------------------------------------------------//
#pragma endregion Store

#pragma region Load
      template<class T> void load(T* anObject, std::false_type isFundamental)
      {
        anObject->serialize(*this);
      }
    //---------------------------------------------------------------------------//
      template<class T> void load(T* anObject, std::true_type isFundamental)
      {
        (*mySceneGraphStr) >> (*anObject);
      }
    //---------------------------------------------------------------------------//
      template<class T> void loadManaged(T** anObject)
      {
        TypeInfo typeInfo = pushTypePeek();
        (*anObject) = T::getByName(typeInfo.myInstanceHash);
        if ((*anObject) == nullptr)
        {
          popTypePeek();
          (*anObject) = FANCY_NEW(T, MemoryCategory::GEOMETRY);
          (*anObject)->serialize(*this);
          T::registerWithName((*anObject));
        }
      }
    //---------------------------------------------------------------------------//
      template<> void load(Scene::SceneNodePtr* anObject, std::false_type isFundamental)
      {
        (*anObject) = std::make_shared<Scene::SceneNode>();
        myCurrentlyProcessedNode = anObject->get();
        (*anObject)->serialize(*this);
      }
    //---------------------------------------------------------------------------//
      void loadNextSceneNodeComponent()
      {
        ASSERT(myCurrentlyProcessedNode);
        TypeInfo typeInfo = pushTypePeek();
        popTypePeek();
        Scene::SceneNodeComponent* component = myCurrentlyProcessedNode->addOrRetrieveComponent(typeInfo.myTypeHash);
        component->serialize(*this);
      }
    //---------------------------------------------------------------------------//
      template<> void load(std::vector<Scene::SceneNodeComponentPtr>* someComponents, std::false_type isFundamental)
      {
        uint32 numComponents;
        (*mySceneGraphStr) >> numComponents;

        for (uint32 i = 0u; i < numComponents; ++i)
        {
          loadNextSceneNodeComponent();
        }
      }
    //---------------------------------------------------------------------------//
      template<> void load(std::vector<Scene::SceneNodePtr>* someNodes, std::false_type isFundamental)
      {
        uint32 numNodes;
        (*mySceneGraphStr) >> numNodes;

        (*someNodes).resize(numNodes);
        for (uint32 i = 0u; i < numNodes; ++i)
        {
          (*this) & (*someNodes)[i];
        }
      }
    //---------------------------------------------------------------------------//
      template<> void load(Geometry::Model** aModel, std::false_type isFundamental)
      {
        loadManaged<Geometry::Model>(aModel);
      }
    //---------------------------------------------------------------------------//
      template<> void load(Geometry::SubModel** aSubModel, std::false_type isFundamental)
      {
        loadManaged<Geometry::SubModel>(aSubModel);
      }
    //---------------------------------------------------------------------------//
      template<> void load(Rendering::Material** aMaterial, std::false_type isFundamental)
      {
        loadManaged<Rendering::Material>(aMaterial);
      }
    //---------------------------------------------------------------------------//
      template<> void load(Rendering::MaterialPassInstance** anMpi, std::false_type isFundamental)
      {
        // TODO: This is EXTREMELY ugly: load the MaterialPassInstance completely, including textures and the matpass,
        // then add the mpi to the matpass if neccessary
        Rendering::MaterialPassInstance mpiTemplate;
        mpiTemplate.serialize(*this);

        Rendering::MaterialPass* matPass = mpiTemplate.getMaterialPass();
       
        // Does this materialPass already contain an MPI with the same hash?
        (*anMpi) = matPass->getMaterialPassInstance(mpiTemplate.computeHash());
        if ((*anMpi) == nullptr)
        {
          (*anMpi) = matPass->createMaterialPassInstance(mpiTemplate.getName(), mpiTemplate);
        }
      }
    //---------------------------------------------------------------------------//
      template<> void load(Rendering::MaterialPass** aMatPass, std::false_type isFundamental)
      {
        loadManaged<Rendering::MaterialPass>(aMatPass);
      }
    //---------------------------------------------------------------------------//
      template<> void load(Geometry::SubModelList* someSubmodels, std::false_type isFundamental)
      {
        uint32 numSubmodels;
        (*mySceneGraphStr) >> numSubmodels;

        someSubmodels->resize(numSubmodels);
        for (uint32 i = 0u; i < numSubmodels; ++i)
        {
          (*this) & (*someSubmodels)[i];
        }
      }
    //---------------------------------------------------------------------------//
      bool isInstanceStored(const TypeInfo& aTypeInfo, uint32* anArchivePosition = nullptr)
      {
        if (aTypeInfo.myTypeHash == 0u && aTypeInfo.myInstanceHash == 0u)
        {
          return true;  // The empty instance is always "stored"
        }

        for (uint32 i = 0u; i < myToc.size(); ++i)
        {
          if (myToc[i].myTypeInfo.myTypeHash == aTypeInfo.myTypeHash && 
              myToc[i].myTypeInfo.myTypeHash == aTypeInfo.myInstanceHash)
          {
            if (anArchivePosition != nullptr)
            {
              (*anArchivePosition) = myToc[i].myArchivePosition;
            }
            return true;
          }
        }

        return false;
      }
    //---------------------------------------------------------------------------//
      TypeInfo pushTypePeek()
      {
        TypeInfo info = { 0u };
        if (myMode != ESerializationMode::LOAD)
        {
          return info;
        }
        mySceneGraphStr->read((char*)&info, sizeof(TypeInfo));
        return info;
      }
    //---------------------------------------------------------------------------//
      void popTypePeek()
      {
        uint32 currStreamPos = mySceneGraphStr->tellg();
        mySceneGraphStr->seekg(currStreamPos - sizeof(TypeInfo));
      }
    //---------------------------------------------------------------------------//
#pragma endregion Load

    private:
      ESerializationMode myMode;
      std::fstream* mySceneGraphStr;
      std::vector<TocEntry> myToc;
      Scene::SceneNode* myCurrentlyProcessedNode;
    };
  //---------------------------------------------------------------------------//

#endif  // SERIALIZER_BINARY_SUPPORT

  //---------------------------------------------------------------------------//
  } } // end of namespace Fancy::IO 
  
#endif  // INCLUDE_FILEREADER_H