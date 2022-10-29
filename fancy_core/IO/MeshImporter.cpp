#include "fancy_core_precompile.h"
#include "MeshImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>

#include <unordered_map>
#include <unordered_set>

#include "Common/MathUtil.h"
#include "Rendering/RenderCore.h"
#include "Mesh.h"
#include "Rendering/VertexInputLayoutProperties.h"
#include "PathService.h"
#include "Rendering/RendererPrerequisites.h"
#include "Common/StaticString.h"
#include "Material.h"
#include "BinaryCache.h"
#include "Common/CommandLine.h"
#include "Rendering/ShaderPipeline.h"

namespace Fancy 
{
  namespace Priv_MeshImporter
  {
  //---------------------------------------------------------------------------//
    struct ScopedLoggingStream : public Assimp::LogStream
    {
      explicit ScopedLoggingStream(uint aSeverityMask) { Assimp::DefaultLogger::get()->attachStream(this, aSeverityMask); }
      ~ScopedLoggingStream() { Assimp::DefaultLogger::get()->detachStream(this); }
      void write(const char* message) override { LOG_INFO("SceneImporter: %s", message); }
    };
  //---------------------------------------------------------------------------//
    glm::float4x4 MatFromAiMat(const aiMatrix4x4& mat)
    {
      return glm::float4x4(mat.a1, mat.a2, mat.a3, mat.a4,
        mat.b1, mat.b2, mat.b3, mat.b4,
        mat.c1, mat.c2, mat.c3, mat.c4,
        mat.d1, mat.d2, mat.d3, mat.d4);
    }
  //---------------------------------------------------------------------------//
    uint GetAiImportOptions(MeshImporter::ImportOptions someImportOptions)
    {
      uint aiOptions = 0u;
      if (someImportOptions & MeshImporter::CALC_TANGENT_SPACE)
        aiOptions |= aiProcess_CalcTangentSpace;
      if (someImportOptions & MeshImporter::TRIANGULATE)
        aiOptions |= aiProcess_Triangulate;
      if (someImportOptions & MeshImporter::JOIN_IDENTICAL_VERTICES)
        aiOptions |= aiProcess_JoinIdenticalVertices;
      if (someImportOptions & MeshImporter::SPLIT_BY_PRIMITIVE_TYPE)
        aiOptions |= aiProcess_SortByPType;
      if (someImportOptions & MeshImporter::INSTANTIATE_DUPLICATES)
        aiOptions |= aiProcess_FindInstances;
      return aiOptions;
    }
  //---------------------------------------------------------------------------//
    VertexInputLayoutProperties CreateVertexInputLayout(const eastl::fixed_vector<VertexShaderAttributeDesc, 16>& someVertexAttributes)
    {
      VertexInputLayoutProperties layoutProps;

      uint overallVertexSize = 0u;
      for (const VertexShaderAttributeDesc& expectedAttribute : someVertexAttributes)
      {
        overallVertexSize += DataFormatInfo::GetFormatInfo(expectedAttribute.myFormat).mySizeBytes;
        layoutProps.myAttributes.push_back({ expectedAttribute.myFormat, expectedAttribute.mySemantic, expectedAttribute.mySemanticIndex, 0u });
      }

      VertexBufferBindDesc bufferBindDesc;
      bufferBindDesc.myInputRate = VertexInputRate::PER_VERTEX;
      bufferBindDesc.myStride = overallVertexSize;
      layoutProps.myBufferBindings.push_back(bufferBindDesc);

      return layoutProps;
    }
  //---------------------------------------------------------------------------//
    eastl::string BuildTexturePath(const aiMaterial* anAiMaterial, uint anAiTextureType, uint aTexIndex, const eastl::string& aSceneSourcePath)
    {
      uint numTextures = anAiMaterial->GetTextureCount(static_cast<aiTextureType>(anAiTextureType));
      if (numTextures == 0u)
        return "";

      ASSERT(numTextures > aTexIndex);

      aiString szATexPath;
      anAiMaterial->Get(AI_MATKEY_TEXTURE(anAiTextureType, aTexIndex), szATexPath);

      eastl::string texPathAbs = eastl::string(szATexPath.C_Str());
      Path::ConvertToSlash(texPathAbs);

      if (!Path::IsPathAbsolute(texPathAbs))
      {
        eastl::string relativePath = Path::GetContainingFolder(aSceneSourcePath) + "/" + texPathAbs;
        Path::RemoveFolderUpMarkers(relativePath);
        texPathAbs = Path::GetAbsolutePath(relativePath);
      }

      Path::RemoveFolderUpMarkers(texPathAbs);
      const eastl::string& texPathInResources = Path::GetRelativePath(texPathAbs);

      return texPathInResources;
    }
  }
//---------------------------------------------------------------------------//
  bool MeshImporter::Import(const char* aPath, const ShaderPipeline* aShaderPipeline, SceneData& aResultOut, ImportOptions someImportOptions)
  {
    const Shader* vertexShader = aShaderPipeline->GetShader(SHADERSTAGE_VERTEX);
    ASSERT(vertexShader != nullptr, "Vertex shader needed during mesh import to get the required vertex attributes");
    return Import(aPath, vertexShader->myVertexAttributes, aResultOut, someImportOptions);
  }
//---------------------------------------------------------------------------//
  bool MeshImporter::Import(const char* aPath, const eastl::fixed_vector<VertexShaderAttributeDesc, 16>& someVertexAttributes, SceneData& aResultOut, ImportOptions someImportOptions)
  {
#if FANCY_USE_BINARY_CACHE
      if (!CommandLine::GetInstance()->HasArgument("nobinarycache") && BinaryCache::ReadScene(aPath, aResultOut))
        return true;
#endif

    Priv_MeshImporter::ScopedLoggingStream loggingStream(Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Err | Assimp::Logger::Warn);

    eastl::string pathAbs = Path::GetAbsolutePath(aPath);

    Assimp::Importer importer;
    const aiScene* importedScene = importer.ReadFile(pathAbs.c_str(), Priv_MeshImporter::GetAiImportOptions(someImportOptions));

    if (!importedScene)
      return false;

    myScene = importedScene;
    mySourcePath = aPath;
    mySceneFileTimeStamp = Path::GetFileWriteTime(pathAbs);
    myVertexAttributes = someVertexAttributes;
    myVertexInputLayout = Priv_MeshImporter::CreateVertexInputLayout(someVertexAttributes);
    aResultOut.myVertexInputLayoutProperties = myVertexInputLayout;

    aiNode* rootNode = importedScene->mRootNode;
    bool success = ProcessNodeRecursive(rootNode, Priv_MeshImporter::MatFromAiMat(rootNode->mTransformation), aResultOut);

#if FANCY_USE_BINARY_CACHE
    if (success && !CommandLine::GetInstance()->HasArgument("nobinarycache"))
      BinaryCache::WriteScene(aPath, aResultOut);
#endif

    myScene = nullptr;
    mySourcePath.clear();
    mySceneFileTimeStamp = 0ull;
    myVertexAttributes.clear();
    myVertexInputLayout = VertexInputLayoutProperties();

    return success;
  }
//---------------------------------------------------------------------------//
  bool MeshImporter::ProcessNodeRecursive(const aiNode* aNode, const glm::float4x4& aParentTransform, SceneData& aResultOut)
  {
    if (!aNode)
      return false;

    glm::float4x4 transform = Priv_MeshImporter::MatFromAiMat(aNode->mTransformation) * aParentTransform;

    if (!ProcessMeshes(aNode, transform, aResultOut))
      return false;

    for (uint i = 0u; i < aNode->mNumChildren; ++i)
      if (!ProcessNodeRecursive(aNode->mChildren[i], transform, aResultOut))
        return false;

    return true;
  }
//---------------------------------------------------------------------------//
  bool MeshImporter::ProcessMeshes(const aiNode* aNode, const glm::float4x4& aTransform, SceneData& aResultOut)
  {
    if (aNode->mNumMeshes == 0)
      return true;

    std::unordered_map<uint, eastl::fixed_vector<aiMesh*, 32>> materialMeshMap;
    for (uint i = 0u; i < aNode->mNumMeshes; ++i)
    {
      aiMesh* mesh = myScene->mMeshes[aNode->mMeshes[i]];

      eastl::fixed_vector<aiMesh*, 32>& meshList = materialMeshMap[mesh->mMaterialIndex];
      if (std::find(meshList.begin(), meshList.end(), mesh) == meshList.end())
        meshList.push_back(mesh);
    }
    
    for (auto it = materialMeshMap.begin(); it != materialMeshMap.end(); ++it)
    {
      eastl::fixed_vector<aiMesh*, 32>& meshList = it->second;
      const uint aiMatIndex = it->first;

      aiMaterial* aiMaterial = myScene->mMaterials[aiMatIndex];

      SceneMeshInstance meshInstance;
      meshInstance.myMeshIndex = CreateMesh(meshList.data(), (uint)meshList.size(), aResultOut);
      meshInstance.myMaterialIndex = CreateMaterial(aiMaterial, aResultOut);
      meshInstance.myTransform = aTransform;
      aResultOut.myInstances.push_back(meshInstance);
    }

    return true;
  }
//---------------------------------------------------------------------------//
  uint MeshImporter::CreateMesh(aiMesh** someMeshes, uint aMeshCount, SceneData& aResultOut)
  {
    // Mesh already created during this import-process?
    uint64 assimpMeshListHash = 0u;
    for (uint i = 0u; i < aMeshCount; ++i)
      MathUtil::hash_combine(assimpMeshListHash, reinterpret_cast<uint64>(someMeshes[i]));

    auto it = myMeshCache.find(assimpMeshListHash);
    if (it != myMeshCache.end())
      return it->second;

    MeshData meshData;
    eastl::vector<MeshPartData>& meshPartDatas = meshData.myParts;
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
      eastl::vector<ImportVertexStream> importStreams;

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

      const eastl::fixed_vector<VertexShaderAttributeDesc, 16>& expectedAttributes = myVertexAttributes;
      uint attributeSizes[16] = { 0u };
      int importStreamForAttribute[16];
      for (uint i = 0u; i < 16; ++i)
        importStreamForAttribute[i] = -1;
      uint overallVertexSize = 0u;
      for (uint i = 0u; i < expectedAttributes.size(); ++i)
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
            if (stream.myDataSize > expectedSize)
              LOG_WARNING("Vertex attribute size for semantic %d in model is larger than the expected size. Only the expected size will be copied per vertex: Expected %d bytes - has %d bytes",
              (uint)expectedAttribute.mySemantic, expectedSize, stream.myDataSize);

            importStreamForAttribute[i] = (int) k;
            break;
          }
        }
      }

      // Create an interleaved vertex buffer
      MeshPartData partData;
      partData.myVertexLayoutProperties = myVertexInputLayout;
      partData.myVertexData.resize(overallVertexSize * aiMesh->mNumVertices);

      uint8* dstPtr = partData.myVertexData.data();
      for (uint v = 0u; v < aiMesh->mNumVertices; ++v)
      {
        for (uint i = 0u; i < expectedAttributes.size(); ++i)
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

      /// Construct the index buffer
#if defined (FANCY_IMPORTER_USE_VALIDATION)
      // Ensure that we have only triangles
      for (uint i = 0u; i < aiMesh->mNumFaces; ++i)
      {
        ASSERT(aiMesh->mFaces[i].mNumIndices == 3u, "Unsupported face type");
      }
#endif  // FANCY_IMPORTER_USE_VALIDATION

      partData.myIndexData.resize(sizeof(uint) * aiMesh->mNumFaces * 3u);
      uint* indices = (uint*)partData.myIndexData.data();

      for (uint i = 0u; i < aiMesh->mNumFaces; ++i)
      {
        const aiFace& aFace = aiMesh->mFaces[i];
        memcpy(&indices[i * 3u], aFace.mIndices, sizeof(uint) * 3u);
      }

      meshPartDatas.push_back(partData);
    }
    
    meshData.myDesc = CreateMeshDesc(assimpMeshListHash);
    aResultOut.myMeshes.push_back(std::move(meshData));

    uint index = (uint) aResultOut.myMeshes.size() - 1;
    myMeshCache[assimpMeshListHash] = index;
    return index;
  }
