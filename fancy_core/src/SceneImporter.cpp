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

#include "Scene.h"
#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Mesh.h"
#include "Material.h"
#include "MaterialPass.h"
#include "PathService.h"
#include "ModelComponent.h"
#include "Model.h"
#include "SubModel.h"
#include "Mesh.h"
#include "GeometryData.h"
#include "GeometryVertexLayout.h"
#include "StringUtil.h"
#include "Texture.h"
#include "TextureLoader.h"
#include "FileReader.h"
#include "GpuProgram.h"
#include "GpuProgramCompiler.h"

#define FANCY_IMPORTER_USE_VALIDATION
#include "MathUtil.h"
#include "BinaryCache.h"
#include "JSONwriter.h"
#include "JSONreader.h"
#include "GpuProgramFeatures.h"
#include "VertexInputLayout.h"
#include "GpuProgramPipeline.h"
#include "MaterialPassInstance.h"

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
      log_Info(message);
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
//---------------------------------------------------------------------------//
  namespace Processing
  {
    const uint32 kMaxNumAssimpMeshesPerNode = 128u;
    typedef FixedArray<aiMesh*, kMaxNumAssimpMeshesPerNode> AiMeshList;
    typedef std::map<const aiMaterial*, Rendering::Material*> MaterialCacheMap;
    typedef FixedArray<std::pair<AiMeshList, Geometry::Mesh*>, 256u> MeshCacheList;

    struct WorkingData
    {
      WorkingData() : szCurrScenePathInResources(""), pCurrScene(nullptr),
        u32NumCreatedMeshes(0u), u32NumCreatedModels(0u), u32NumCreatedGeometryDatas(0u), u32NumCreatedSubModels(0u) {}

      std::string szCurrScenePathInResources;
      const aiScene* pCurrScene;
      MaterialCacheMap mapAiMatToMat;
      MeshCacheList localMeshCache;

      uint32 u32NumCreatedMeshes;
      uint32 u32NumCreatedModels;
      uint32 u32NumCreatedGeometryDatas;
      uint32 u32NumCreatedSubModels;
    };

    bool processAiScene(WorkingData& _workingData, const aiScene* _pAscene, Scene::SceneNode* _pParentNode);
    bool processAiNode(WorkingData& _workingData, const aiNode* _pAnode, Scene::SceneNode* _pParentNode);
    bool processMeshes(WorkingData& _workingData, const aiNode* _pAnode, Scene::ModelComponent* _pModelComponent);
    Geometry::Mesh* constructOrRetrieveMesh(WorkingData& _workingData, const aiNode* _pAnode, aiMesh** someMeshes, uint32 aMeshCount);
    Rendering::Material* createOrRetrieveMaterial(WorkingData& _workingData, const aiMaterial* _pAmaterial);
    Rendering::Texture* createOrRetrieveTexture(WorkingData& _workingData, const aiMaterial* _pAmaterial, aiTextureType _aiTextureType, uint32 _texIndex);

    std::string GetCachePathForMesh(WorkingData& _workingData);
    std::string getUniqueModelName(WorkingData& _workingData);
    std::string getUniqueSubModelName(WorkingData& _workingData);
    std::string getUniqueGeometryDataName(WorkingData& _workingData, const aiMesh* _pMesh);
  }
//---------------------------------------------------------------------------//
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
  bool SceneImporter::importToSceneGraph( const std::string& _szImportPathRel, Scene::SceneNode* _pParentNode )
  {
    bool success = false;
    std::string szImportPathAbs = PathService::convertToAbsPath(_szImportPathRel);

    // TODO: Look for cached binary data and don't re-import if possible

    Assimp::Importer aImporter;

    const aiScene* aScene = aImporter.ReadFile(szImportPathAbs,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_FindInstances);

    if (!aScene)
    {
      return false;
    }

    Processing::WorkingData workingData;
    workingData.szCurrScenePathInResources = _szImportPathRel;
    success = Processing::processAiScene(workingData, aScene, _pParentNode);

    // Serialization-tests....
    {
      JSONwriter serializer(szImportPathAbs);
      serializer.serialize(&_pParentNode, "rootNode");
    }

    {
      JSONreader serializer(szImportPathAbs);
      serializer.serialize(&_pParentNode, "rootNode");
    }
    

    return success;
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  bool Processing::processAiScene(WorkingData& _workingData, const aiScene* _pAscene, Scene::SceneNode* _pParentNode)
  {
    const aiNode* pArootNode = _pAscene->mRootNode;

    if (!pArootNode)
    {
      return false;
    }

    _workingData.pCurrScene = _pAscene;
    return processAiNode(_workingData, pArootNode, _pParentNode);
  }
//---------------------------------------------------------------------------//
  bool Processing::processAiNode(WorkingData& _workingData, const aiNode* _pAnode, Scene::SceneNode* _pParentNode)
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
      Processing::processMeshes(_workingData, _pAnode, pModelComponent);
    }
    
    for (uint32 i = 0u; i < _pAnode->mNumChildren; ++i)
    {
      success &= Processing::processAiNode(_workingData, _pAnode->mChildren[i], pNode);
    }

    return success;
  }
