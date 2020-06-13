#include "fancy_assets_precompile.h"
#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>

#include <unordered_map>
#include <unordered_set>

#include <fancy_core/MathUtil.h>
#include <fancy_core/RenderCore.h>
#include <fancy_core/Mesh.h>
#include <fancy_core/VertexInputLayoutProperties.h>
#include <fancy_core/PathService.h>
#include <fancy_core/RendererPrerequisites.h>
#include <fancy_core/MeshData.h>

#include "Material.h"
#include "AssetManager.h"

#include "ModelDesc.h"
#include "fancy_core/StaticString.h"
#include "fancy_core/StaticArray.h"

namespace Fancy { namespace ModelLoader {
//---------------------------------------------------------------------------//
  struct ScopedLoggingStream : public Assimp::LogStream
  {
    explicit ScopedLoggingStream(uint aSeverityMask) { Assimp::DefaultLogger::get()->attachStream(this, aSeverityMask); }
    ~ScopedLoggingStream() { Assimp::DefaultLogger::get()->detatchStream(this); }
    void write(const char* message) override { LOG_INFO("SceneImporter: %s", message); }
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
    uint64 mySceneFileTimeStamp;
    StaticArray<VertexShaderAttributeDesc, 16> myVertexAttributes;
    std::unordered_map<const aiMaterial*, SharedPtr<Material>> myMaterialCache;
    std::unordered_map<uint64, SharedPtr<Mesh>> myMeshCache;
  };
//---------------------------------------------------------------------------//
  String CreateUniqueMeshName(uint64 anAssimpMeshListHash, ProcessData& aProcessData)
  {
    String name(StaticString<260>("%s_Mesh_%d", aProcessData.mySourcePath, anAssimpMeshListHash));
    return name;
  }

  SharedPtr<Mesh> CreateMesh(const aiNode* aNode, ProcessData& aProcessData, AssetManager& aStorage, aiMesh** someMeshes, uint aMeshCount)
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
        const uint8* mySourceData;
        uint mySourceDataStride;
        uint myDataSize;
        VertexAttributeSemantic mySourceSemantic;
        uint mySourceSemanticIndex;
        uint myReadOffset = 0u;
      };
      DynamicArray<ImportVertexStream> importStreams;

      ASSERT(aiMesh->HasPositions());
      {
        ImportVertexStream stream;
        stream.mySourceDataStride = sizeof(aiMesh->mVertices[0]);
        stream.myDataSize = sizeof(aiMesh->mVertices[0]);
        stream.mySourceData = reinterpret_cast<const uint8*>(aiMesh->mVertices);
        stream.mySourceSemantic = VertexAttributeSemantic::POSITION;
        stream.mySourceSemanticIndex = 0u;
        importStreams.push_back(stream);
      }

      if (aiMesh->HasNormals())
      {
        ImportVertexStream stream;
        stream.mySourceDataStride = sizeof(aiMesh->mNormals[0]);
        stream.myDataSize = sizeof(aiMesh->mNormals[0]);
        stream.mySourceData = reinterpret_cast<const uint8*>(aiMesh->mNormals);
        stream.mySourceSemantic = VertexAttributeSemantic::NORMAL;
        stream.mySourceSemanticIndex = 0u;
        importStreams.push_back(stream);
      }

      if (aiMesh->HasTangentsAndBitangents())
      {
        {
          ImportVertexStream stream;
          stream.mySourceDataStride = sizeof(aiMesh->mTangents[0]);
          stream.myDataSize = sizeof(aiMesh->mTangents[0]);
          stream.mySourceData = reinterpret_cast<const uint8*>(aiMesh->mTangents);
          stream.mySourceSemantic = VertexAttributeSemantic::TANGENT;
          stream.mySourceSemanticIndex = 0u;
          importStreams.push_back(stream);
        }
        {
          ImportVertexStream stream;
          stream.mySourceDataStride = sizeof(aiMesh->mBitangents[0]);
          stream.myDataSize = sizeof(aiMesh->mBitangents[0]);
          stream.mySourceData = reinterpret_cast<const uint8*>(aiMesh->mBitangents);
          stream.mySourceSemantic = VertexAttributeSemantic::BINORMAL;
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
          stream.myDataSize = sizeof(aiMesh->mTextureCoords[iUVchannel][0].x) * aiMesh->mNumUVComponents[iUVchannel];
          stream.mySourceData = reinterpret_cast<const uint8*>(aiMesh->mTextureCoords[iUVchannel]);
          stream.mySourceSemantic = VertexAttributeSemantic::TEXCOORD;
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
          stream.mySourceData = reinterpret_cast<const uint8*>(aiMesh->mColors[iColorChannel]);
          stream.mySourceSemantic = VertexAttributeSemantic::COLOR;
          stream.mySourceSemanticIndex = iColorChannel;
          importStreams.push_back(stream);
        }
      }

