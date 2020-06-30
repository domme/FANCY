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

#include "MathUtil.h"
#include "RenderCore.h"
#include "Mesh.h"
#include "VertexInputLayoutProperties.h"
#include "PathService.h"
#include "RendererPrerequisites.h"
#include "StaticString.h"
#include "StaticArray.h"
#include "Assets.h"

#include "Material.h"

namespace Fancy 
{
  namespace Priv_MeshImporter
  {
  //---------------------------------------------------------------------------//
    struct ScopedLoggingStream : public Assimp::LogStream
    {
      explicit ScopedLoggingStream(uint aSeverityMask) { Assimp::DefaultLogger::get()->attachStream(this, aSeverityMask); }
      ~ScopedLoggingStream() { Assimp::DefaultLogger::get()->detatchStream(this); }
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
    SharedPtr<VertexInputLayout> CreateVertexInputLayout(const StaticArray<VertexShaderAttributeDesc, 16>& someVertexAttributes)
    {
      VertexInputLayoutProperties layoutProps;

      uint overallVertexSize = 0u;
      for (uint i = 0u; i < someVertexAttributes.Size(); ++i)
      {
        const VertexShaderAttributeDesc& expectedAttribute = someVertexAttributes[i];
        overallVertexSize += DataFormatInfo::GetFormatInfo(expectedAttribute.myFormat).mySizeBytes;
        layoutProps.myAttributes.Add({ expectedAttribute.myFormat, expectedAttribute.mySemantic, expectedAttribute.mySemanticIndex, 0u });
      }

      VertexBufferBindDesc bufferBindDesc;
      bufferBindDesc.myInputRate = VertexInputRate::PER_VERTEX;
      bufferBindDesc.myStride = overallVertexSize;
      layoutProps.myBufferBindings.Add(bufferBindDesc);

      return RenderCore::CreateVertexInputLayout(layoutProps);
    }
  //---------------------------------------------------------------------------//
    String BuildTexturePath(const aiMaterial* anAiMaterial, uint anAiTextureType, uint aTexIndex, const String& aSceneSourcePath)
    {
      uint numTextures = anAiMaterial->GetTextureCount(static_cast<aiTextureType>(anAiTextureType));
      if (numTextures == 0u)
        return "";

      ASSERT(numTextures > aTexIndex);

      aiString szATexPath;
      anAiMaterial->Get(AI_MATKEY_TEXTURE(anAiTextureType, aTexIndex), szATexPath);

      String texPathAbs = String(szATexPath.C_Str());
      Path::ConvertToSlash(texPathAbs);

      if (!Path::IsPathAbsolute(texPathAbs))
      {
        String relativePath = Path::GetContainingFolder(aSceneSourcePath) + "/" + texPathAbs;
        Path::RemoveFolderUpMarkers(relativePath);
        texPathAbs = Path::GetAbsoluteResourcePath(relativePath);
      }

      Path::RemoveFolderUpMarkers(texPathAbs);
      const String& texPathInResources = Path::GetRelativeResourcePath(texPathAbs);

      return texPathInResources;
    }
  }
//---------------------------------------------------------------------------//
  bool MeshImporter::Import(const char* aPath, const StaticArray<VertexShaderAttributeDesc, 16>& someVertexAttributes, ImportResult& aResultOut, ImportOptions someImportOptions)
  {
    Priv_MeshImporter::ScopedLoggingStream loggingStream(Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Err | Assimp::Logger::Warn);

    String pathAbs = Path::GetAbsoluteResourcePath(aPath);

    Assimp::Importer importer;
    const aiScene* importedScene = importer.ReadFile(pathAbs.c_str(), Priv_MeshImporter::GetAiImportOptions(someImportOptions));

    if (!importedScene)
      return false;

    myScene = importedScene;
    mySourcePath = aPath;
    mySceneFileTimeStamp = Path::GetFileWriteTime(pathAbs);
    myVertexAttributes = someVertexAttributes;
    myVertexInputLayout = Priv_MeshImporter::CreateVertexInputLayout(someVertexAttributes);
    aResultOut.myVertexInputLayout = myVertexInputLayout;

    aiNode* rootNode = importedScene->mRootNode;
    bool success = ProcessNodeRecursive(rootNode, Priv_MeshImporter::MatFromAiMat(rootNode->mTransformation), aResultOut);

    myScene = nullptr;
    mySourcePath.clear();
    mySceneFileTimeStamp = 0ull;
    myVertexAttributes.Clear();
    myVertexInputLayout.reset();

    return success;
  }
//---------------------------------------------------------------------------//
  bool MeshImporter::ProcessNodeRecursive(const aiNode* aNode, const glm::float4x4& aParentTransform, ImportResult& aResultOut)
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
  bool MeshImporter::ProcessMeshes(const aiNode* aNode, const glm::float4x4& aTransform, ImportResult& aResultOut)
  {
    if (aNode->mNumMeshes == 0)
      return true;

    std::unordered_map<uint, DynamicArray<aiMesh*>> materialMeshMap;
    for (uint i = 0u; i < aNode->mNumMeshes; ++i)
    {
      aiMesh* mesh = myScene->mMeshes[aNode->mMeshes[i]];

      DynamicArray<aiMesh*>& meshList = materialMeshMap[mesh->mMaterialIndex];
      if (std::find(meshList.begin(), meshList.end(), mesh) == meshList.end())
        meshList.push_back(mesh);
    }
    
    bool hasValidMeshes = false;
    for (auto it = materialMeshMap.begin(); it != materialMeshMap.end(); ++it)
    {
      DynamicArray<aiMesh*>& meshList = it->second;
      const uint materialIndex = it->first;

      SharedPtr<Mesh> mesh = CreateMesh(aNode, meshList.data(), (uint)meshList.size());

      aiMaterial* aiMaterial = myScene->mMaterials[materialIndex];
      SharedPtr<Material> material = CreateMaterial(aiMaterial);

      if (!mesh || !material)
        continue;

      hasValidMeshes = true;
      
      aResultOut.myMaterials.push_back(material);
      aResultOut.myMeshes.push_back(mesh);
      aResultOut.myTransforms.push_back(aTransform);
    }

    return hasValidMeshes;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> MeshImporter::CreateMesh(const aiNode* aNode, aiMesh** someMeshes, uint aMeshCount)
  {
    // Mesh already created during this import-process?
    uint64 assimpMeshListHash = 0u;
    for (uint i = 0u; i < aMeshCount; ++i)
      MathUtil::hash_combine(assimpMeshListHash, reinterpret_cast<uint64>(someMeshes[i]));

    auto it = myMeshCache.find(assimpMeshListHash);
    if (it != myMeshCache.end())
      return it->second;
 
    DynamicArray<MeshPartData> meshPartDatas;
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

      const StaticArray<VertexShaderAttributeDesc, 16>& expectedAttributes = myVertexAttributes;
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
      MeshPartData partData;
      partData.myVertexLayoutProperties = myVertexInputLayout->myProperties;
      partData.myVertexData.resize(overallVertexSize * aiMesh->mNumVertices);

      uint8* dstPtr = partData.myVertexData.data();
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
    
    MeshDesc desc = CreateMeshDesc(assimpMeshListHash);
    SharedPtr<Mesh> mesh = Assets::CreateMesh(desc, meshPartDatas.data(), (uint) meshPartDatas.size());
    ASSERT(mesh != nullptr);
    
    myMeshCache[assimpMeshListHash] = mesh;
    return mesh;
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
  SharedPtr<Material> MeshImporter::CreateMaterial(const aiMaterial* anAiMaterial)
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

    const String& diffuseTexPath = Priv_MeshImporter::BuildTexturePath(anAiMaterial, aiTextureType_DIFFUSE, 0u, mySourcePath);
    const String& normalTexPath = Priv_MeshImporter::BuildTexturePath(anAiMaterial, aiTextureType_NORMALS, 0u, mySourcePath);
    const String& specularTexPath = Priv_MeshImporter::BuildTexturePath(anAiMaterial, aiTextureType_SPECULAR, 0u, mySourcePath);
    const String& specPowerTexPath = Priv_MeshImporter::BuildTexturePath(anAiMaterial, aiTextureType_SHININESS, 0u, mySourcePath);
    const String& opacityTexPath = Priv_MeshImporter::BuildTexturePath(anAiMaterial, aiTextureType_OPACITY, 0u, mySourcePath);

    MaterialDesc matDesc;
    matDesc.myTextures[(uint)MaterialTextureType::BASE_COLOR] = diffuseTexPath;
    matDesc.myTextures[(uint)MaterialTextureType::NORMAL] = normalTexPath;
    matDesc.myTextures[(uint)MaterialTextureType::MATERIAL] = specPowerTexPath;
    matDesc.myParameters[(uint)MaterialParameterType::DIFFUSE_REFLECTIVITY] = glm::float4((color_diffuse.r + color_diffuse.g + color_diffuse.b) * (1.0f / 3.0f));
    matDesc.myParameters[(uint)MaterialParameterType::SPECULAR_REFLECTIVITY] = glm::float4(specular);
    matDesc.myParameters[(uint)MaterialParameterType::SPECULAR_POWER] = glm::float4(specularPower);
    matDesc.myParameters[(uint)MaterialParameterType::OPACITY] = glm::float4(opacity);

    return Assets::CreateMaterial(matDesc);
  }
//---------------------------------------------------------------------------//
}
