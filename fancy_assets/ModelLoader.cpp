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

#include "fancy_core/MathUtil.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/Mesh.h"
#include "fancy_core/VertexInputLayout.h"
#include <fancy_core/PathService.h>

#include "Material.h"
#include "AssetStorage.h"
#include "fancy_core/MeshData.h"
#include "ModelDesc.h"
#include "fancy_core/RenderPlatformCaps.h"

namespace Fancy { namespace ModelLoader {
//---------------------------------------------------------------------------//
  struct ScopedLoggingStream : public Assimp::LogStream
  {
    explicit ScopedLoggingStream(uint aSeverityMask) { Assimp::DefaultLogger::get()->attachStream(this, aSeverityMask); }
    ~ScopedLoggingStream() { Assimp::DefaultLogger::get()->detatchStream(this); }
    void write(const char* message) override { C_LOG_INFO("SceneImporter: %s", message); }
  };
//---------------------------------------------------------------------------//
  glm::mat4 matFromAiMat(const aiMatrix4x4& mat)
  {
    return glm::mat4(mat.a1, mat.a2, mat.a3, mat.a4,
      mat.b1, mat.b2, mat.b3, mat.b4,
      mat.c1, mat.c2, mat.c3, mat.c4,
      mat.d1, mat.d2, mat.d3, mat.d4);
  }
//---------------------------------------------------------------------------//
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
//---------------------------------------------------------------------------//
  struct ProcessData
  {
    ProcessData() 
      : mySourcePath("") 
      , myScene(nullptr)
      , mySceneFileTimeStamp(0u)
    {}

    const char* mySourcePath;
    const aiScene* myScene;
    uint mySceneFileTimeStamp;
    std::unordered_map<const aiMaterial*, SharedPtr<Material>> myMaterialCache;
    std::unordered_map<uint64, SharedPtr<Mesh>> myMeshCache;
  };
//---------------------------------------------------------------------------//
  String CreateUniqueMeshName(uint64 anAssimpMeshListHash, ProcessData& aProcessData)
  {
    String name;
    name.Format("%_Mesh_%", aProcessData.mySourcePath, anAssimpMeshListHash);
    return name;
  }

  SharedPtr<Mesh> CreateMesh(const aiNode* aNode, ProcessData& aProcessData, AssetStorage& aStorage, aiMesh** someMeshes, uint aMeshCount)
  {
    // Mesh already created during this import-process?
    uint64 assimpMeshListHash = 0u;
    for (uint i = 0u; i < aMeshCount; ++i)
      MathUtil::hash_combine(assimpMeshListHash, reinterpret_cast<uint64>(someMeshes[i]));

    auto it = aProcessData.myMeshCache.find(assimpMeshListHash);
    if (it != aProcessData.myMeshCache.end())
      return it->second;
 
    DynamicArray<MeshData> meshDatas;
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
      DynamicArray<ImportVertexStream> importStreams;

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

      ASSERT(importStreams.size() <= RenderCore::GetPlatformCaps().myMaxNumVertexAttributes);

      const ShaderVertexInputLayout* expectedLayout = &ShaderVertexInputLayout::ourDefaultModelLayout;
      const DynamicArray<ShaderVertexInputElement>& expectedInputList = expectedLayout->myVertexInputElements;

      // Check if we need additional patching-streams when the model-shaders expect more data than this model has

      DynamicArray<ImportVertexStream> actualImportStreams;
      actualImportStreams.resize(expectedInputList.size());

      DynamicArray<void*> patchingDatas;

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
      GeometryVertexLayout vertexLayout;

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

      MeshData meshData;
      meshData.myLayout = vertexLayout;
      meshData.myVertexData.resize(vertexLayout.myStride * aiMesh->mNumVertices);

      // Construct an interleaved vertex array
      for (uint iVertex = 0u; iVertex < aiMesh->mNumVertices; ++iVertex)
      {
        for (uint iVertexElement = 0u; iVertexElement < vertexLayout.myElements.size(); ++iVertexElement)
        {
          const GeometryVertexElement& vertexElem = vertexLayout.myElements[iVertexElement];
          uint destInterleavedOffset = iVertex * vertexLayout.myStride + vertexElem.u32OffsetBytes;
          uint srcOffset = iVertex * actualImportStreams[iVertexElement].mySourceDataStride;

          uint8* dest = (meshData.myVertexData.data()) + destInterleavedOffset;
          uint8* src = ((uint8*)actualImportStreams[iVertexElement].mySourceData) + srcOffset;

          memcpy(dest, src, vertexElem.u32SizeBytes);
        }
      }

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

      meshData.myIndexData.resize(sizeof(uint) * aiMesh->mNumFaces * 3u);
      uint* indices = (uint*)meshData.myIndexData.data();

      for (uint i = 0u; i < aiMesh->mNumFaces; ++i)
      {
        const aiFace& aFace = aiMesh->mFaces[i];
        memcpy(&indices[i * 3u], aFace.mIndices, sizeof(uint) * 3u);
      }

      meshDatas.push_back(meshData);
    }