      ASSERT(importStreams.size() <= RenderCore::GetPlatformCaps().myMaxNumVertexAttributes);

      const StaticArray<VertexShaderAttributeDesc, 16>& expectedAttributes = aProcessData.myVertexAttributes;
      uint attributeSizes[16] = { 0u };
      int importStreamForAttribute[16];
      for (uint i = 0u; i < 16; ++i)
        importStreamForAttribute[i] = -1;
      uint overallVertexSize = 0u;
      for (uint i = 0u; i < expectedAttributes.Size(); ++i)
      {
        const VertexShaderAttributeDesc& expectedAttribute = expectedAttributes[i];
        uint expectedSize = DataFormatInfo::GetFormatInfo(expectedAttribute.myFormat).mySizeBytes;
        attributeSizes[i] = expectedSize;
        overallVertexSize += expectedSize;
        for (uint k = 0u; k < importStreams.size(); ++k)
        {
          const ImportVertexStream& stream = importStreams[k];
          if (stream.mySourceSemantic == expectedAttribute.mySemantic && stream.mySourceSemanticIndex == expectedAttribute.mySemanticIndex)
          {
            ASSERT(stream.myDataSize == expectedSize, "Mismatching vertex attribute size for semantic %d: Expected %d bytes - has %d bytes",
              (uint) expectedAttribute.mySemantic, expectedSize, stream.myDataSize);
            importStreamForAttribute[i] = (int) k;
            break;
          }
        }
      }

      // Create an interleaved vertex buffer
      MeshData meshData;
      meshData.myVertexData.resize(overallVertexSize * aiMesh->mNumVertices);

      uint8* dstPtr = meshData.myVertexData.data();
      for (uint v = 0u; v < aiMesh->mNumVertices; ++v)
      {
        for (uint i = 0u; i < expectedAttributes.Size(); ++i)
        {
          if (importStreamForAttribute[i] != -1)
          {
            ImportVertexStream& stream = importStreams[importStreamForAttribute[i]];
            ASSERT(attributeSizes[i] <= stream.myDataSize);
            memcpy(dstPtr, stream.mySourceData + stream.myReadOffset, attributeSizes[i]);
            stream.myReadOffset += stream.mySourceDataStride;
          }
          else
          {
            memset(dstPtr, 0u, attributeSizes[i]);
          }

          dstPtr += attributeSizes[i];
        }
      }

      VertexBufferBindDesc bufferBindDesc;
      bufferBindDesc.myInputRate = VertexInputRate::PER_VERTEX;
      bufferBindDesc.myStride = overallVertexSize;
      meshData.myVertexLayout.myBufferBindings.Add(bufferBindDesc);

      for (uint i = 0u; i < expectedAttributes.Size(); ++i)
      {
        const VertexShaderAttributeDesc& expectedAttribute = expectedAttributes[i];
        meshData.myVertexLayout.myAttributes.Add({ expectedAttribute.myFormat, expectedAttribute.mySemantic, expectedAttribute.mySemanticIndex, 0u });
      }

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
    
    SharedPtr<Mesh> mesh = aStorage.CreateMesh(meshDesc, meshDatas.data(), (uint) meshDatas.size(), aProcessData.mySceneFileTimeStamp);
    ASSERT(mesh != nullptr);
    
    aProcessData.myMeshCache[assimpMeshListHash] = mesh;
    return mesh;
  }
