#include "ModelLoader.h"

#include <assimp/config.h>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>
#include <xxHash/xxhash.h>

#include <unordered_map>
#include <unordered_set>
#include "SubModel.h"
#include "fancy_core/MathUtil.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/Mesh.h"

namespace Fancy { namespace ModelLoader {
//---------------------------------------------------------------------------//
  struct ScopedLoggingStream : public Assimp::LogStream
  {
    explicit ScopedLoggingStream(uint aSeverityMask) { Assimp::DefaultLogger::get()->attachStream(this, aSeverityMask); }
    ~ScopedLoggingStream() { Assimp::DefaultLogger::get()->detatchStream(this); }
    void write(const char* message) override { C_LOG_INFO("SceneImporter: %s", message); }
  };
  
  glm::mat4 matFromAiMat(const aiMatrix4x4& mat)
  {
    return glm::mat4(mat.a1, mat.a2, mat.a3, mat.a4,
      mat.b1, mat.b2, mat.b3, mat.b4,
      mat.c1, mat.c2, mat.c3, mat.c4,
      mat.d1, mat.d2, mat.d3, mat.d4);
  }

  uint GetAiImportOptions(ImportOptions someImportOptions)
  {
    uint aiOptions = 0u;
    if (someImportOptions & CALC_TANGENT_SPACE)
      aiOptions |= aiProcess_CalcTangentSpace;
    if (someImportOptions & TRIANGULATE)
      aiOptions |= aiProcess_Triangulate;
    if (someImportOptions & JOIN_IDENTICAL_VERTICES)
      aiOptions |= aiProcess_JoinIdenticalVertices;
    if (someImportOptions & SPLIT_BY_PRIMITIVE_TYPE)
      aiOptions |= aiProcess_SortByPType;
    if (someImportOptions & INSTANTIATE_DUPLICATES)
      aiOptions |= aiProcess_FindInstances;
    return aiOptions;
  }

  uint64 ComputeHashFromVertexData(aiMesh** someMeshes, uint aMeshCount)
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
  
  struct ProcessData
  {
    ProcessData() 
      : mySourcePath("") 
      , myScene(nullptr)
      , myNumCreatedMeshes(0u)
      , myNumCreatedModels(0u)
      , myNumCreatedGeometryDatas(0u)
      , myNumCreatedSubModels(0u) 
    {}

    const char* mySourcePath;
    const aiScene* myScene;
    uint myNumCreatedMeshes;
    uint myNumCreatedModels;
    uint myNumCreatedGeometryDatas;
    uint myNumCreatedSubModels;
    std::unordered_map<const aiMaterial*, SharedPtr<Material>> myMaterialCache;
    std::unordered_map<uint, SharedPtr<Mesh>> myMeshCache;
  };

  SharedPtr<Mesh> CreateMesh(const aiNode* aNode, ProcessData& aProcessData, aiMesh** someMeshes, uint aMeshCount)
  {
    // String meshName = GetCachePathForMesh();
    // if (BinaryCache::read(&outputMesh, meshName, 0u))
    // return outputMesh;

    // Mesh already created during this import-process?
    uint64 hash = 0u;
    for (uint i = 0u; i < aMeshCount; ++i)
      MathUtil::hash_combine(hash, reinterpret_cast<uint64>(someMeshes[i]));

    auto it = aProcessData.myMeshCache.find(hash);
    if (it != aProcessData.myMeshCache.end())
      return it->second;

    // Mesh already created in the renderer?
    uint64 vertexIndexHash = ComputeHashFromVertexData(someMeshes, aMeshCount);
    SharedPtr<Mesh> mesh = RenderCore::GetMesh(vertexIndexHash);
    if (mesh != nullptr)
      return mesh;

    // We don't have the mesh in any cache and have to create it.
    mesh.reset(FANCY_NEW(Mesh, MemoryCategory::GEOMETRY));

    
    MeshDesc meshDesc;
    meshDesc.myVertexAndIndexHash = vertexIndexHash;

    DynamicArray<void*> vertexDatas;
    DynamicArray<uint> numVertices;
    DynamicArray<void*> indexDatas;
    DynamicArray<uint> numIndices;
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
      FixedArray<ImportVertexStream, Constants::kMaxNumGeometryVertexAttributes> importStreams;

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

  bool ProcessMeshes(const aiNode* aNode, ProcessData& aProcessData, LoadResult& aResultOut)
  {
    if (aNode->mNumMeshes == 0)
      return true;

    std::unordered_map<uint, std::unordered_set<aiMesh*>> materialMeshMap;
    for (uint i = 0u; i < aNode->mNumMeshes; ++i)
    {
      aiMesh* mesh = aProcessData.myScene->mMeshes[aNode->mMeshes[i]];
      materialMeshMap[mesh->mMaterialIndex].insert(mesh);
    }

    // Construct or retrieve Fancy Meshes and Submodels
    // Each mesh-list with the same material becomes a submodel
    DynamicArray<SharedPtr<SubModel>> submodels;
    for (auto it = materialMeshMap.begin(); it != materialMeshMap.end(); ++it)
    {
      const uint materialIndex = it->first;
      auto& materialMeshSet = it->second;

      SharedPtr<Geometry::Mesh> pMesh =
        constructOrRetrieveMesh(_pAnode, &vAssimpMeshes[0], vAssimpMeshes.size());

      // Create or retrieve the material
      aiMaterial* pAmaterial =
        myWorkingData.pCurrScene->mMaterials[materialIndex];
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



  }

  bool ProcessNodeRecursive(const aiNode* aNode, ProcessData& aProcessData, LoadResult& aResultOut)
  {
    if (!aNode) 
      return false;

    if (!ProcessMeshes(aNode, aProcessData, aResultOut))
      return false;

    for (uint i = 0u; i < aNode->mNumChildren; ++i)
      if (!ProcessNodeRecursive(aNode->mChildren[i], aProcessData, aResultOut))
        return false;

    return true;
  }

  bool LoadFromFile(const char* aPath, LoadResult& aResultOut, ImportOptions someImportOptions/* = ALL*/)
  {
    ScopedLoggingStream loggingStream(Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Err | Assimp::Logger::Warn);
    
    Assimp::Importer importer;
    const aiScene* importedScene = importer.ReadFile(aPath, GetAiImportOptions(someImportOptions));

    if (!importedScene)
      return false;

    ProcessData data;
    data.myScene = importedScene;
    data.mySourcePath = aPath;
    return ProcessNodeRecursive(importedScene->mRootNode, data, aResultOut);
  }
//---------------------------------------------------------------------------//
} }


