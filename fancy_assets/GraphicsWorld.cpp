#include "GraphicsWorld.h"
#include "Material.h"
#include "TextureLoader.h"
#include "MaterialDesc.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/PathService.h"
#include "ModelDesc.h"
#include "Model.h"
#include "fancy_core/BinaryCache.h"

using namespace Fancy;

//---------------------------------------------------------------------------//
  GraphicsWorld::GraphicsWorld()
  {
  }
//---------------------------------------------------------------------------//
  GraphicsWorld::~GraphicsWorld()
  {
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> GraphicsWorld::GetTexture(uint64 aDescHash)
  {
    auto it = std::find(myTextures.begin(), myTextures.end(), aDescHash);
    if (it != myTextures.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Material> GraphicsWorld::CreateMaterial(const MaterialDesc& aDesc)
  {
    const uint64 descHash = aDesc.GetHash();

    auto it = std::find(myMaterials.begin(), myMaterials.end(), descHash);
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
  SharedPtr<Texture> GraphicsWorld::CreateTexture(const TextureDesc& aDesc)
  {
    if (!aDesc.myIsExternalTexture)
      return RenderCore::GetInternalTexture(aDesc);

    return CreateTexture(aDesc.mySourcePath.c_str());
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> GraphicsWorld::CreateTexture(const char* aPath)
  {
    String texPathAbs = aPath;
    String texPathRel = aPath;
    if (!Path::IsPathAbsolute(texPathAbs))
      texPathAbs = Resources::FindPath(texPathAbs);
    else
      texPathRel = Resources::FindName(texPathAbs);

    TextureDesc desc;
    desc.mySourcePath = texPathAbs;
    desc.myIsExternalTexture = true;
    uint64 hash = desc.GetHash();

    if (SharedPtr<Texture> texFromMemCache = GetTexture(hash))
      return texFromMemCache;
    
    if (SharedPtr<Texture> texFromDiskCache = BinaryCache::ReadTexture(hash, 0u))
    {
      myTextures[hash] = texFromDiskCache;
      return texFromDiskCache;
    }

    std::vector<uint8> textureBytes;
    TextureLoadInfo textureInfo;
    if (!TextureLoader::Load(texPathAbs, textureBytes, textureInfo))
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
  SharedPtr<Model> GraphicsWorld::CreateModel(const ModelDesc& aDesc)
  {
    uint64 hash = aDesc.GetHash();

    auto it = std::find(myModels.begin(), myModels.end(), hash);
    if (it != myModels.end())
      return it->second;

    SharedPtr<Model> model(new Model);
    model->myMaterial = CreateMaterial(aDesc.myMaterial);
    model->myMesh = GetMesh(aDesc.myMesh.myVertexAndIndexHash);
    
    myModels[hash] = model;
    return model;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> GraphicsWorld::GetMesh(uint64 aDescHash)
  {
    auto it = std::find(myMeshes.begin(), myMeshes.end(), aDescHash);
    if (it != myMeshes.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> GraphicsWorld::CreateMesh(const MeshDesc& aDesc, const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas, const std::vector<uint>& someNumVertices, const std::vector<uint>& someNumIndices)
  {
    if (!aDesc.myIsExternalMesh)
      return RenderCore::GetInternalMesh(aDesc);

    SharedPtr<Mesh> meshFromDiskCache = BinaryCache::ReadMesh(aDesc.GetHash(), 0u);
    if (meshFromDiskCache)
    {
      myMeshes[aDesc.GetHash()] = meshFromDiskCache;
      return meshFromDiskCache;
    }

    SharedPtr<Mesh> mesh = RenderCore::CreateMesh(aDesc, someVertexDatas, someIndexDatas, someNumVertices, someNumIndices);
    if (mesh)
    {
      BinaryCache::WriteMesh(mesh.get(), someVertexDatas, someIndexDatas);

      myMeshes[aDesc.GetHash()] = mesh;
      return mesh;
    }


  }
//---------------------------------------------------------------------------//