    MeshDesc meshDesc;
    meshDesc.myIsExternalMesh = true;
    meshDesc.myUniqueName = CreateUniqueMeshName(assimpMeshListHash, aProcessData);
    
    SharedPtr<Mesh> mesh = aStorage.CreateMesh(meshDesc, meshDatas.data(), meshDatas.size(), aProcessData.mySceneFileTimeStamp);
    ASSERT(mesh != nullptr);
    
    aProcessData.myMeshCache[assimpMeshListHash] = mesh;
    return mesh;
  }
//---------------------------------------------------------------------------//
  TextureDesc CreateTextureDesc(const aiMaterial* _pAmaterial, uint _aiTextureType, uint _texIndex, ProcessData& aProcessData)
  {
    uint numTextures = _pAmaterial->GetTextureCount(static_cast<aiTextureType>(_aiTextureType));
    if (numTextures == 0u)
      return TextureDesc();

    ASSERT(numTextures > _texIndex);

    aiString szATexPath;
    _pAmaterial->Get(AI_MATKEY_TEXTURE(_aiTextureType, _texIndex), szATexPath);

    String texPathAbs = String(szATexPath.C_Str());
    Path::UnifySlashes(texPathAbs);

    if (!Path::IsPathAbsolute(texPathAbs))
    {
      String relativePath = Path::GetContainingFolder(aProcessData.mySourcePath) + "/" + texPathAbs;
      Path::RemoveFolderUpMarkers(relativePath);
      texPathAbs = Resources::FindPath(relativePath);
    }

    Path::RemoveFolderUpMarkers(texPathAbs);
    String texPathInResources = Resources::FindName(texPathAbs);

    TextureDesc desc;
    desc.mySourcePath = texPathInResources;
    desc.myInternalRefIndex = ~0;
    desc.myIsExternalTexture = true;
    return desc;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Material> CreateMaterial(const aiMaterial* _pAmaterial, ProcessData& aProcessData, AssetStorage& aStorage)
  {
    // Did we already import this material?
    {
      const auto cacheIt = aProcessData.myMaterialCache.find(_pAmaterial);
      if (cacheIt != aProcessData.myMaterialCache.end())
        return cacheIt->second;
    }
    
    // Retrieve the material properties most relevant for us
    aiString szAname;
    const bool hasName = _pAmaterial->Get(AI_MATKEY_NAME, szAname) == AI_SUCCESS;

    aiColor3D color_diffuse;
    const bool hasColor = _pAmaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color_diffuse) == AI_SUCCESS;

    aiColor3D color_specular;
    const bool hasSpecularColor = _pAmaterial->Get(AI_MATKEY_COLOR_SPECULAR, color_specular) == AI_SUCCESS;

    aiColor3D color_ambient;
    const bool hasAmbientColor = _pAmaterial->Get(AI_MATKEY_COLOR_AMBIENT, color_ambient) == AI_SUCCESS;

    aiColor3D color_emissive;
    const bool hasEmissiveColor = _pAmaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color_emissive) == AI_SUCCESS;

    aiColor3D color_transparent;
    const bool hasTransparentColor = _pAmaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, color_transparent) == AI_SUCCESS;

    aiBlendMode blend_func;
    const bool hasBlendFunc = _pAmaterial->Get(AI_MATKEY_BLEND_FUNC, blend_func) == AI_SUCCESS;

    float opacity;
    const bool hasOpacity = _pAmaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS;

    float specularPower;
    const bool hasSpecularPower = _pAmaterial->Get(AI_MATKEY_SHININESS, specularPower) == AI_SUCCESS;

    float specular;
    const bool hasSpecular = _pAmaterial->Get(AI_MATKEY_SHININESS_STRENGTH, specular) == AI_SUCCESS;

    const TextureDesc& diffuseTexDesc = CreateTextureDesc(_pAmaterial, aiTextureType_DIFFUSE, 0u, aProcessData);
    const TextureDesc& normalTexDesc = CreateTextureDesc(_pAmaterial, aiTextureType_NORMALS, 0u, aProcessData);
    const TextureDesc& specularTexDesc = CreateTextureDesc(_pAmaterial, aiTextureType_SPECULAR, 0u, aProcessData);
    const TextureDesc& specPowerTexDesc = CreateTextureDesc(_pAmaterial, aiTextureType_SHININESS, 0u, aProcessData);
    const TextureDesc& opacityTexDesc = CreateTextureDesc(_pAmaterial, aiTextureType_OPACITY, 0u, aProcessData);

    MaterialDesc matDesc;
    matDesc.mySemanticTextures[(uint)TextureSemantic::BASE_COLOR] = diffuseTexDesc;
    matDesc.mySemanticTextures[(uint)TextureSemantic::NORMAL] = normalTexDesc;
    matDesc.mySemanticTextures[(uint)TextureSemantic::MATERIAL] = specPowerTexDesc;
    matDesc.mySemanticParameters[(uint)ParameterSemantic::DIFFUSE_REFLECTIVITY] = (color_diffuse.r + color_diffuse.g + color_diffuse.b) * (1.0f / 3.0f);
    matDesc.mySemanticParameters[(uint)ParameterSemantic::SPECULAR_REFLECTIVITY] = specular;
    matDesc.mySemanticParameters[(uint)ParameterSemantic::SPECULAR_POWER] = specularPower;
    matDesc.mySemanticParameters[(uint)ParameterSemantic::OPACITY] = opacity;

    return aStorage.CreateMaterial(matDesc);
  }