//---------------------------------------------------------------------------//
  bool Processing::processMeshes(WorkingData& _workingData, const aiNode* _pAnode, Scene::ModelComponent* _pModelComponent)
  {
    // Sort all meshes by their material. Each entry will become a submodel of the model
    
    typedef FixedArray<aiMesh*, kMaxNumAssimpMeshesPerNode> AssimpMeshList;

    typedef std::map<uint32, AssimpMeshList> MaterialMeshMap;
     MaterialMeshMap mapMaterialIndexMesh;

    const uint32 uNumMeshes = _pAnode->mNumMeshes;
    for (uint32 i = 0u; i < uNumMeshes; ++i)
    {
      const uint32 uMeshIndex = _pAnode->mMeshes[i];
      aiMesh* pAmesh = 
        _workingData.pCurrScene->mMeshes[uMeshIndex];

      const uint32 uMaterialIndex = pAmesh->mMaterialIndex;
      AssimpMeshList& vMeshesWithMaterial = mapMaterialIndexMesh[uMaterialIndex];
      
      if (!vMeshesWithMaterial.contains(pAmesh))
      {
        vMeshesWithMaterial.push_back(pAmesh);
      }
    }

    // Construct or retrieve Fancy Meshes and Submodels
    // Each mesh-list with the same material becomes a submodel
    std::vector<SubModel*> vSubModels;
    for (MaterialMeshMap::iterator it = mapMaterialIndexMesh.begin(); it != mapMaterialIndexMesh.end(); ++it)
    {
      const uint32 uMaterialIndex = it->first;
      AssimpMeshList& vAssimpMeshes = it->second;
      
      Geometry::Mesh* pMesh = constructOrRetrieveMesh(_workingData, _pAnode, &vAssimpMeshes[0], vAssimpMeshes.size());

      // Create or retrieve the material
      aiMaterial* pAmaterial = 
        _workingData.pCurrScene->mMaterials[uMaterialIndex];
      Rendering::Material* pMaterial = Processing::createOrRetrieveMaterial(_workingData, pAmaterial);

      // Do we already have a Submodel with this mesh and material?
      Geometry::SubModelDesc submodelDesc;
      submodelDesc.myMaterial = pMaterial->GetDescription();
      submodelDesc.myMesh = pMesh->GetDescription();

      Geometry::SubModel* pSubModel = Geometry::SubModel::FindFromDesc(submodelDesc);
      
      if (!pSubModel)
      {
        pSubModel = FANCY_NEW(Geometry::SubModel, MemoryCategory::GEOMETRY);
        pSubModel->SetFromDescription(submodelDesc);
        SubModel::Register(pSubModel);
      }

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

    Geometry::Model* pModel = Geometry::Model::FindFromDesc(modelDesc);

    if (!pModel)
    {
      pModel = FANCY_NEW(Geometry::Model, MemoryCategory::GEOMETRY);
      pModel->setName(Processing::getUniqueModelName(_workingData));
      pModel->SetFromDescription(modelDesc);
      Model::Register(pModel);
    }

    _pModelComponent->setModel(pModel);

    return true;
  }
//---------------------------------------------------------------------------//
  uint64 locComputeHashFromVertexData(aiMesh** someMeshes, uint32 aMeshCount)
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
  Geometry::Mesh* Processing::constructOrRetrieveMesh(WorkingData& _workingData, const aiNode* _pANode, aiMesh** someMeshes, uint32 aMeshCount)
  {
    Geometry::Mesh* outputMesh = nullptr;

    // TODO: Refactor this caching-mechanism:
    // We don't save any processing time if we read in the cached mesh, construct its hash and THEN check if we already have this mesh loaded in the engine
    // Instead, do the following:
    // 1) Modify the cache-system so we can "peek ahead" and only retrieve its vertexIndexHash
    // 2) Check if we already have a mesh with this hash, if so --> return this mesh
    // 3) If there isn't any matching Mesh in the engine yet --> load it from cache if the cache-timestamp is newer than the imported scene-file
    // 4) If the scene-file is newer: load the mesh from assimp and update cache file

    ObjectName meshName = Processing::GetCachePathForMesh(_workingData);
    // if (BinaryCache::read(&outputMesh, meshName, 0u))
    // return outputMesh;

    // Did we construct a similar mesh before from the same ai-Meshes?
    auto findMeshInCache = [_workingData, someMeshes, aMeshCount]() -> Geometry::Mesh* {
      for (uint32 i = 0u; i < _workingData.localMeshCache.size(); ++i)
      {
        std::pair<AiMeshList, Geometry::Mesh*> entry = _workingData.localMeshCache[i];

        bool isValid = true;
        for (uint32 iAiMesh = 0u; isValid && iAiMesh < aMeshCount; ++iAiMesh)
          isValid &= entry.first.contains(someMeshes[iAiMesh]);

        if (isValid)
          return entry.second;
      }
      return nullptr;
    };

    outputMesh = findMeshInCache();
    if (outputMesh != nullptr)
      return outputMesh;

    uint64 vertexIndexHash = locComputeHashFromVertexData(someMeshes, aMeshCount);

    // Already loaded this scene before? Then we already have this mesh
    outputMesh = Geometry::Mesh::Find(vertexIndexHash);
    if (outputMesh != nullptr)
      return outputMesh;

    // We have to construct a new mesh...
    GeometryDataList vGeometryDatas;
    FixedArray<void*, Constants::kMaxNumGeometriesPerSubModel> vertexDatas;
    FixedArray<void*, Constants::kMaxNumGeometriesPerSubModel> indexDatas;
    for (uint32 iAiMesh = 0; iAiMesh < aMeshCount; ++iAiMesh)
    {
      const aiMesh* aiMesh = someMeshes[iAiMesh];

      Geometry::GeometryData* pGeometryData = FANCY_NEW(Geometry::GeometryData, MemoryCategory::GEOMETRY);
      vGeometryDatas.push_back(pGeometryData);

      // Construct the vertex layout description
      Rendering::GeometryVertexLayout vertexLayout;
      ASSERT(aiMesh->HasPositions());

      FixedArray<void*, Rendering::kMaxNumGeometryVertexAttributes> vVertexDataPointers;
      FixedArray<uint32, Rendering::kMaxNumInputVertexAttributes> vSourceDataStrides;

      uint32 u32OffsetBytes = 0u;
      if (aiMesh->HasPositions())
      {
        Rendering::GeometryVertexElement vertexElement;
        vertexElement.name = _N(Positions);
        vertexElement.eSemantics = Rendering::VertexSemantics::POSITION;
        vertexElement.eFormat = Rendering::DataFormat::RGB_32F;
        vertexElement.u32OffsetBytes = u32OffsetBytes;
        vertexElement.u32SizeBytes = sizeof(float) * 3u;
        u32OffsetBytes += vertexElement.u32SizeBytes;
        vertexLayout.addVertexElement(vertexElement);
        vVertexDataPointers.push_back(aiMesh->mVertices);
        vSourceDataStrides.push_back(sizeof(aiMesh->mVertices[0]));
        ASSERT(sizeof(aiMesh->mVertices[0]) == vertexElement.u32SizeBytes);
      }

      if (aiMesh->HasNormals())
      {
        Rendering::GeometryVertexElement vertexElement;
        vertexElement.name = _N(Normals);
        vertexElement.eSemantics = Rendering::VertexSemantics::NORMAL;
        vertexElement.eFormat = Rendering::DataFormat::RGB_32F;
        vertexElement.u32OffsetBytes = u32OffsetBytes;
        vertexElement.u32SizeBytes = sizeof(float) * 3u;
        u32OffsetBytes += vertexElement.u32SizeBytes;
        vertexLayout.addVertexElement(vertexElement);
        vVertexDataPointers.push_back(aiMesh->mNormals);
        vSourceDataStrides.push_back(sizeof(aiMesh->mNormals[0]));
        ASSERT(sizeof(aiMesh->mNormals[0]) == vertexElement.u32SizeBytes);
      }

      if (aiMesh->HasTangentsAndBitangents())
      {
        Rendering::GeometryVertexElement vertexElement;
        vertexElement.name = _N(Tangents);
        vertexElement.eSemantics = Rendering::VertexSemantics::TANGENT;
        vertexElement.eFormat = Rendering::DataFormat::RGB_32F;
        vertexElement.u32OffsetBytes = u32OffsetBytes;
        vertexElement.u32SizeBytes = sizeof(float) * 3u;
        u32OffsetBytes += vertexElement.u32SizeBytes;
        vertexLayout.addVertexElement(vertexElement);
        vVertexDataPointers.push_back(aiMesh->mTangents);
        vSourceDataStrides.push_back(sizeof(aiMesh->mTangents[0]));
        ASSERT(sizeof(aiMesh->mTangents[0]) == vertexElement.u32SizeBytes);

        vertexElement = Rendering::GeometryVertexElement();
        vertexElement.name = _N(Bitangents);
        vertexElement.eSemantics = Rendering::VertexSemantics::BITANGENT;
        vertexElement.eFormat = Rendering::DataFormat::RGB_32F;
        vertexElement.u32OffsetBytes = u32OffsetBytes;
        vertexElement.u32SizeBytes = sizeof(float) * 3u;
        u32OffsetBytes += vertexElement.u32SizeBytes;
        vertexLayout.addVertexElement(vertexElement);
        vVertexDataPointers.push_back(aiMesh->mBitangents);
        vSourceDataStrides.push_back(sizeof(aiMesh->mBitangents[0]));
        ASSERT(sizeof(aiMesh->mBitangents[0]) == vertexElement.u32SizeBytes);
      }

      for (uint32 iUVchannel = 0u; iUVchannel < aiMesh->GetNumUVChannels(); ++iUVchannel)
      {
        if (aiMesh->HasTextureCoords(iUVchannel))
        {
          Rendering::GeometryVertexElement vertexElement;
          vertexElement.name = _N(TexCoords);
          vertexElement.eSemantics = VertexSemantics::TEXCOORD;
          vertexElement.mySemanticIndex = iUVchannel;

          if (aiMesh->mNumUVComponents[iUVchannel] == 1u)
          {
            vertexElement.eFormat = Rendering::DataFormat::R_32F;
          }
          else if (aiMesh->mNumUVComponents[iUVchannel] == 2u)
          {
            vertexElement.eFormat = Rendering::DataFormat::RG_32F;
          }
          else if (aiMesh->mNumUVComponents[iUVchannel] == 3u)
          {
            vertexElement.eFormat = Rendering::DataFormat::RGB_32F;
          }
          else
          {
            // Not implemented
            ASSERT(false);
          }

          vertexElement.u32OffsetBytes = u32OffsetBytes;
          vertexElement.u32SizeBytes = sizeof(float) * aiMesh->mNumUVComponents[iUVchannel];
          u32OffsetBytes += vertexElement.u32SizeBytes;
          vertexLayout.addVertexElement(vertexElement);

          // These sizes might differ since assimp stores 2-component UVs in Vec3s
          // ASSERT(sizeof(aiMesh->mTextureCoords[iUVchannel][0]) == vertexElement.u32SizeBytes);

          vSourceDataStrides.push_back(sizeof(aiMesh->mTextureCoords[iUVchannel][0]));
          vVertexDataPointers.push_back(aiMesh->mTextureCoords[iUVchannel]);
        }
      }

      for (uint32 iColorChannel = 0u; iColorChannel < aiMesh->GetNumColorChannels(); ++iColorChannel)
      {
        if (aiMesh->HasVertexColors(iColorChannel))
        {
          Rendering::GeometryVertexElement vertexElement;
          vertexElement.name = _N(Colors);
          vertexElement.eSemantics = VertexSemantics::COLOR;
          vertexElement.mySemanticIndex = iColorChannel;
          vertexElement.eFormat = Rendering::DataFormat::RGBA_32F;
          vertexElement.u32OffsetBytes = u32OffsetBytes;
          vertexElement.u32SizeBytes = sizeof(float) * 4u;
          u32OffsetBytes += vertexElement.u32SizeBytes;
          vertexLayout.addVertexElement(vertexElement);
          ASSERT(sizeof(aiMesh->mColors[iColorChannel][0]) == vertexElement.u32SizeBytes);
          vSourceDataStrides.push_back(sizeof(aiMesh->mColors[iColorChannel][0]));
          vVertexDataPointers.push_back(aiMesh->mColors[iColorChannel]);
        }
      }

      if (aiMesh->HasBones())
      {
        log_Warning("Bone data is currently not supported in FANCY");
      }

      const uint uSizeVertexBufferBytes = vertexLayout.getStrideBytes() * aiMesh->mNumVertices;
      void* pData = FANCY_ALLOCATE(uSizeVertexBufferBytes, MemoryCategory::GEOMETRY);

      if (!pData)
      {
        log_Error("Failed to allocate vertex buffer");
        return false;
      }

      // Construct an interleaved vertex array
      ASSERT(vVertexDataPointers.size() == vertexLayout.getNumVertexElements());
      for (uint iVertex = 0u; iVertex < aiMesh->mNumVertices; ++iVertex)
      {
        for (uint iVertexElement = 0u; iVertexElement < vertexLayout.getNumVertexElements(); ++iVertexElement)
        {
          const Rendering::GeometryVertexElement& rVertexElement = vertexLayout.getVertexElement(iVertexElement);
          uint uInterleavedOffset = iVertex * vertexLayout.getStrideBytes() + rVertexElement.u32OffsetBytes;
          uint uContinousOffset = iVertex * vSourceDataStrides[iVertexElement];
          void* pDataPointer = vVertexDataPointers[iVertexElement];
          memcpy(static_cast<uint8*>(pData)+uInterleavedOffset, static_cast<uint8*>(pDataPointer)+uContinousOffset, rVertexElement.u32SizeBytes);
        }
      }

      // Construct the actual vertex buffer object
      Rendering::GpuBuffer* vertexBuffer =
        FANCY_NEW(Rendering::GpuBuffer, MemoryCategory::GEOMETRY);

      Rendering::GpuBufferCreationParams bufferParams;
      bufferParams.bIsMultiBuffered = false;
      bufferParams.ePrimaryUsageType = Rendering::GpuBufferUsage::VERTEX_BUFFER;
      bufferParams.uAccessFlags = static_cast<uint>(Rendering::GpuResourceAccessFlags::NONE);
      bufferParams.uNumElements = aiMesh->mNumVertices;
      bufferParams.uElementSizeBytes = vertexLayout.getStrideBytes();

      vertexBuffer->create(bufferParams, pData);
      pGeometryData->setVertexLayout(vertexLayout);
      pGeometryData->setVertexBuffer(vertexBuffer);

      vertexDatas.push_back(pData);
      

      /// Construct the index buffer
#if defined (FANCY_IMPORTER_USE_VALIDATION)
      // Ensure that we have only triangles
      for (uint i = 0u; i < aiMesh->mNumFaces; ++i)
      {
        ASSERT_M(aiMesh->mFaces[i].mNumIndices == 3u, "Unsupported face type");
      }
#endif  // FANCY_IMPORTER_USE_VALIDATION

      uint32* indices = FANCY_NEW(uint32[aiMesh->mNumFaces * 3u], MemoryCategory::GEOMETRY);

      for (uint i = 0u; i < aiMesh->mNumFaces; ++i)
      {
        const aiFace& aFace = aiMesh->mFaces[i];

        ASSERT(sizeof(indices[0]) == sizeof(aFace.mIndices[0]));
        memcpy(&indices[i * 3u], aFace.mIndices, sizeof(uint32) * 3u);
      }

      Rendering::GpuBuffer* indexBuffer =
        FANCY_NEW(Rendering::GpuBuffer, MemoryCategory::GEOMETRY);

      Rendering::GpuBufferCreationParams indexBufParams;
      indexBufParams.bIsMultiBuffered = false;
      indexBufParams.ePrimaryUsageType = Rendering::GpuBufferUsage::INDEX_BUFFER;
      indexBufParams.uAccessFlags = static_cast<uint32>(Rendering::GpuResourceAccessFlags::NONE);
      indexBufParams.uNumElements = aiMesh->mNumFaces * 3u;
      indexBufParams.uElementSizeBytes = sizeof(uint32);

      indexBuffer->create(indexBufParams, indices);
      pGeometryData->setIndexBuffer(indexBuffer);
      indexDatas.push_back(indices);
    }

    Geometry::Mesh* mesh = FANCY_NEW(Geometry::Mesh, MemoryCategory::GEOMETRY);
    mesh->setName(meshName);
    mesh->setGeometryDataList(vGeometryDatas);
    mesh->SetVertexIndexHash(vertexIndexHash);
    Geometry::Mesh::Register(mesh);
    
    BinaryCache::write(mesh, &vertexDatas[0], &indexDatas[0]);

    AiMeshList aiMeshList;
    aiMeshList.resize(aMeshCount);
    memcpy(&aiMeshList[0], &someMeshes[0], sizeof(aiMesh*) * aMeshCount);
    _workingData.localMeshCache.push_back(std::pair<AiMeshList, Geometry::Mesh*>(aiMeshList, mesh));

    for (uint32 i = 0u; i < vertexDatas.size(); ++i)
    {
      FANCY_FREE(vertexDatas[i], MemoryCategory::GEOMETRY);
    }
    vertexDatas.clear();

    for (uint32 i = 0u; i < indexDatas.size(); ++i)
    {
      FANCY_DELETE_ARR(indexDatas[i], MemoryCategory::GEOMETRY);
    }
    indexDatas.clear();
    
    return mesh;
  }
//---------------------------------------------------------------------------//
  Rendering::Material* Processing::createOrRetrieveMaterial(WorkingData& _workingData, const aiMaterial* _pAmaterial)
  {
    // Did we already import this material?
    Processing::MaterialCacheMap::iterator cacheIt = _workingData.mapAiMatToMat.find(_pAmaterial);
    if (cacheIt != _workingData.mapAiMatToMat.end())
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

    Texture* pDiffuseTex = createOrRetrieveTexture(_workingData, _pAmaterial, aiTextureType_DIFFUSE, 0u);
    Texture* pNormalTex = createOrRetrieveTexture(_workingData, _pAmaterial, aiTextureType_NORMALS, 0u);
    Texture* pSpecularTex = createOrRetrieveTexture(_workingData, _pAmaterial, aiTextureType_SPECULAR, 0u);
    Texture* pSpecPowerTex = createOrRetrieveTexture(_workingData, _pAmaterial, aiTextureType_SHININESS, 0u);
    Texture* pOpacityTex = createOrRetrieveTexture(_workingData, _pAmaterial, aiTextureType_OPACITY, 0u);

    bool hasDiffuseTex = pDiffuseTex != nullptr;
    bool hasNormalTex = pDiffuseTex != nullptr;
    bool hasSpecTex = pSpecularTex != nullptr;
    bool hasSpecPowerTex = pSpecPowerTex != nullptr;
    bool hasOpacityTex = pOpacityTex != nullptr;

    // Fancy only supports textures where opacity is combined with diffuse and specPower is combined with specular
    if (hasSpecTex && hasSpecPowerTex && pSpecPowerTex != pSpecularTex)
    {
      log_Warning("Fancy doesn't support storing specular power in a separate texture. Consider putting it in spec.a");
    }

    if (hasDiffuseTex && hasOpacityTex && pOpacityTex != pDiffuseTex)
    {
      log_Warning("Fancy doesn't support storing opacity in a separate texture. Consider putting it in diff.a");
    }

    // Find/Create matching Shaders
    //---------------------------------------------------------------------------//
    GpuProgramPermutation permutation;
    if (hasDiffuseTex) permutation.addFeature(GpuProgramFeature::FEAT_ALBEDO_TEXTURE);
    if (hasNormalTex) permutation.addFeature(GpuProgramFeature::FEAT_NORMAL_MAPPED);
    if (hasSpecTex) permutation.addFeature(GpuProgramFeature::FEAT_SPECULAR); permutation.addFeature(GpuProgramFeature::FEAT_SPECULAR_TEXTURE);

    GpuProgramDesc vertexProgramDesc;
    vertexProgramDesc.myPermutation = permutation;
    vertexProgramDesc.myShaderPath = Rendering::GpuProgramCompiler::GetPlatformShaderFileDirectory() + 
      "MaterialForward" + Rendering::GpuProgramCompiler::GetPlatformShaderFileExtension();
    vertexProgramDesc.myShaderStage = static_cast<uint32>(ShaderStage::VERTEX);
    GpuProgram* pVertexProgram = GpuProgramCompiler::createOrRetrieve(vertexProgramDesc);

    GpuProgramDesc fragmentProgramDesc = vertexProgramDesc;
    fragmentProgramDesc.myShaderStage = static_cast<uint32>(ShaderStage::FRAGMENT);
    GpuProgram* pFragmentProgram = GpuProgramCompiler::createOrRetrieve(fragmentProgramDesc);
    //---------------------------------------------------------------------------//


    // Find/Create a matching GpuProgramPipeline
    //---------------------------------------------------------------------------//
    GpuProgramPipelineDesc pipelineDesc;
    pipelineDesc.myGpuPrograms[(uint32)ShaderStage::VERTEX] = vertexProgramDesc;
    pipelineDesc.myGpuPrograms[(uint32)ShaderStage::FRAGMENT] = fragmentProgramDesc;
    GpuProgramPipeline* pipeline = GpuProgramPipeline::FindFromDesc(pipelineDesc);
    if (pipeline == nullptr)
    {
      pipeline = FANCY_NEW(GpuProgramPipeline, MemoryCategory::MATERIALS);
      pipeline->SetFromDescription(pipelineDesc);
      GpuProgramPipeline::Register(pipeline);
    }
    //---------------------------------------------------------------------------//


    // Find/Create a matching MaterialPass
    //---------------------------------------------------------------------------//
    MaterialPassDesc matPassDesc;
    matPassDesc.m_BlendStateDesc = BlendStateDesc::GetDefaultSolid();
    matPassDesc.m_DepthStencilStateDesc = DepthStencilStateDesc::GetDefaultDepthNoStencil();
    matPassDesc.m_GpuProgramPipelineDesc = pipelineDesc;
    matPassDesc.m_eCullMode = static_cast<uint32>(CullMode::BACK);
    matPassDesc.m_eFillMode = static_cast<uint32>(FillMode::SOLID);
    matPassDesc.m_eWindingOrder = static_cast<uint32>(WindingOrder::CCW);
    
    MaterialPass* pMaterialPass = MaterialPass::FindFromDesc(matPassDesc);
    if (pMaterialPass == nullptr)
    {
      pMaterialPass = FANCY_NEW(MaterialPass, MemoryCategory::MATERIALS);
      pMaterialPass->SetFromDescription(matPassDesc);
      MaterialPass::Register(pMaterialPass);
    }
    //---------------------------------------------------------------------------//


    // Find/Create a matching MaterialPassInstance
    //---------------------------------------------------------------------------//
    MaterialPassInstanceDesc mpiDesc;
    mpiDesc.myMaterialPass = matPassDesc;
    mpiDesc.myReadTextures[0u] = pDiffuseTex->GetDescription();
    mpiDesc.myReadTextures[1u] = pNormalTex->GetDescription();
    mpiDesc.myReadTextures[2u] = pSpecularTex->GetDescription();
    MaterialPassInstance* pSolidForwardMpi = MaterialPassInstance::FindFromDesc(mpiDesc);
    if (pSolidForwardMpi == nullptr)
    {
      pSolidForwardMpi = FANCY_NEW(MaterialPassInstance(pMaterialPass), MemoryCategory::MATERIALS);
      pSolidForwardMpi->SetFromDescription(mpiDesc);
      MaterialPassInstance::Register(pSolidForwardMpi);
    }
    //---------------------------------------------------------------------------//

    // Find/Create a matching Material
    //---------------------------------------------------------------------------//
    MaterialDesc matDesc;
    matDesc.myPasses[(uint32)EMaterialPass::SOLID_FORWARD] = mpiDesc;
    
    if (hasColor)
      matDesc.myParameters[(uint32)EMaterialParameterSemantic::DIFFUSE_REFLECTIVITY] = (color_diffuse.r + color_diffuse.g + color_diffuse.b) * (1.0f / 3.0f);
    if (hasSpecular)
      matDesc.myParameters[(uint32)EMaterialParameterSemantic::SPECULAR_REFLECTIVITY] = specular;
    if (hasOpacity)
      matDesc.myParameters[(uint32)EMaterialParameterSemantic::OPACITY] = opacity;
    if (hasSpecularPower)
      matDesc.myParameters[(uint32)EMaterialParameterSemantic::SPECULAR_POWER] = specularPower;
    
    Material* pMaterial = Material::FindFromDesc(matDesc);
    if (pMaterial == nullptr)
    {
      pMaterial = FANCY_NEW(Material, MemoryCategory::MATERIALS);
      pMaterial->SetFromDescription(matDesc);
      Material::Register(pMaterial);
    }
    //---------------------------------------------------------------------------//

    return pMaterial;
  }
//---------------------------------------------------------------------------//
  Rendering::Texture* Processing::createOrRetrieveTexture(WorkingData& _workingData, 
    const aiMaterial* _pAmaterial, aiTextureType _aiTextureType, uint32 _texIndex)
  {
    uint32 numTextures = _pAmaterial->GetTextureCount(_aiTextureType);
    if (numTextures == 0u)
    {
      return nullptr;
    }

    ASSERT(numTextures > _texIndex);

    aiString szATexPath;
    _pAmaterial->Get(AI_MATKEY_TEXTURE(_aiTextureType, _texIndex), szATexPath);

    String szTexPath = String(szATexPath.C_Str());
    
    if (!PathService::isAbsolutePath(szTexPath))
    {
      String absSceneFolderPath = _workingData.szCurrScenePathInResources;
      PathService::convertToAbsPath(absSceneFolderPath);
      PathService::removeFilenameFromPath(absSceneFolderPath);
      szTexPath = absSceneFolderPath + szTexPath;
    }

    PathService::removeFolderUpMarkers(szTexPath);

    ObjectName textureName = PathService::toRelPath(szTexPath);

    // Did we already load this texture before?
    Texture* pTexture = Texture::getByName(textureName);
    if (pTexture)
    {
      return pTexture;
    }

    // Try to load the texture from cache
    /*pTexture = BinaryCache::get<Texture>(textureName);
    if (pTexture)
    {
      return pTexture;
    }*/

    // Load and decode the texture to memory
    std::vector<uint8> vTextureBytes;
    TextureLoadInfo texLoadInfo;
    if (!TextureLoader::loadTexture(szTexPath, vTextureBytes, texLoadInfo))
    {
      log_Error("Failed to load texture at path " + szTexPath);
      return nullptr;
    }

    if (texLoadInfo.bitsPerPixel / texLoadInfo.numChannels != 8u)
    {
      log_Error("Unsupported texture format: " + szTexPath);
      return nullptr;
    }

    pTexture = FANCY_NEW(Texture, MemoryCategory::TEXTURES);
    
    TextureCreationParams texParams;
    texParams.bIsDepthStencil = false;
    texParams.eFormat = texLoadInfo.numChannels == 3u ? DataFormat::SRGB_8 : DataFormat::SRGB_8_A_8;
    texParams.u16Width = texLoadInfo.width;
    texParams.u16Height = texLoadInfo.height;
    texParams.u16Depth = 0u;
    texParams.uAccessFlags = (uint32) GpuResourceAccessFlags::NONE;
    texParams.uPixelDataSizeBytes = (texLoadInfo.width * texLoadInfo.height * texLoadInfo.bitsPerPixel) / 8u;
    texParams.pPixelData = &vTextureBytes[0];
    pTexture->create(texParams);
    pTexture->setPath(textureName);

    if (!pTexture->isValid())
    {
      log_Error("Failed to upload pixel data of texture " + szTexPath);
      FANCY_DELETE(pTexture, MemoryCategory::TEXTURES);
      pTexture = nullptr;
    }
    else
    {
      BinaryCache::write(pTexture, texParams.pPixelData, texParams.uPixelDataSizeBytes);
      Texture::registerWithName(textureName, pTexture);  
    }

    return pTexture;
  }
//---------------------------------------------------------------------------//
  std::string Processing::getUniqueModelName(WorkingData& _workingData)
  {
    return "Model_" + _workingData.szCurrScenePathInResources + "_" 
      + StringUtil::toString(_workingData.u32NumCreatedModels++);
  }
//---------------------------------------------------------------------------//
  std::string Processing::GetCachePathForMesh(WorkingData& _workingData)
  {
    return "Mesh_" + _workingData.szCurrScenePathInResources + "_" 
      + StringUtil::toString(_workingData.u32NumCreatedMeshes++);
  }
//---------------------------------------------------------------------------//
  std::string Processing::getUniqueGeometryDataName(WorkingData& _workingData, const aiMesh* _pMesh)
  {
    return "GeometryData_" + _workingData.szCurrScenePathInResources + "_" 
      + std::string(_pMesh->mName.C_Str()) + "_"
      + StringUtil::toString(_workingData.u32NumCreatedGeometryDatas++);
  }
//---------------------------------------------------------------------------//
  std::string Processing::getUniqueSubModelName(WorkingData& _workingData)
  {
    return "SubModel_" + _workingData.szCurrScenePathInResources + "_" 
      + StringUtil::toString(_workingData.u32NumCreatedSubModels++);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO


// Legacy:
// Modelloder:

/*
const float fGlobalNormalMod = 1.0f;


ModelLoader::ModelLoader()
{
  const uint uLogSeverity = Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Err | Assimp::Logger::Warn;
  Assimp::DefaultLogger::get()->attachStream( &m_clLogStream, uLogSeverity );
}

ModelLoader::~ModelLoader()
{

}

Mesh* ModelLoader::LoadSingleMeshGeometry(  const String& szModelPath )
{
  String szAbsPath = PathService::convertToAbsPath( szModelPath );
  String szModelFolder = PathService::GetContainingFolder( szModelPath );

  const aiScene* pAiScene = m_aiImporter.ReadFile( szAbsPath, aiProcess_CalcTangentSpace |
                                aiProcess_JoinIdenticalVertices |
                                aiProcess_Triangulate );

  if( !pAiScene )
  {
    LOG( String( "Import failed of Scene : " ) + szModelPath );
    return NULL;
  }

  if( pAiScene->mNumMeshes == 0 )
  {
    LOG( String( "No Mesh in Scene " ) + szModelPath );
    return NULL;
  }

  else if( pAiScene->mNumMeshes > 1 )
    LOG( String( "WARNING: Loading single Mesh from File with more Meshes! " ) + szModelPath );


  return processMesh( pAiScene, pAiScene->mMeshes[ 0 ], szModelPath, NULL, 0, false );
}


SceneNode* ModelLoader::LoadAsset( const String& szModelPath, SceneManager* pScene )
{
  String szAbsPath = PathService::convertToAbsPath( szModelPath );
  String szModelFolder = PathService::GetContainingFolder( szModelPath );

  const aiScene* pAiScene = m_aiImporter.ReadFile( szAbsPath, aiProcess_CalcTangentSpace |
                               aiProcess_JoinIdenticalVertices |
                               aiProcess_Triangulate );

  if( !pAiScene )
  {
    LOG( String( "Import failed of Scene : " ) + szModelPath );
    return NULL;
  }

  Material** vpMaterials = new Material*[ pAiScene->mNumMaterials ];
  for( int i = 0; i < pAiScene->mNumMaterials; ++i )
  {
    vpMaterials[ i ] = processMaterial( pAiScene, pAiScene->mMaterials[ i ], szModelFolder );
  }


  Mesh** vpMeshes = new Mesh*[ pAiScene->mNumMeshes ];
  for( int i = 0; i < pAiScene->mNumMeshes; ++i )
  {
    vpMeshes[ i ] = processMesh( pAiScene, pAiScene->mMeshes[ i ], szModelPath, vpMaterials, i );
  }

  SceneNode* pAssetRootNode = new SceneNode( szModelPath );
  processNode( pScene, pAiScene, pAssetRootNode, pAiScene->mRootNode, vpMeshes );


  //Materials are cloned into the meshes and Meshes are cloned into the model - so these lists can be deleted (assuming the cloning is bug-free ;=) )
  for( int i = 0; i < pAiScene->mNumMaterials; ++i )
    delete vpMaterials[ i ];

  delete[] vpMaterials;

  for( int i = 0; i < pAiScene->mNumMeshes; ++i )
    delete vpMeshes[ i ];

  delete[] vpMeshes;

  m_aiImporter.FreeScene();
  
  return pAssetRootNode;
}

Material* ModelLoader::processMaterial( const aiScene* pAiScene, const aiMaterial* paiMaterial, const String& szModelFolder )
{
  Material* returnMaterial = NULL;

  aiString szName;
  paiMaterial->Get( AI_MATKEY_NAME, szName );
    
  //Determine the Textures defined - and choose the exact type of Engine-Material depending on that
  uint uNumTexturesDiffuse = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_DIFFUSE );
  uint uNumTexturesAmbient = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_AMBIENT );
  uint uNumTexturesNormal = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_NORMALS );
  uint uNumTexturesSpecular = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_SPECULAR );
  uint uNumTexturesGloss = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_SHININESS );

  uint uNumTexturesUnknown = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_UNKNOWN );
  if( uNumTexturesUnknown != 0 )
    LOG( std::string( "WARNING: There are unknown textures defined in Material " ) + std::string( szName.data ) );


  //////////////////////////////////////////////////////////////////////////
  // MAT_Colored
  //////////////////////////////////////////////////////////////////////////
  if(	uNumTexturesDiffuse == 0 && 
        uNumTexturesNormal == 0 && 
        uNumTexturesGloss == 0 && 
        uNumTexturesSpecular == 0 )
  {
    MAT_Colored* pMat = new MAT_Colored();
    pMat->Init();

    returnMaterial = pMat;
  }

  //////////////////////////////////////////////////////////////////////////
  //MAT_TEXTURED
  //////////////////////////////////////////////////////////////////////////
  else if(	uNumTexturesDiffuse >= 1 && 
        uNumTexturesNormal == 0 && 
        uNumTexturesGloss == 0 && 
        uNumTexturesSpecular == 0 )
  {
    MAT_Textured* pMat = new MAT_Textured();
    pMat->Init();

    aiString aiSzDiffTexture;
    //TODO:add mapping, etc.-support
    paiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &aiSzDiffTexture );
    pMat->GetDiffuseTexture().SetTexture( szModelFolder + String( aiSzDiffTexture.data ) );

    returnMaterial = pMat;
  }


  //////////////////////////////////////////////////////////////////////////
  //MAT_TexturedNormal
  //////////////////////////////////////////////////////////////////////////
  else if(	uNumTexturesDiffuse >= 1 && 
        uNumTexturesNormal >= 1 && 
        uNumTexturesGloss == 0 && 
        uNumTexturesSpecular == 0 )
  {
    MAT_TexturedNormal* pMat = new MAT_TexturedNormal();
    pMat->Init();

    aiString aiSzDiffTexture;
    
    //TODO:add mapping, etc.-support
    paiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &aiSzDiffTexture );
    pMat->GetDiffuseTexture().SetTexture( szModelFolder + String( aiSzDiffTexture.data ) );

    aiString aiSzNormTexture;

    //TODO:add mapping, etc.-support
    paiMaterial->GetTexture( aiTextureType_NORMALS, 0, &aiSzNormTexture );
    pMat->GetNormalTexture().SetTexture( szModelFolder + String( aiSzNormTexture.data ) );

    float fBumpIntensity = 1.0f;
    paiMaterial->Get( AI_MATKEY_BUMPSCALING, fBumpIntensity );
    pMat->SetBumpIntensity( fBumpIntensity * fGlobalNormalMod );

    returnMaterial = pMat;
  }


  //////////////////////////////////////////////////////////////////////////
  //MAT_TexturedNormalSpecular
  //////////////////////////////////////////////////////////////////////////
  else if(	uNumTexturesDiffuse >= 1 && 
        uNumTexturesNormal >= 1 && 
        ( uNumTexturesGloss >= 1 || uNumTexturesSpecular >= 1 )
      )
  {
    MAT_TexturedNormalSpecular* pMat = new MAT_TexturedNormalSpecular();
    pMat->Init();

    aiString aiSzTexture;

    //TODO:add mapping, etc.-support
    paiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &aiSzTexture );
    pMat->GetDiffuseTexture().SetTexture( szModelFolder + String( aiSzTexture.data ) );

    //TODO:add mapping, etc.-support
    paiMaterial->GetTexture( aiTextureType_NORMALS, 0, &aiSzTexture );
    pMat->GetNormalTexture().SetTexture( szModelFolder + String( aiSzTexture.data ) );

    if( uNumTexturesGloss )
    {
      //TODO:add mapping, etc.-support
      paiMaterial->GetTexture( aiTextureType_SHININESS, 0, &aiSzTexture );
      pMat->GetGlossTexture().SetTexture( szModelFolder + String( aiSzTexture.data ) );
    }

    if( uNumTexturesSpecular )
    {
      //TODO:add mapping, etc.-support
      paiMaterial->GetTexture( aiTextureType_SPECULAR, 0, &aiSzTexture );
      pMat->GetSpecularTexture().SetTexture( szModelFolder + String( aiSzTexture.data ) );
    }
    
    float fBumpIntensity = 1.0f;
    paiMaterial->Get( AI_MATKEY_BUMPSCALING, fBumpIntensity );
    pMat->SetBumpIntensity( fBumpIntensity * fGlobalNormalMod );

    returnMaterial = pMat;
  }

  

  //////////////////////////////////////////////////////////////////////////
  // MAT_TEST to indicade missing material information
  //////////////////////////////////////////////////////////////////////////
  else
  {
    MAT_Test* pMat = new MAT_Test;
    pMat->Init();

    returnMaterial = pMat;
  }

  //Note: Uncomment to debug with single color
  MAT_Colored* pMat = new MAT_Colored();
  pMat->Init();

  returnMaterial = pMat;
  
  

  //Get Properties that all materials have in common
  aiColor3D diffColor ( 0.0f, 0.0f, 0.0f );
  paiMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, diffColor );

  aiColor3D ambColor ( 1.0f, 1.0f, 1.0f );
  paiMaterial->Get( AI_MATKEY_COLOR_AMBIENT, ambColor );

  float fOpacity = 1.0f;
  paiMaterial->Get( AI_MATKEY_OPACITY, fOpacity );

  aiColor3D specColor ( 0.0f, 0.0f, 0.0f );
  paiMaterial->Get( AI_MATKEY_COLOR_SPECULAR, specColor );
  
  float fSpecExponent = 0.0f;
  paiMaterial->Get( AI_MATKEY_SHININESS, fSpecExponent );

  float fGloss = 0.0f;
  paiMaterial->Get( AI_MATKEY_SHININESS_STRENGTH, fGloss );

  returnMaterial->SetColor( glm::vec3( diffColor.r, diffColor.g, diffColor.b ) );
  returnMaterial->SetAmbientReflectivity( glm::vec3( ambColor.r, ambColor.g, ambColor.b ) );
  returnMaterial->SetOpacity( fOpacity );
  returnMaterial->SetSpecularColor(  glm::vec3( specColor.r, specColor.g, specColor.b ) );

  if( fSpecExponent > 255.0f ) 
    fSpecExponent = 255.0f;

  returnMaterial->SetSpecularExponent( fSpecExponent / 255.0f ); //conversion to 0...1
  returnMaterial->SetGlossiness( fGloss 1.0f ); //Hack for now till I figured out a nice way to import that from maya...
  returnMaterial->SetDiffuseReflectivity( glm::vec3( 1.0f, 1.0f, 1.0f ) ); //Hack for now till I figured out a nice way to import that from maya...

  return returnMaterial;
}

Mesh* ModelLoader::processMesh( const aiScene* pAiScene, aiMesh* paiMesh, const String& szModelPath, Material** vpMaterials, uint i, bool assignMaterial  = true  )
{
  GLVBOpathManager& rVBOpathMgr = GLVBOpathManager::GetInstance();
  GLIBOpathManager& rIBOpathMgr = GLIBOpathManager::GetInstance();
  GLBufferUploader& rBufferUploader = GLBufferUploader::GetInstance();

  Mesh* pMesh = new Mesh;
  
  String szName = "";

  if( paiMesh->mName.length > 0 )
    szName = szModelPath + std::string( paiMesh->mName.data );

  else
  {
    stringstream ss;
    ss << szModelPath << "_Mesh_" << i;
    szName = ss.str();
  }

  pMesh->SetName( szName );
    
  if( assignMaterial )
    pMesh->SetMaterial( vpMaterials[ paiMesh->mMaterialIndex ]->Clone() );

  VertexDeclaration* pVertexInfo = new VertexDeclaration;
  
  if( !paiMesh->HasPositions() )
  {
    LOG( std::string( "ERROR: Mesh " ) + pMesh->GetName() + std::string( " has no Vertex Positions!" ) );
    delete pMesh;
    return NULL;
  }
  
  //POSITION
  pVertexInfo->AddVertexElement( VertexElement( 0, Vertex::DataType::FLOAT3, ShaderSemantics::POSITION ) );
  
  //NORMAL
  if( paiMesh->HasNormals() )
    pVertexInfo->AddVertexElement( pVertexInfo->ComputeCurrentOffset(), Vertex::DataType::FLOAT3, ShaderSemantics::NORMAL );

  //UVs
  for( int i = 0; i < paiMesh->GetNumUVChannels() && i < ShaderSemantics::MAX_UV_CHANNELS; ++i )
    if( paiMesh->HasTextureCoords( i ) )
      pVertexInfo->AddVertexElement( pVertexInfo->ComputeCurrentOffset(), Vertex::DataType::FLOAT2, (ShaderSemantics::Semantic) ( ShaderSemantics::UV0 + i ) );
  
  //TANGENT
  if( paiMesh->HasTangentsAndBitangents() )
    //Currently only support for Tangents. Bitangents are calculated in the shaders
    pVertexInfo->AddVertexElement( pVertexInfo->ComputeCurrentOffset(), Vertex::DataType::FLOAT3, ShaderSemantics::TANGENT );

  
  //TODO: Support bones, VertexColors etc. in the future!


  if( !rVBOpathMgr.HasResource( pMesh->GetName() ) )
  {
    uint8* pVBOdata = (uint8*) malloc( pVertexInfo->GetStride() * paiMesh->mNumVertices );
    memset( pVBOdata, 0, pVertexInfo->GetStride() * paiMesh->mNumVertices );

    const std::vector<VertexElement>& vVertexElements = pVertexInfo->GetVertexElements();


    for( int i = 0; i < paiMesh->mNumVertices; ++i )
    {
      uint8* pVertex = &pVBOdata[ pVertexInfo->GetStride() * i ];

      for( int iVelement = 0; iVelement < vVertexElements.size(); ++iVelement )
      {
        const VertexElement& clElement = vVertexElements[ iVelement ];

        switch( clElement.GetSemantic() )
        {
        case ShaderSemantics::POSITION:
          {
            aiVector3D pos = paiMesh->mVertices[ i ];
            glm::vec3* pVal = reinterpret_cast<glm::vec3*>( pVertex );
            pVal->x = pos.x;
            pVal->y = pos.y;
            pVal->z = pos.z;

            ++pVal;
            pVertex = (uint8*) pVal;
          }
          break;

        case ShaderSemantics::NORMAL:
          {
            aiVector3D norm = paiMesh->mNormals[ i ];
            glm::vec3* pVal = reinterpret_cast<glm::vec3*>( pVertex );
            pVal->x = norm.x;
            pVal->y = norm.y;
            pVal->z = norm.z;

            ++pVal;
            pVertex = (uint8*) pVal;

          }
          break;

        case ShaderSemantics::UV0:
        case ShaderSemantics::UV1:
        case ShaderSemantics::UV2:
        case ShaderSemantics::UV3:
        case ShaderSemantics::UV4:
        case ShaderSemantics::UV5:
        case ShaderSemantics::UV6:
        case ShaderSemantics::UV7:
          {
            aiVector3D uv = (paiMesh->mTextureCoords[ clElement.GetSemantic() - ShaderSemantics::UV0 ])[ i ];
            glm::vec2* pVal = reinterpret_cast<glm::vec2*>( pVertex );
            pVal->x = uv.x;
            pVal->y = uv.y;

            ++pVal;
            pVertex = (uint8*) pVal;
          }
          break;

        case ShaderSemantics::TANGENT:
          {
            aiVector3D tangent = paiMesh->mTangents[ i ];
            glm::vec3* pVal = reinterpret_cast<glm::vec3*>( pVertex );
            pVal->x = tangent.x;
            pVal->y = tangent.y;
            pVal->z = tangent.z;

            ++pVal;
            pVertex = (uint8*) pVal;
          }
          break;
        }
      }
    }

    GLuint uVBO = rBufferUploader.UploadBufferData( pVBOdata, paiMesh->mNumVertices, pVertexInfo->GetStride(), GL_ARRAY_BUFFER, GL_STATIC_DRAW );

    rVBOpathMgr.AddResource( pMesh->GetName(), uVBO );

    free( pVBOdata );

  } //end if( !rVBOpathMgr.HasResource( pMesh->GetName() ) )

  pVertexInfo->m_uVertexBufferLoc = rVBOpathMgr.GetResource( pMesh->GetName() );
  pVertexInfo->m_u32VertexCount = paiMesh->mNumVertices;
  pVertexInfo->m_bUseIndices = paiMesh->HasFaces();
  pVertexInfo->m_uPrimitiveType = GL_TRIANGLES;

  if( paiMesh->HasFaces() )
  {
    if( !rIBOpathMgr.HasResource( pMesh->GetName() ) )
    {
      //NOTE: Assuming that faces are always triangles
      uint uNumIndices = paiMesh->mNumFaces * 3;

      uint* pIBOdata = (uint*) malloc( sizeof( uint ) * uNumIndices );

      uint uIdx = 0;
      for( int iFace = 0; iFace < paiMesh->mNumFaces; ++iFace )
      {
        aiFace& rFace = paiMesh->mFaces[ iFace ];

        for( int iFaceIndex = 0; iFaceIndex < rFace.mNumIndices; ++iFaceIndex )
        {
          pIBOdata[ uIdx++ ] = rFace.mIndices[ iFaceIndex ];
        }
      }
      
      GLuint uIBO = rBufferUploader.UploadBufferData( pIBOdata, uNumIndices, sizeof( uint ), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW );

      rIBOpathMgr.AddResource( pMesh->GetName(), uIBO );

      free( pIBOdata );
    }

    pVertexInfo->m_u32IndexCount = paiMesh->mNumFaces * 3;
    pVertexInfo->m_uIndexBufferLoc = rIBOpathMgr.GetResource( pMesh->GetName() );

  } //end if( paiMesh->HasFaces() )
    
  
  //Translate ai-positions to glm-positions
  //Ugly but necessary for mesh-initialization for now
  std::vector<glm::vec3> vPositions;
  for( int i = 0; i < paiMesh->mNumVertices; ++i )
    vPositions.push_back( glm::vec3( paiMesh->mVertices[ i ].x, paiMesh->mVertices[ i ].y, paiMesh->mVertices[ i ].z ) );

  pMesh->Init( vPositions );

  //Append the vertex-Info to the mesh. Thereby, the Resource manager gets notified and adds a reference count for the gl-buffers
  pMesh->SetVertexInfo( pVertexInfo );
  
  return pMesh;	
}


void ModelLoader::processNode( SceneManager* pScene, const aiScene* pAiScene, SceneNode* pNode, aiNode* pAiNode, Mesh** vMeshes )
{
  SceneNode* pCurrNode = pNode->createChildSceneNode( String( pAiNode->mName.data ) );
  pCurrNode->setTransform( matFromAiMat( pAiNode->mTransformation ) );
  
  for( int i = 0; i < pAiNode->mNumMeshes; ++i )
  {
    uint uMeshIdx = pAiNode->mMeshes[ i ];
    Mesh* pCurrMesh = new Mesh( *vMeshes[ uMeshIdx ] ); //Clone the mesh to allow multiple meshes defined in one file
    Entity* pEntity = pScene->CreateEntity( std::unique_ptr<Mesh>( pCurrMesh ) );
    pCurrNode->attatchEntity( pEntity );
  }

  for( int i = 0; i < pAiNode->mNumChildren; ++i )
  {
    aiNode* paiChildNode = pAiNode->mChildren[ i ];
    processNode( pScene, pAiScene, pCurrNode, paiChildNode, vMeshes ); //Recursively handle the child nodes
  }
}


glm::mat4 ModelLoader::matFromAiMat( const aiMatrix4x4& mat )
{
  return glm::mat4(	mat.a1, mat.a2, mat.a3, mat.a4,
            mat.b1, mat.b2, mat.b3, mat.b4,
            mat.c1, mat.c2, mat.c3, mat.c4,
            mat.d1, mat.d2, mat.d3, mat.d4 );
}

*/
