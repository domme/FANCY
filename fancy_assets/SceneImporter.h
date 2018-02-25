#ifndef INCLUDE_SCENEIMPORTER_H
#define INCLUDE_SCENEIMPORTER_H

#include "FancyCorePrerequisites.h"
#include "FixedArray.h"

enum aiTextureType;

namespace Fancy {
class GraphicsWorld;
}

namespace Fancy { namespace Scene {
class ModelComponent;
class Scene;
  class SceneNode;
} }

struct aiMesh;
struct aiMaterial;
struct aiScene;
struct aiNode;

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class SceneImporter 
  {
    public:
    explicit SceneImporter(GraphicsWorld& aGraphicsWorld);
      ~SceneImporter();

      bool importToSceneGraph(const std::string& _szImportPathRel, Scene::SceneNode* _pParentNode);

      // static void _shaderTest();

      static void initLogger();
      static void destroyLogger();

  private:
    static const uint kMaxNumAssimpMeshesPerNode = 128u;
    typedef FixedArray<aiMesh*, kMaxNumAssimpMeshesPerNode> AiMeshList;
    typedef std::map<const aiMaterial*, SharedPtr<Rendering::Material>> MaterialCacheMap;
    typedef FixedArray<std::pair<AiMeshList, SharedPtr<Geometry::Mesh>>, 256u> MeshCacheList;

    struct WorkingData
    {
      WorkingData() : szCurrScenePathInResources(""), pCurrScene(nullptr),
        u32NumCreatedMeshes(0u), u32NumCreatedModels(0u), u32NumCreatedGeometryDatas(0u), u32NumCreatedSubModels(0u) {}

      std::string szCurrScenePathInResources;
      const aiScene* pCurrScene;
      MaterialCacheMap mapAiMatToMat;
      MeshCacheList localMeshCache;

      uint u32NumCreatedMeshes;
      uint u32NumCreatedModels;
      uint u32NumCreatedGeometryDatas;
      uint u32NumCreatedSubModels;
    };

    GraphicsWorld& myGraphicsWorld;
    WorkingData myWorkingData;

    bool processAiNode(const aiNode* _pAnode, Scene::SceneNode* _pParentNode);
    bool processMeshes(const aiNode* _pAnode, Scene::ModelComponent* _pModelComponent);

    SharedPtr<Geometry::Mesh> constructOrRetrieveMesh(const aiNode* _pAnode, aiMesh** someMeshes, uint aMeshCount);
    SharedPtr<Rendering::Material> createOrRetrieveMaterial(const aiMaterial* _pAmaterial);
    SharedPtr<Rendering::Texture> createOrRetrieveTexture(const aiMaterial* _pAmaterial, uint _aiTextureType, uint _texIndex);

    std::string GetCachePathForMesh();


  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO

#endif  // INCLUDE_SCENEIMPORTER_H