//---------------------------------------------------------------------------//
  bool ProcessMeshes(const aiNode* aNode, const glm::mat4& aTransform, ProcessData& aProcessData, AssetStorage& aStorage, Scene& aSceneOut)
  {
    if (aNode->mNumMeshes == 0)
      return true;

    std::unordered_map<uint, DynamicArray<aiMesh*>> materialMeshMap;
    for (uint i = 0u; i < aNode->mNumMeshes; ++i)
    {
      aiMesh* mesh = aProcessData.myScene->mMeshes[aNode->mMeshes[i]];

      DynamicArray<aiMesh*>& meshList = materialMeshMap[mesh->mMaterialIndex];
      if (std::find(meshList.begin(), meshList.end(), mesh) == meshList.end())
        meshList.push_back(mesh);
    }

    // Construct or retrieve Fancy Meshes and models
    // Each mesh-list with the same material becomes a model
    uint numCreatedModels = 0u;
    for (auto it = materialMeshMap.begin(); it != materialMeshMap.end(); ++it)
    {
      DynamicArray<aiMesh*>& meshList = it->second;
      const uint materialIndex = it->first;

      SharedPtr<Mesh> mesh = CreateMesh(aNode, aProcessData, aStorage, &meshList[0], meshList.size());

      aiMaterial* pAmaterial = aProcessData.myScene->mMaterials[materialIndex];
      SharedPtr<Material> material = CreateMaterial(pAmaterial, aProcessData, aStorage);

      if (!mesh || !material)
        continue;

      ModelDesc modelDesc;
      modelDesc.myMaterial = material->GetDescription();
      modelDesc.myMesh = mesh->myDesc;
      SharedPtr<Model> model = aStorage.CreateModel(modelDesc);

      if (model)
      {
        ++numCreatedModels;
        aSceneOut.myModels.push_back(model);
        aSceneOut.myTransforms.push_back(aTransform);
      }
    }

    return numCreatedModels > 0u;
  }

  bool ProcessNodeRecursive(const aiNode* aNode, const glm::mat4& aParentTransform, ProcessData& aProcessData, AssetStorage& aStorage, Scene& aSceneOut)
  {
    if (!aNode) 
      return false;

    glm::mat4 transform = matFromAiMat(aNode->mTransformation) * aParentTransform;

    if (!ProcessMeshes(aNode, transform, aProcessData, aStorage, aSceneOut))
      return false;

    for (uint i = 0u; i < aNode->mNumChildren; ++i)
      if (!ProcessNodeRecursive(aNode->mChildren[i], transform, aProcessData, aStorage, aSceneOut))
        return false;

    return true;
  }

  bool LoadFromFile(const char* aPath, AssetStorage& aStorage, Scene& aSceneOut, ImportOptions someImportOptions/* = ALL*/)
  {
    ScopedLoggingStream loggingStream(Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Err | Assimp::Logger::Warn);

    String pathAbs = Resources::FindPath(aPath);
    
    Assimp::Importer importer;
    const aiScene* importedScene = importer.ReadFile(pathAbs.c_str(), GetAiImportOptions(someImportOptions));

    if (!importedScene)
      return false;

    ProcessData data;
    data.myScene = importedScene;
    data.mySourcePath = aPath;
    data.mySceneFileTimeStamp = Path::GetFileWriteTime(pathAbs.c_str());

    aiNode* rootNode = importedScene->mRootNode;
    return ProcessNodeRecursive(rootNode, matFromAiMat(rootNode->mTransformation), data, aStorage, aSceneOut);
  }
//---------------------------------------------------------------------------//
} }