//---------------------------------------------------------------------------//
  MeshDesc MeshImporter::CreateMeshDesc(uint64 anAssimpMeshListHash)
  {
    MeshDesc desc;
    desc.myName = StaticString<260>("%s_Mesh_%d", mySourcePath.c_str(), anAssimpMeshListHash);
    desc.myHash = MathUtil::Hash(desc.myName);
    return desc;
  }
//---------------------------------------------------------------------------//
  uint MeshImporter::CreateMaterial(const aiMaterial* anAiMaterial, SceneData& aResultOut)
  {
    // Did we already import this material?
    {
      const auto cacheIt = myMaterialCache.find(anAiMaterial);
      if (cacheIt != myMaterialCache.end())
        return cacheIt->second;
    }

    // Retrieve the material properties most relevant for us
    aiString szAname;
    const bool hasName = anAiMaterial->Get(AI_MATKEY_NAME, szAname) == AI_SUCCESS;

    aiColor3D color_diffuse;
    const bool hasColor = anAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color_diffuse) == AI_SUCCESS;

    aiColor3D color_specular;
    const bool hasSpecularColor = anAiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color_specular) == AI_SUCCESS;

    aiColor3D color_ambient;
    const bool hasAmbientColor = anAiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color_ambient) == AI_SUCCESS;

    aiColor3D color_emissive;
    const bool hasEmissiveColor = anAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color_emissive) == AI_SUCCESS;

    aiColor3D color_transparent;
    const bool hasTransparentColor = anAiMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, color_transparent) == AI_SUCCESS;

    aiBlendMode blend_func;
    const bool hasBlendFunc = anAiMaterial->Get(AI_MATKEY_BLEND_FUNC, blend_func) == AI_SUCCESS;

    float opacity;
    const bool hasOpacity = anAiMaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS;

    float specularPower;
    const bool hasSpecularPower = anAiMaterial->Get(AI_MATKEY_SHININESS, specularPower) == AI_SUCCESS;

    float specular;
    const bool hasSpecular = anAiMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, specular) == AI_SUCCESS;

    const eastl::string& diffuseTexPath = Priv_MeshImporter::BuildTexturePath(anAiMaterial, aiTextureType_DIFFUSE, 0u, mySourcePath);
    const eastl::string& normalTexPath = Priv_MeshImporter::BuildTexturePath(anAiMaterial, aiTextureType_NORMALS, 0u, mySourcePath);
    // const eastl::string& specularTexPath = Priv_MeshImporter::BuildTexturePath(anAiMaterial, aiTextureType_SPECULAR, 0u, mySourcePath);
    const eastl::string& specPowerTexPath = Priv_MeshImporter::BuildTexturePath(anAiMaterial, aiTextureType_SHININESS, 0u, mySourcePath);
    // const eastl::string& opacityTexPath = Priv_MeshImporter::BuildTexturePath(anAiMaterial, aiTextureType_OPACITY, 0u, mySourcePath);

    MaterialDesc matDesc;
    matDesc.myTextures[(uint)MaterialTextureType::BASE_COLOR] = diffuseTexPath;
    matDesc.myTextures[(uint)MaterialTextureType::NORMAL] = normalTexPath;
    matDesc.myTextures[(uint)MaterialTextureType::MATERIAL] = specPowerTexPath;
    matDesc.myParameters[(uint)MaterialParameterType::COLOR] = glm::float4(color_diffuse.r, color_diffuse.g, color_diffuse.b, 1.0f);
    matDesc.myParameters[(uint)MaterialParameterType::SPECULAR_REFLECTIVITY] = glm::float4(specular);
    matDesc.myParameters[(uint)MaterialParameterType::SPECULAR_POWER] = glm::float4(specularPower);
    matDesc.myParameters[(uint)MaterialParameterType::OPACITY] = glm::float4(opacity);

    aResultOut.myMaterials.push_back(std::move(matDesc));
    uint index = (uint) aResultOut.myMaterials.size() - 1;

    myMaterialCache[anAiMaterial] = index;
    return index;
  }
//---------------------------------------------------------------------------//
}