//---------------------------------------------------------------------------//
  String BuildTexturePath(const aiMaterial* _pAmaterial, uint _aiTextureType, uint _texIndex, ProcessData& aProcessData)
  {
    uint numTextures = _pAmaterial->GetTextureCount(static_cast<aiTextureType>(_aiTextureType));
    if (numTextures == 0u)
      return "";

    ASSERT(numTextures > _texIndex);

    aiString szATexPath;
    _pAmaterial->Get(AI_MATKEY_TEXTURE(_aiTextureType, _texIndex), szATexPath);

    String texPathAbs = String(szATexPath.C_Str());
    Path::ConvertToSlash(texPathAbs);

    if (!Path::IsPathAbsolute(texPathAbs))
    {
      String relativePath = Path::GetContainingFolder(aProcessData.mySourcePath) + "/" + texPathAbs;
      Path::RemoveFolderUpMarkers(relativePath);
      texPathAbs = Path::GetAbsoluteResourcePath(relativePath);
    }

    Path::RemoveFolderUpMarkers(texPathAbs);
    const String& texPathInResources = Path::GetRelativeResourcePath(texPathAbs);
    
    return texPathInResources;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Material> CreateMaterial(const aiMaterial* _pAmaterial, ProcessData& aProcessData, AssetManager& aStorage)
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

    const String& diffuseTexPath = BuildTexturePath(_pAmaterial, aiTextureType_DIFFUSE, 0u, aProcessData);
    const String& normalTexPath = BuildTexturePath(_pAmaterial, aiTextureType_NORMALS, 0u, aProcessData);
    const String& specularTexPath = BuildTexturePath(_pAmaterial, aiTextureType_SPECULAR, 0u, aProcessData);
    const String& specPowerTexPath = BuildTexturePath(_pAmaterial, aiTextureType_SHININESS, 0u, aProcessData);
    const String& opacityTexPath = BuildTexturePath(_pAmaterial, aiTextureType_OPACITY, 0u, aProcessData);

    MaterialDesc matDesc;
    matDesc.mySemanticTextures[(uint)TextureSemantic::BASE_COLOR] = diffuseTexPath;
    matDesc.mySemanticTextures[(uint)TextureSemantic::NORMAL] = normalTexPath;
    matDesc.mySemanticTextures[(uint)TextureSemantic::MATERIAL] = specPowerTexPath;
    matDesc.mySemanticParameters[(uint)ParameterSemantic::DIFFUSE_REFLECTIVITY] = (color_diffuse.r + color_diffuse.g + color_diffuse.b) * (1.0f / 3.0f);
    matDesc.mySemanticParameters[(uint)ParameterSemantic::SPECULAR_REFLECTIVITY] = specular;
    matDesc.mySemanticParameters[(uint)ParameterSemantic::SPECULAR_POWER] = specularPower;
    matDesc.mySemanticParameters[(uint)ParameterSemantic::OPACITY] = opacity;

    return aStorage.CreateMaterial(matDesc);
  }
//---------------------------------------------------------------------------//
  bool ProcessMeshes(const aiNode* aNode, const glm::mat4& aTransform, ProcessData& aProcessData, AssetManager& aStorage, Scene& aSceneOut)
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

      SharedPtr<Mesh> mesh = CreateMesh(aNode, aProcessData, aStorage, &meshList[0], (uint) meshList.size());

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

  bool ProcessNodeRecursive(const aiNode* aNode, const glm::mat4& aParentTransform, ProcessData& aProcessData, AssetManager& aStorage, Scene& aSceneOut)
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

  bool LoadFromFile(const char* aPath, const StaticArray<VertexShaderAttributeDesc, 16>& someVertexAttributes, AssetManager& aStorage, Scene& aSceneOut, ImportOptions someImportOptions/* = ALL*/)
  {
    ScopedLoggingStream loggingStream(Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Err | Assimp::Logger::Warn);

    String pathAbs = Path::GetAbsoluteResourcePath(aPath);
    
    Assimp::Importer importer;
    const aiScene* importedScene = importer.ReadFile(pathAbs.c_str(), GetAiImportOptions(someImportOptions));

    if (!importedScene)
      return false;

    ProcessData data;
    data.myScene = importedScene;
    data.mySourcePath = aPath;
    data.mySceneFileTimeStamp = Path::GetFileWriteTime(pathAbs.c_str());
    data.myVertexAttributes = someVertexAttributes;

    aiNode* rootNode = importedScene->mRootNode;
    return ProcessNodeRecursive(rootNode, matFromAiMat(rootNode->mTransformation), data, aStorage, aSceneOut);
  }
//---------------------------------------------------------------------------//
} }


