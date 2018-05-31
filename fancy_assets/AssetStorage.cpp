#include "AssetStorage.h"
#include "Material.h"
#include "TextureLoader.h"
#include "MaterialDesc.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/PathService.h"
#include "ModelDesc.h"
#include "Model.h"
#include "fancy_core/BinaryCache.h"
#include <fancy_core/Mesh.h>

using namespace Fancy;

//---------------------------------------------------------------------------//
  AssetStorage::AssetStorage()
  {
  }
//---------------------------------------------------------------------------//
  AssetStorage::~AssetStorage()
  {
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> AssetStorage::GetTexture(const TextureDesc& aDesc)
  {
    auto it = myTextures.find(aDesc.GetHash());
    if (it != myTextures.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Material> AssetStorage::CreateMaterial(const MaterialDesc& aDesc)
  {
    const uint64 descHash = aDesc.GetHash();

    auto it = myMaterials.find(descHash);
    if (it != myMaterials.end())
      return it->second;

    SharedPtr<Material> mat(new Material);
    for (uint i = 0u; i < aDesc.mySemanticTextures.size(); ++i)
      mat->mySemanticTextures[i] = CreateTexture(aDesc.mySemanticTextures[i]);

    for (const TextureDesc& texDesc : aDesc.myExtraTextures)
      mat->myExtraTextures.push_back(CreateTexture(texDesc));

    for (uint i = 0u; i < aDesc.mySemanticParameters.size(); ++i)
      mat->mySemanticParameters[i] = aDesc.mySemanticParameters[i];

    for (float param : aDesc.myExtraParameters)
      mat->myExtraParameters.push_back(param);
    
    myMaterials[descHash] = mat;
    return mat;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> AssetStorage::CreateTexture(const TextureDesc& aDesc)
  {
    return CreateTexture(aDesc.mySourcePath.c_str());
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> AssetStorage::CreateTexture(const char* aPath)
  {
    if (strlen(aPath) == 0)
      return nullptr;

    String texPathAbs = aPath;
    String texPathRel = aPath;
    if (!Path::IsPathAbsolute(texPathAbs))
      texPathAbs = Resources::FindPath(texPathAbs);
    else
      texPathRel = Resources::FindName(texPathAbs);

    TextureDesc desc;
    desc.mySourcePath = texPathRel;
    desc.myIsExternalTexture = true;
    uint64 hash = desc.GetHash();

    if (SharedPtr<Texture> texFromMemCache = GetTexture(desc))
      return texFromMemCache;

    uint64 timestamp = Path::GetFileWriteTime(texPathAbs);
    if (SharedPtr<Texture> texFromDiskCache = BinaryCache::ReadTexture(desc, timestamp))
    {
      myTextures[hash] = texFromDiskCache;
      return texFromDiskCache;
    }

    std::vector<uint8> textureBytes;
    TextureLoadInfo textureInfo;
    if (!TextureLoader::Load(texPathAbs.c_str(), textureBytes, textureInfo))
    {
      LOG_ERROR("Failed to load texture at path %", texPathAbs);
      return nullptr;
    }

    if (textureInfo.bitsPerPixel / textureInfo.numChannels != 8u)
    {
      LOG_ERROR("Unsupported texture format: %", texPathAbs);
      return nullptr;
    }

    TextureParams texParams;
    texParams.myIsExternalTexture = true;
    texParams.path = texPathRel;
    texParams.bIsDepthStencil = false;
    texParams.eFormat = textureInfo.numChannels == 3u ? DataFormat::SRGB_8 : DataFormat::SRGB_8_A_8;
    texParams.u16Width = textureInfo.width;
    texParams.u16Height = textureInfo.height;
    texParams.u16Depth = 0u;
    texParams.uAccessFlags = (uint)GpuResourceAccessFlags::NONE;

    TextureUploadData uploadData;
    uploadData.myData = &textureBytes[0];
    uploadData.myPixelSizeBytes = textureInfo.bitsPerPixel / 8u;
    uploadData.myRowSizeBytes = textureInfo.width * uploadData.myPixelSizeBytes;
    uploadData.mySliceSizeBytes = textureInfo.width * textureInfo.height * uploadData.myPixelSizeBytes;
    uploadData.myTotalSizeBytes = uploadData.mySliceSizeBytes;

    SharedPtr<Texture> tex = RenderCore::CreateTexture(texParams, &uploadData, 1u);

    if (tex != nullptr)
    {
      BinaryCache::WriteTexture(tex.get(), uploadData);

      myTextures[hash] = tex;
      return tex;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Model> AssetStorage::CreateModel(const ModelDesc& aDesc)
  {
    const uint64 hash = aDesc.GetHash();

    auto it = myModels.find(hash);
    if (it != myModels.end())
      return it->second;

    SharedPtr<Model> model(new Model);
    model->myMaterial = CreateMaterial(aDesc.myMaterial);
    model->myMesh = GetMesh(aDesc.myMesh);
    
    myModels[hash] = model;
    return model;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> AssetStorage::GetMesh(const MeshDesc& aDesc)
  {
    auto it = myMeshes.find(aDesc.GetHash());
    if (it != myMeshes.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> AssetStorage::CreateMesh(const MeshDesc& aDesc, MeshData* someMeshDatas, uint aNumMeshDatas, uint64 aMeshFileTimestamp /* = 0u */)
  {
    if (aMeshFileTimestamp != 0u)
    {
      SharedPtr<Mesh> meshFromDiskCache = BinaryCache::ReadMesh(aDesc, aMeshFileTimestamp);
      if (meshFromDiskCache)
      {
        myMeshes[meshFromDiskCache->myVertexAndIndexHash] = meshFromDiskCache;
        return meshFromDiskCache;
      }
    }
    
    SharedPtr<Mesh> mesh = RenderCore::CreateMesh(aDesc, someMeshDatas, aNumMeshDatas);
    if (mesh)
    {
      BinaryCache::WriteMesh(mesh.get(), someMeshDatas, aNumMeshDatas);

      myMeshes[aDesc.GetHash()] = mesh;
      return mesh;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//