#include "SceneImporter.h"

#include <assimp/config.h>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>
#include <xxHash/xxhash.h>

#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Mesh.h"
#include "Material.h"
#include "PathService.h"
#include "ModelComponent.h"
#include "Model.h"
#include "SubModel.h"
#include "GeometryData.h"
#include "GeometryVertexLayout.h"
#include "StringUtil.h"
#include "Texture.h"
#include "JSONwriter.h"
#include "JSONreader.h"
#include "VertexInputLayout.h"
#include "Log.h"
#include "RenderCore.h"
#include "GraphicsWorld.h"

#define FANCY_IMPORTER_USE_VALIDATION

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  using namespace Fancy::Geometry;
  using namespace Fancy::Rendering;
//---------------------------------------------------------------------------//
  namespace Internal
  {
  //---------------------------------------------------------------------------//
    class FancyLog : public Assimp::LogStream
    {
      void write(const char* message) override;
    };
    //---------------------------------------------------------------------------//
    void FancyLog::write(const char* message)
    {
      C_LOG_INFO("SceneImporter: %s", message);
    }
//---------------------------------------------------------------------------//
    glm::mat4 matFromAiMat( const aiMatrix4x4& mat )
    {
      return glm::mat4(	mat.a1, mat.a2, mat.a3, mat.a4,
        mat.b1, mat.b2, mat.b3, mat.b4,
        mat.c1, mat.c2, mat.c3, mat.c4,
        mat.d1, mat.d2, mat.d3, mat.d4 );
    }
  //---------------------------------------------------------------------------//
  //---------------------------------------------------------------------------//
    static FancyLog* m_pLogger = nullptr;
  //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  void SceneImporter::initLogger()
  {
    const unsigned int severity = 
        Assimp::Logger::Debugging
        |Assimp::Logger::Info
        |Assimp::Logger::Err
        |Assimp::Logger::Warn;

    Internal::m_pLogger = new Internal::FancyLog();

    Assimp::DefaultLogger::get()->attachStream(Internal::m_pLogger, severity);
  }
//---------------------------------------------------------------------------//
  void SceneImporter::destroyLogger()
  {
    Assimp::DefaultLogger::get()->detatchStream(Internal::m_pLogger);
  }
//---------------------------------------------------------------------------//
  SceneImporter::SceneImporter(GraphicsWorld& aGraphicsWorld)
    : myGraphicsWorld(aGraphicsWorld)
  {
  }
//---------------------------------------------------------------------------//
  SceneImporter::~SceneImporter()
  {

  }
//---------------------------------------------------------------------------//
  bool SceneImporter::importToSceneGraph( const std::string& _szImportPathRel, Scene::SceneNode* _pParentNode)
  {
    bool foundFile = false;
    String resourcePathAbs = Resources::FindPath(_szImportPathRel, &foundFile);

    if (!foundFile)
      return false;

    // TODO: Look for cached binary data and don't re-import if possible

    Assimp::Importer aImporter;

    const aiScene* aScene = aImporter.ReadFile(resourcePathAbs,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_FindInstances);

    if (!aScene)
      return false;

    myWorkingData = WorkingData();
    myWorkingData.szCurrScenePathInResources = _szImportPathRel;
    myWorkingData.pCurrScene = aScene;

    const aiNode* pArootNode = aScene->mRootNode;

    if (!pArootNode)
      return false;

    Scene::Scene* scene = _pParentNode->GetScene();
    Scene::SceneNode* importedSceneNode = new Scene::SceneNode(scene);
    
    if (!processAiNode(pArootNode, importedSceneNode))
      return false;

    // Serialization-tests....
    // Serialization needs a re-work after adjusting Resource-Handling apis
    /*
    {
      JSONwriter serializer(resourcePathAbs);
      serializer.Serialize(&importedSceneNode, "rootNode");
    }

    {
      SAFE_DELETE(importedSceneNode);
      JSONreader serializer(resourcePathAbs, myGraphicsWorld);
      serializer.Serialize(&importedSceneNode, "rootNode");
    }
    */

    Scene::SceneNode::AddChildNode(SharedPtr<Scene::SceneNode>(importedSceneNode), _pParentNode);
    
    return true;
  }
//---------------------------------------------------------------------------//
  bool SceneImporter::processAiNode(const aiNode* _pAnode, Scene::SceneNode* _pParentNode)
  {
    bool success = true;

    Scene::SceneNode* pNode = _pParentNode->createChildNode();
    if (_pAnode->mName.length > 0u)
    {
      pNode->setName(ObjectName(_pAnode->mName.C_Str()));
    }

    glm::mat4 transformMat = Internal::matFromAiMat(_pAnode->mTransformation);
    glm::mat3 rotationScale(transformMat);
    glm::quat rotation = glm::toQuat(rotationScale);
    glm::vec3 scale(glm::length(rotationScale[0]), glm::length(rotationScale[1]), glm::length(rotationScale[2]));
    glm::vec3 pos(transformMat[3]);

    pNode->getTransform().setRotationLocal(rotation);
    pNode->getTransform().setScaleLocal(scale);
    pNode->getTransform().setPositionLocal(pos);

    if (_pAnode->mNumMeshes > 0u)
    {
      Scene::ModelComponent* pModelComponent = static_cast<Scene::ModelComponent*>(pNode->addOrRetrieveComponent(_N(ModelComponent)));
      success &= processMeshes(_pAnode, pModelComponent);
    }
    
    for (uint i = 0u; success && i < _pAnode->mNumChildren; ++i)
    {
      success &= processAiNode(_pAnode->mChildren[i], pNode);
    }

    return success;
  }
//---------------------------------------------------------------------------//
  bool SceneImporter::processMeshes(const aiNode* _pAnode, Scene::ModelComponent* _pModelComponent)
  {
    // Sort all meshes by their material. Each entry will become a submodel of the model
    
    typedef FixedArray<aiMesh*, kMaxNumAssimpMeshesPerNode> AssimpMeshList;

    typedef std::map<uint, AssimpMeshList> MaterialMeshMap;
     MaterialMeshMap mapMaterialIndexMesh;

    const uint uNumMeshes = _pAnode->mNumMeshes;
    for (uint i = 0u; i < uNumMeshes; ++i)
    {
      const uint uMeshIndex = _pAnode->mMeshes[i];
      aiMesh* pAmesh = 
        myWorkingData.pCurrScene->mMeshes[uMeshIndex];

      const uint uMaterialIndex = pAmesh->mMaterialIndex;
      AssimpMeshList& vMeshesWithMaterial = mapMaterialIndexMesh[uMaterialIndex];
      
      if (!vMeshesWithMaterial.contains(pAmesh))
      {
        vMeshesWithMaterial.push_back(pAmesh);
      }
    }

    // Construct or retrieve Fancy Meshes and Submodels
    // Each mesh-list with the same material becomes a submodel
    std::vector<SharedPtr<SubModel>> vSubModels;
    for (MaterialMeshMap::iterator it = mapMaterialIndexMesh.begin(); it != mapMaterialIndexMesh.end(); ++it)
    {
      const uint uMaterialIndex = it->first;
      AssimpMeshList& vAssimpMeshes = it->second;
      
      SharedPtr<Geometry::Mesh> pMesh = 
        constructOrRetrieveMesh(_pAnode, &vAssimpMeshes[0], vAssimpMeshes.size());

      // Create or retrieve the material
      aiMaterial* pAmaterial = 
        myWorkingData.pCurrScene->mMaterials[uMaterialIndex];
      SharedPtr<Rendering::Material> pMaterial = createOrRetrieveMaterial(pAmaterial);

      // Do we already have a Submodel with this mesh and material?
      Geometry::SubModelDesc submodelDesc;
      submodelDesc.myMaterial = pMaterial->GetDescription();
      submodelDesc.myMesh = pMesh->GetDescription();

      SharedPtr<Geometry::SubModel> pSubModel = myGraphicsWorld.CreateSubModel(submodelDesc);
      if (vSubModels.end() == std::find(vSubModels.begin(), vSubModels.end(), pSubModel))
      {
        vSubModels.push_back(pSubModel);
      }
    }  // end iteration of materialMeshList-map

    // At this point, we constructed a bunch of submodels. Now construct them to 
    // a Model (or retrieve an equivalent one...)
    Geometry::ModelDesc modelDesc;
    modelDesc.mySubmodels.resize(vSubModels.size());
    for (uint i = 0u; i < vSubModels.size(); ++i)
      modelDesc.mySubmodels[i] = vSubModels[i]->GetDescription();

    SharedPtr<Geometry::Model> pModel = myGraphicsWorld.CreateModel(modelDesc);
    _pModelComponent->setModel(pModel);

    return true;
  }
//---------------------------------------------------------------------------//
  uint64 locComputeHashFromVertexData(aiMesh** someMeshes, uint aMeshCount)
  {
    XXH64_state_t* xxHashState = XXH64_createState();
    XXH64_reset(xxHashState, 0u);

    for (uint iMesh = 0u; iMesh < aMeshCount; ++iMesh)
    {
      aiMesh* mesh = someMeshes[iMesh];

      XXH64_update(xxHashState, mesh->mVertices, mesh->mNumVertices * sizeof(aiVector3D));

      if (mesh->HasNormals())
        XXH64_update(xxHashState, mesh->mNormals, mesh->mNumVertices * sizeof(aiVector3D));

      if (mesh->HasTangentsAndBitangents())
      {
        XXH64_update(xxHashState, mesh->mTangents, mesh->mNumVertices * sizeof(aiVector3D));
        XXH64_update(xxHashState, mesh->mBitangents, mesh->mNumVertices * sizeof(aiVector3D));
      }

      for (uint iChannel = 0u; iChannel < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++iChannel)
      {
        if (mesh->HasTextureCoords(iChannel))
          XXH64_update(xxHashState, mesh->mTextureCoords[iChannel], mesh->mNumVertices * sizeof(aiVector3D));
      }

      for (uint iChannel = 0u; iChannel < AI_MAX_NUMBER_OF_COLOR_SETS; ++iChannel)
      {
        if (mesh->HasVertexColors(iChannel))
          XXH64_update(xxHashState, mesh->mColors[iChannel], mesh->mNumVertices * sizeof(aiColor4D));
      }

      if (mesh->HasFaces())
      {
        for (uint iFace = 0u; iFace < mesh->mNumFaces; ++iFace)
        {
          aiFace& face = mesh->mFaces[iFace];
          XXH64_update(xxHashState, face.mIndices, face.mNumIndices * sizeof(unsigned int));
        }
      }
    }

    uint64 hash = XXH64_digest(xxHashState);

    XXH64_freeState(xxHashState);

    return hash;
  }
//---------------------------------------------------------------------------//
  const ShaderVertexInputElement* locGetShaderExpectedInput(VertexSemantics aSemantic)
  {
    const ShaderVertexInputLayout& modelLayout = ShaderVertexInputLayout::ourDefaultModelLayout;
    const ShaderVertexInputElementList& vertexElements = modelLayout.getVertexElementList();

    for (uint i = 0u; i < vertexElements.size(); ++i)
      if (vertexElements[i].mySemantics == aSemantic)
        return &vertexElements[i];

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Geometry::Mesh> SceneImporter::constructOrRetrieveMesh(const aiNode* _pANode, aiMesh** someMeshes, uint aMeshCount)
  {
    // TODO: Refactor this caching-mechanism:
    // We don't save any processing time if we read in the cached mesh, construct its hash and THEN check if we already have this mesh loaded in the engine
    // Instead, do the following:
    // 1) Modify the cache-system so we can "peek ahead" and only retrieve its vertexIndexHash
    // 2) Check if we already have a mesh with this hash, if so --> return this mesh
    // 3) If there isn't any matching Mesh in the engine yet --> load it from cache if the cache-timestamp is newer than the imported scene-file
    // 4) If the scene-file is newer: load the mesh from assimp and update cache file

    ObjectName meshName = GetCachePathForMesh();
    // if (BinaryCache::read(&outputMesh, meshName, 0u))
    // return outputMesh;

    // Did we construct a similar mesh before from the same ai-Meshes?
    auto findMeshInCache = [&]() -> SharedPtr<Geometry::Mesh> {
      for (uint i = 0u; i < myWorkingData.localMeshCache.size(); ++i)
      {
        const std::pair<AiMeshList, SharedPtr<Geometry::Mesh>>& entry = myWorkingData.localMeshCache[i];

        bool isValid = true;
        for (uint iAiMesh = 0u; isValid && iAiMesh < aMeshCount; ++iAiMesh)
          isValid &= entry.first.contains(someMeshes[iAiMesh]);

        if (isValid)
          return entry.second;
      }
      return nullptr;
    };
    
    SharedPtr<Geometry::Mesh> mesh;
    mesh = findMeshInCache();
    if (mesh != nullptr)
      return mesh;

    uint64 vertexIndexHash = locComputeHashFromVertexData(someMeshes, aMeshCount);
    
    mesh = Rendering::RenderCore::GetMesh(vertexIndexHash);
    if (mesh != nullptr)
      return mesh;
    
    // We don't have the mesh in any cache and have to create it.

    MeshDesc meshDesc;
    meshDesc.myVertexAndIndexHash = vertexIndexHash;
    
    std::vector<void*> vertexDatas;
    std::vector<uint> numVertices;
    std::vector<void*> indexDatas;
    std::vector<uint> numIndices;
    for (uint iAiMesh = 0; iAiMesh < aMeshCount; ++iAiMesh)
    {
      const aiMesh* aiMesh = someMeshes[iAiMesh];

      struct ImportVertexStream
      {
        void* mySourceData;
        uint mySourceDataStride;
        VertexSemantics mySourceSemantic;
        uint mySourceSemanticIndex;
      };
      FixedArray<ImportVertexStream, Rendering::kMaxNumGeometryVertexAttributes> importStreams;

      ASSERT(aiMesh->HasPositions());
      {
        ImportVertexStream stream;
        stream.mySourceDataStride = sizeof(aiMesh->mVertices[0]);
        stream.mySourceData = aiMesh->mVertices;
        stream.mySourceSemantic = VertexSemantics::POSITION;
        stream.mySourceSemanticIndex = 0u;
        importStreams.push_back(stream);
      }

      if (aiMesh->HasNormals())
      {
        ImportVertexStream stream;
        stream.mySourceDataStride = sizeof(aiMesh->mNormals[0]);
        stream.mySourceData = aiMesh->mNormals;
        stream.mySourceSemantic = VertexSemantics::NORMAL;
        stream.mySourceSemanticIndex = 0u;
        importStreams.push_back(stream);
      }

      if (aiMesh->HasTangentsAndBitangents())
      {
        {
          ImportVertexStream stream;
          stream.mySourceDataStride = sizeof(aiMesh->mTangents[0]);
          stream.mySourceData = aiMesh->mTangents;
          stream.mySourceSemantic = VertexSemantics::TANGENT;
          stream.mySourceSemanticIndex = 0u;
          importStreams.push_back(stream);
        }
        {
          ImportVertexStream stream;
          stream.mySourceDataStride = sizeof(aiMesh->mBitangents[0]);
          stream.mySourceData = aiMesh->mBitangents;
          stream.mySourceSemantic = VertexSemantics::BITANGENT;
          stream.mySourceSemanticIndex = 0u;
          importStreams.push_back(stream);
        }
      }

      for (uint iUVchannel = 0u; iUVchannel < aiMesh->GetNumUVChannels(); ++iUVchannel)
      {
        if (aiMesh->HasTextureCoords(iUVchannel))
        {
          ImportVertexStream stream;
          stream.mySourceDataStride = sizeof(aiMesh->mTextureCoords[iUVchannel][0]);
          stream.mySourceData = aiMesh->mTextureCoords[iUVchannel];
          stream.mySourceSemantic = VertexSemantics::TEXCOORD;
          stream.mySourceSemanticIndex = iUVchannel;
          importStreams.push_back(stream);
        }
      }

      for (uint iColorChannel = 0u; iColorChannel < aiMesh->GetNumColorChannels(); ++iColorChannel)
      {
        if (aiMesh->HasVertexColors(iColorChannel))
        {
          ImportVertexStream stream;
          stream.mySourceDataStride = sizeof(aiMesh->mColors[iColorChannel][0]);
          stream.mySourceData = aiMesh->mColors[iColorChannel];
          stream.mySourceSemantic = VertexSemantics::COLOR;
          stream.mySourceSemanticIndex = iColorChannel;
          importStreams.push_back(stream);
        }
      }

      const ShaderVertexInputLayout* expectedLayout = &ShaderVertexInputLayout::ourDefaultModelLayout;
      const ShaderVertexInputElementList& expectedInputList = expectedLayout->getVertexElementList();

      // Check if we need additional patching-streams when the model-shaders expect more data than this model has

      FixedArray<ImportVertexStream, Rendering::kMaxNumGeometryVertexAttributes> actualImportStreams;
      actualImportStreams.resize(expectedInputList.size());

      FixedArray<void*, Rendering::kMaxNumGeometryVertexAttributes> patchingDatas;

      for (uint i = 0u; i < expectedInputList.size(); ++i)
      {
        const ShaderVertexInputElement& expectedElem = expectedInputList[i];

        bool foundInImportStreams = false;
        for (uint k = 0u; k < importStreams.size(); ++k)
        {
          ImportVertexStream& stream = importStreams[k];

          if (expectedElem.mySemantics != stream.mySourceSemantic)
            continue;

          foundInImportStreams = true;
          
          // Stride-mismatch? Then we have to patch! 
          // Either we have more data we need or not enough. Below accounts for both cases
          if (stream.mySourceDataStride != expectedElem.mySizeBytes) 
          {
            void* patchedData = malloc(expectedElem.mySizeBytes * aiMesh->mNumVertices);
            patchingDatas.push_back(patchedData);

            for (uint iVertex = 0u; iVertex < aiMesh->mNumVertices; ++iVertex)
            {
              uint8* dest = ((uint8*)patchedData) + iVertex * expectedElem.mySizeBytes;
              uint8* src = ((uint8*)stream.mySourceData) + iVertex * stream.mySourceDataStride;

              memcpy(dest, src, glm::min(expectedElem.mySizeBytes, stream.mySourceDataStride));

              if (expectedElem.mySizeBytes > stream.mySourceDataStride)
                memset(dest + stream.mySourceDataStride, 0, (expectedElem.mySizeBytes - stream.mySourceDataStride));
            }

            stream.mySourceDataStride = expectedElem.mySizeBytes;
            stream.mySourceData = patchedData;
          }

          actualImportStreams[i] = stream;
        }

        // Is this semantic missing entirely? 
        if (!foundInImportStreams)
        {
          void* patchedData = malloc(expectedElem.mySizeBytes * aiMesh->mNumVertices);
          patchingDatas.push_back(patchedData);
          memset(patchedData, 0, expectedElem.mySizeBytes * aiMesh->mNumVertices);

          actualImportStreams[i].mySourceDataStride = expectedElem.mySizeBytes;
          actualImportStreams[i].mySourceData = patchedData;
          actualImportStreams[i].mySourceSemantic = expectedElem.mySemantics;
          actualImportStreams[i].mySourceSemanticIndex = expectedElem.mySemanticIndex;
        }
      }
            
      // Construct the vertex layout description.
      // After doing the patching-work above, this can be set up to exactly match the input layout expected by the shaders
      Rendering::GeometryVertexLayout vertexLayout;

      uint offset = 0u;
      for (uint i = 0u; i < expectedInputList.size(); ++i)
      {
        const ShaderVertexInputElement& expectedInput = expectedInputList[i];

        GeometryVertexElement vertexElem;
        vertexElem.u32OffsetBytes = offset;
        vertexElem.eFormat = expectedInput.myFormat;
        vertexElem.mySemanticIndex = expectedInput.mySemanticIndex;
        vertexElem.eSemantics = expectedInput.mySemantics;
        vertexElem.name = expectedInput.myName;
        vertexElem.u32SizeBytes = expectedInput.mySizeBytes;

        offset += vertexElem.u32SizeBytes;
        vertexLayout.addVertexElement(vertexElem);
      }

      const uint uSizeVertexBufferBytes = vertexLayout.getStrideBytes() * aiMesh->mNumVertices;
      void* pData = FANCY_ALLOCATE(uSizeVertexBufferBytes, MemoryCategory::GEOMETRY);

      if (!pData)
      {
        LOG_ERROR("Failed to allocate vertex buffer");
        return nullptr;
      }

      // Construct an interleaved vertex array
      for (uint iVertex = 0u; iVertex < aiMesh->mNumVertices; ++iVertex)
      {
        for (uint iVertexElement = 0u; iVertexElement < vertexLayout.getNumVertexElements(); ++iVertexElement)
        {
          const Rendering::GeometryVertexElement& vertexElem = vertexLayout.getVertexElement(iVertexElement);
          uint destInterleavedOffset = iVertex * vertexLayout.getStrideBytes() + vertexElem.u32OffsetBytes;
          uint srcOffset = iVertex * actualImportStreams[iVertexElement].mySourceDataStride;

          uint8* dest = ((uint8*)pData) + destInterleavedOffset;
          uint8* src = ((uint8*)actualImportStreams[iVertexElement].mySourceData) + srcOffset;

          memcpy(dest, src, vertexElem.u32SizeBytes);
        }
      }

      vertexDatas.push_back(pData);
      numVertices.push_back(aiMesh->mNumVertices);
      meshDesc.myVertexLayouts.push_back(vertexLayout);

      for (uint i = 0u; i < patchingDatas.size(); ++i)
        free(patchingDatas[i]);
      patchingDatas.clear();
      
      /// Construct the index buffer
#if defined (FANCY_IMPORTER_USE_VALIDATION)
      // Ensure that we have only triangles
      for (uint i = 0u; i < aiMesh->mNumFaces; ++i)
      {
        ASSERT(aiMesh->mFaces[i].mNumIndices == 3u, "Unsupported face type");
      }
#endif  // FANCY_IMPORTER_USE_VALIDATION

      uint* indices = FANCY_NEW(uint[aiMesh->mNumFaces * 3u], MemoryCategory::GEOMETRY);

      for (uint i = 0u; i < aiMesh->mNumFaces; ++i)
      {
        const aiFace& aFace = aiMesh->mFaces[i];

        ASSERT(sizeof(indices[0]) == sizeof(aFace.mIndices[0]));
        memcpy(&indices[i * 3u], aFace.mIndices, sizeof(uint) * 3u);
      }

      // indexBufParams.uNumElements = aiMesh->mNumFaces * 3u;
      indexDatas.push_back(indices);
      numIndices.push_back(aiMesh->mNumFaces * 3u);
    }

    mesh = Rendering::RenderCore::CreateMesh(meshDesc, vertexDatas, indexDatas, numVertices, numIndices);
    ASSERT(mesh != nullptr);

    AiMeshList aiMeshList;
    aiMeshList.resize(aMeshCount);
    memcpy(&aiMeshList[0], &someMeshes[0], sizeof(aiMesh*) * aMeshCount);
    myWorkingData.localMeshCache.push_back(std::pair<AiMeshList, SharedPtr<Geometry::Mesh>>(aiMeshList, mesh));

    for (uint i = 0u; i < vertexDatas.size(); ++i)
    {
      FANCY_FREE(vertexDatas[i], MemoryCategory::GEOMETRY);
    }
    vertexDatas.clear();

    for (uint i = 0u; i < indexDatas.size(); ++i)
    {
      FANCY_DELETE_ARR(indexDatas[i], MemoryCategory::GEOMETRY);
    }
    indexDatas.clear();
    
    return mesh;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Rendering::Material> SceneImporter::createOrRetrieveMaterial(const aiMaterial* _pAmaterial)
  {
    // Did we already import this material?
    MaterialCacheMap::iterator cacheIt = myWorkingData.mapAiMatToMat.find(_pAmaterial);
    if (cacheIt != myWorkingData.mapAiMatToMat.end())
    {
      return cacheIt->second;
    }
        
    // Retrieve the material properties most relevant for us
    aiString szAname;
    bool hasName = _pAmaterial->Get(AI_MATKEY_NAME, szAname) == AI_SUCCESS;

    aiColor3D color_diffuse;
    bool hasColor = _pAmaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color_diffuse) == AI_SUCCESS;

    aiColor3D color_specular;
    bool hasSpecularColor = _pAmaterial->Get(AI_MATKEY_COLOR_SPECULAR, color_specular) == AI_SUCCESS;

    aiColor3D color_ambient;
    bool hasAmbientColor = _pAmaterial->Get(AI_MATKEY_COLOR_AMBIENT, color_ambient) == AI_SUCCESS;

    aiColor3D color_emissive;
    bool hasEmissiveColor = _pAmaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color_emissive) == AI_SUCCESS;

    aiColor3D color_transparent;
    bool hasTransparentColor = _pAmaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, color_transparent) == AI_SUCCESS;

    aiBlendMode blend_func;
    bool hasBlendFunc = _pAmaterial->Get(AI_MATKEY_BLEND_FUNC, blend_func) == AI_SUCCESS;

    float opacity;
    bool hasOpacity = _pAmaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS;

    float specularPower;
    bool hasSpecularPower = _pAmaterial->Get(AI_MATKEY_SHININESS, specularPower) == AI_SUCCESS;

    float specular;
    bool hasSpecular = _pAmaterial->Get(AI_MATKEY_SHININESS_STRENGTH, specular) == AI_SUCCESS;

    SharedPtr<Texture> pDiffuseTex = createOrRetrieveTexture(_pAmaterial, aiTextureType_DIFFUSE, 0u);
    SharedPtr<Texture> pNormalTex = createOrRetrieveTexture(_pAmaterial, aiTextureType_NORMALS, 0u);
    SharedPtr<Texture> pSpecularTex = createOrRetrieveTexture(_pAmaterial, aiTextureType_SPECULAR, 0u);
    SharedPtr<Texture> pSpecPowerTex = createOrRetrieveTexture(_pAmaterial, aiTextureType_SHININESS, 0u);
    SharedPtr<Texture> pOpacityTex = createOrRetrieveTexture(_pAmaterial, aiTextureType_OPACITY, 0u);

    bool hasDiffuseTex = pDiffuseTex != nullptr;
    bool hasNormalTex = pDiffuseTex != nullptr;
    bool hasSpecTex = pSpecularTex != nullptr;
    bool hasSpecPowerTex = pSpecPowerTex != nullptr;
    bool hasOpacityTex = pOpacityTex != nullptr;

    // Fancy only supports textures where opacity is combined with diffuse and specPower is combined with specular
    if (hasSpecTex && hasSpecPowerTex && pSpecPowerTex != pSpecularTex)
    {
      LOG_WARNING("Fancy doesn't support storing specular power in a separate texture. Consider putting it in spec.a");
    }

    if (hasDiffuseTex && hasOpacityTex && pOpacityTex != pDiffuseTex)
    {
      LOG_WARNING("Fancy doesn't support storing opacity in a separate texture. Consider putting it in diff.a");
    }
    
    // Find/Create a matching Material
    //---------------------------------------------------------------------------//
    MaterialDesc matDesc;

    if (pDiffuseTex != nullptr)
    {
      MaterialTextureDesc matTex((uint)EMaterialTextureSemantic::BASE_COLOR, pDiffuseTex->GetDescription());
      matDesc.myTextures.push_back(matTex);
    }
    if (pNormalTex != nullptr)
    {
      MaterialTextureDesc matTex((uint)EMaterialTextureSemantic::NORMAL, pNormalTex->GetDescription());
      matDesc.myTextures.push_back(matTex);
    }
    if (pSpecularTex != nullptr)
    {
      MaterialTextureDesc matTex((uint)EMaterialTextureSemantic::MATERIAL, pSpecularTex->GetDescription());
      matDesc.myTextures.push_back(matTex);
    }

    if (hasColor)
    {
      MaterialParameterDesc matParam((uint)EMaterialParameterSemantic::DIFFUSE_REFLECTIVITY, 
        (color_diffuse.r + color_diffuse.g + color_diffuse.b) * (1.0f / 3.0f));
      matDesc.myParameters.push_back(matParam);
    }
    if (hasSpecular)
    {
      MaterialParameterDesc matParam((uint)EMaterialParameterSemantic::SPECULAR_REFLECTIVITY,specular);
      matDesc.myParameters.push_back(matParam);
    }
    if (hasOpacity)
    {
      MaterialParameterDesc matParam((uint)EMaterialParameterSemantic::OPACITY, opacity);
      matDesc.myParameters.push_back(matParam);
    }
    if (hasSpecularPower)
    {
      MaterialParameterDesc matParam((uint)EMaterialParameterSemantic::SPECULAR_POWER, specularPower);
      matDesc.myParameters.push_back(matParam);
    }
    
    SharedPtr<Material> pMaterial = myGraphicsWorld.CreateMaterial(matDesc);
    //---------------------------------------------------------------------------//

    return pMaterial;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Rendering::Texture> SceneImporter::createOrRetrieveTexture(
    const aiMaterial* _pAmaterial, uint _aiTextureType, uint _texIndex)
  {
    uint numTextures = _pAmaterial->GetTextureCount(static_cast<aiTextureType>(_aiTextureType));
    if (numTextures == 0u)
    {
      return nullptr;
    }

    ASSERT(numTextures > _texIndex);

    aiString szATexPath;
    _pAmaterial->Get(AI_MATKEY_TEXTURE(_aiTextureType, _texIndex), szATexPath);

    String texPathAbs = String(szATexPath.C_Str());
    Path::UnifySlashes(texPathAbs);
    
    if (!Path::IsPathAbsolute(texPathAbs))
    {
      String relativePath = Path::GetContainingFolder(myWorkingData.szCurrScenePathInResources) + "/" + texPathAbs;
      Path::RemoveFolderUpMarkers(relativePath);
      texPathAbs = Resources::FindPath(relativePath);
    }

    Path::RemoveFolderUpMarkers(texPathAbs);
    
    String texPathInResources = Resources::FindName(texPathAbs);
    return RenderCore::CreateTexture(texPathInResources);
  }
//---------------------------------------------------------------------------//
  std::string SceneImporter::GetCachePathForMesh()
  {
    return "Mesh_" + myWorkingData.szCurrScenePathInResources + "_" 
      + StringUtil::toString(myWorkingData.u32NumCreatedMeshes++);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO