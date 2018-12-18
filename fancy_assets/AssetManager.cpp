#include "AssetManager.h"
#include "Material.h"
#include "TextureLoader.h"
#include "MaterialDesc.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/PathService.h"
#include "ModelDesc.h"
#include "Model.h"
#include "fancy_core/BinaryCache.h"
#include <fancy_core/Mesh.h>
#include "fancy_core/Texture.h"
#include "fancy_core/CommandContext.h"

using namespace Fancy;

//---------------------------------------------------------------------------//
  AssetManager::AssetManager()
  {
    GpuProgramDesc shaderDesc;
    shaderDesc.myShaderFileName = "ResizeTexture2D";
    shaderDesc.myShaderStage = (uint)ShaderStage::COMPUTE;
    shaderDesc.myMainFunction = "main";
    myTextureResizeShader = RenderCore::CreateGpuProgram(shaderDesc);
    ASSERT(myTextureResizeShader != nullptr);
  }
//---------------------------------------------------------------------------//
  void AssetManager::Clear()
  {
    myMaterials.clear();
    myModels.clear();
    myTextures.clear();
    myMeshes.clear();
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> AssetManager::GetTexture(const char* aPath, uint someFlags /* = 0*/)
  {
    String texPathAbs = aPath;
    String texPathRel = aPath;
    if (!Path::IsPathAbsolute(texPathAbs))
      texPathAbs = Resources::FindPath(texPathAbs);
    else
      texPathRel = Resources::FindName(texPathAbs);

    uint64 texPathRelHash = MathUtil::Hash(texPathRel);
    MathUtil::hash_combine(texPathRelHash, ((uint64) someFlags & SHADER_WRITABLE));

    auto it = myTextures.find(texPathRelHash);
    if (it != myTextures.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Material> AssetManager::CreateMaterial(const MaterialDesc& aDesc)
  {
    const uint64 descHash = aDesc.GetHash();

    auto it = myMaterials.find(descHash);
    if (it != myMaterials.end())
      return it->second;

    TextureViewProperties srvViewProps;
    srvViewProps.myDimension = GpuResourceDimension::TEXTURE_2D;

    SharedPtr<Material> mat(new Material);
    for (uint i = 0u; i < aDesc.mySemanticTextures.size(); ++i)
    {
      SharedPtr<Texture> tex = CreateTexture(aDesc.mySemanticTextures[i].c_str());
      if (tex != nullptr)
        mat->mySemanticTextures[i] = RenderCore::CreateTextureView(tex, srvViewProps);
    }
    
    for (const String& texPath : aDesc.myExtraTextures)
    {
      SharedPtr<Texture> tex = CreateTexture(texPath.c_str());
      if (tex != nullptr)
        mat->myExtraTextures.push_back(RenderCore::CreateTextureView(tex, srvViewProps));
    }

    for (uint i = 0u; i < aDesc.mySemanticParameters.size(); ++i)
      mat->mySemanticParameters[i] = aDesc.mySemanticParameters[i];

    for (float param : aDesc.myExtraParameters)
      mat->myExtraParameters.push_back(param);
    
    myMaterials[descHash] = mat;
    return mat;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> AssetManager::CreateTexture(const char* aPath, uint someLoadFlags/* = 0*/)
  {
    if (strlen(aPath) == 0)
      return nullptr;

    String texPathAbs = aPath;
    String texPathRel = aPath;
    if (!Path::IsPathAbsolute(texPathAbs))
      texPathAbs = Resources::FindPath(texPathAbs);
    else
      texPathRel = Resources::FindName(texPathAbs);

    if ((someLoadFlags & NO_MEM_CACHE) == 0)
      if (SharedPtr<Texture> texFromMemCache = GetTexture(texPathRel.c_str()))
        return texFromMemCache;

    uint64 texPathRelHash = MathUtil::Hash(texPathRel);
    MathUtil::hash_combine(texPathRelHash, ((uint64) someLoadFlags & SHADER_WRITABLE));

    if ((someLoadFlags & NO_DISK_CACHE) == 0)
    {
      uint64 timestamp = Path::GetFileWriteTime(texPathAbs);
      if (SharedPtr<Texture> texFromDiskCache = BinaryCache::ReadTexture(texPathRel.c_str(), timestamp))
      {
        myTextures[texPathRelHash] = texFromDiskCache;
        return texFromDiskCache;
      }  
    }

    std::vector<uint8> textureBytes;
    TextureLoadInfo texLoadInfo;
    if (!TextureLoader::Load(texPathAbs.c_str(), textureBytes, texLoadInfo))
    {
      LOG_ERROR("Failed to load texture at path %", texPathAbs);
      return nullptr;
    }

    TextureProperties texProps;
    texProps.myDimension = GpuResourceDimension::TEXTURE_2D;
    texProps.path = texPathRel;
    texProps.bIsDepthStencil = false;
    texProps.myWidth = texLoadInfo.width;
    texProps.myHeight = texLoadInfo.height;
    texProps.myDepthOrArraySize = 0u;
    texProps.myAccessType = CpuMemoryAccessType::NO_CPU_ACCESS;
    texProps.myIsShaderWritable = (someLoadFlags & SHADER_WRITABLE) != 0;

    if (!(texLoadInfo.bitsPerChannel == 8u || texLoadInfo.bitsPerChannel == 16u))
    {
      LOG_ERROR("Unsupported bits per channel in texture % (has % bits per channel)", texPathAbs, texLoadInfo.bitsPerChannel);
      return nullptr;
    }

    switch(texLoadInfo.numChannels)
    {
      case 1: texProps.eFormat = texLoadInfo.bitsPerChannel == 8 ? DataFormat::R_8 : DataFormat::R_16; break;
      case 2: texProps.eFormat = texLoadInfo.bitsPerChannel == 8 ? DataFormat::RG_8 : DataFormat::RG_16; break;

      // LodePNG assumes that 8-bit RGB or RGBA data is always SRGB. The official "sRGB" info-chunk from the PNG format is not supported
      case 3: texProps.eFormat = texLoadInfo.bitsPerChannel == 8 ? DataFormat::SRGB_8 : DataFormat::RGB_16; break;
      case 4: texProps.eFormat = texLoadInfo.bitsPerChannel == 8 ? DataFormat::SRGB_8_A_8 : DataFormat::RGBA_16; break;
      default: ASSERT(false, "Unsupported channels");
      return nullptr;
    }

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(texProps.eFormat);
    const uint expectedDataSize = formatInfo.mySizeBytes * texProps.myWidth * texProps.myHeight;
    if (expectedDataSize != textureBytes.size())
    {
      LOG_ERROR("Invalid pixel data loaded from texture %. Expected: %, Actual: %", texPathAbs, expectedDataSize, textureBytes.size());
      return nullptr;
    }
    
    TextureSubData dataFirstMip;
    dataFirstMip.myData = textureBytes.data();
    dataFirstMip.myPixelSizeBytes = (texLoadInfo.bitsPerChannel * texLoadInfo.numChannels) / 8u;
    dataFirstMip.myRowSizeBytes = texLoadInfo.width * dataFirstMip.myPixelSizeBytes;
    dataFirstMip.mySliceSizeBytes = texLoadInfo.width * texLoadInfo.height * dataFirstMip.myPixelSizeBytes;
    dataFirstMip.myTotalSizeBytes = dataFirstMip.mySliceSizeBytes;
    SharedPtr<Texture> tex = RenderCore::CreateTexture(texProps, texPathRel.c_str(), &dataFirstMip, 1u);

    if (tex != nullptr)
    {
      ComputeMipmaps(tex);

      DynamicArray<uint8> mipmapPixelData;
      DynamicArray<TextureSubData> mipmapSubDatas;
      RenderCore::ReadbackTextureData(tex.get(), TextureSubLocation(1), tex->GetNumSubresources() - 1, mipmapPixelData, mipmapSubDatas);

      mipmapSubDatas.insert(mipmapSubDatas.begin(), dataFirstMip);
      BinaryCache::WriteTextureData(tex.get(), mipmapSubDatas.data(), mipmapSubDatas.size());

      myTextures[texPathRelHash] = tex;
      return tex;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Model> AssetManager::CreateModel(const ModelDesc& aDesc)
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
  SharedPtr<Mesh> AssetManager::GetMesh(const MeshDesc& aDesc)
  {
    auto it = myMeshes.find(aDesc.GetHash());
    if (it != myMeshes.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> AssetManager::CreateMesh(const MeshDesc& aDesc, MeshData* someMeshDatas, uint aNumMeshDatas, uint64 aMeshFileTimestamp /* = 0u */)
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
  void AssetManager::ComputeMipmaps(const SharedPtr<Texture>& aTexture)
  {
    const TextureProperties& texProps = aTexture->GetProperties();
    const uint numMips = texProps.myNumMipLevels;
    glm::float2 srcSize(texProps.myWidth, texProps.myHeight);
    glm::int2 tempTexSize = (glm::int2) glm::float2(glm::ceil(srcSize.x * 0.5), srcSize.y);

    TempTextureResource tempTexResource[2];
    TextureResourceProperties tempTexProps;
    tempTexProps.myTextureProperties = texProps;
    tempTexProps.myTextureProperties.myNumMipLevels = 1;
    tempTexProps.myTextureProperties.myWidth = (uint)tempTexSize.x;
    tempTexProps.myTextureProperties.myHeight = (uint)tempTexSize.y;
    tempTexProps.myIsShaderWritable = true;
    tempTexProps.myIsRenderTarget = false;
    tempTexProps.myIsTexture = true;
    tempTexResource[0] = RenderCore::AllocateTempTexture(tempTexProps, TempResourcePool::FORCE_SIZE, "Mipmapping temp texture 0");
    tempTexResource[1] = RenderCore::AllocateTempTexture(tempTexProps, TempResourcePool::FORCE_SIZE, "Mipmapping temp texture 1");

    CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
    CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
    ctx->SetComputeProgram(myTextureResizeShader.get());

    struct CBuffer
    {
      glm::float2 mySrcSize;
      glm::float2 myDestSize;

      glm::float2 mySrcScale;
      glm::float2 myDestScale;

      int myIsSRGB;
      int myFilterMethod;
      glm::float2 myAxis;
    } cBuffer;

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(texProps.eFormat);
    
    cBuffer.myIsSRGB = formatInfo.mySRGB ? 1 : 0;
    cBuffer.myFilterMethod = 1; // Lanczos filter

    const uint kMaxNumMips = 17;
    ASSERT(numMips <= kMaxNumMips);
    FixedArray<SharedPtr<TextureView>, kMaxNumMips> readViews;

    TextureViewProperties readProps;
    readProps.myNumMipLevels = 1;
    for (uint mip = 0u; mip < numMips - 1; ++mip)
    {
      readProps.myMipIndex = mip;
      readViews[mip] = RenderCore::CreateTextureView(aTexture, readProps);
    }

    glm::float2 destSize = glm::ceil(srcSize * 0.5f);
    for (uint mip = 1u; mip < numMips; ++mip)
    {
      const GpuResourceView* resources[] = { nullptr, nullptr };

      // Resize horizontal
      glm::float2 tempDestSize(destSize.x, srcSize.y);
      cBuffer.mySrcSize = srcSize;
      cBuffer.myDestSize = tempDestSize;
      cBuffer.mySrcScale = tempDestSize / srcSize;
      cBuffer.myDestScale = srcSize / tempDestSize;
      cBuffer.myAxis = glm::float2(1.0f, 0.0f);
      ctx->BindConstantBuffer(&cBuffer, sizeof(cBuffer), 0u);
      resources[0] = readViews[mip - 1].get();
      resources[1] = tempTexResource[0].myWriteView;
      ctx->BindResourceSet(resources, 2, 1u);
      ctx->Dispatch(glm::int3((int)destSize.x, (int)srcSize.y, 1));

      // Resize vertical
      cBuffer.mySrcSize = tempDestSize;
      cBuffer.myDestSize = destSize;
      cBuffer.mySrcScale = destSize / tempDestSize;
      cBuffer.myDestScale = tempDestSize / destSize;
      cBuffer.myAxis = glm::float2(0.0f, 1.0f);
      ctx->BindConstantBuffer(&cBuffer, sizeof(cBuffer), 0u);
      resources[0] = tempTexResource[0].myReadView;
      resources[1] = tempTexResource[1].myWriteView;
      ctx->BindResourceSet(resources, 2, 1u);
      ctx->Dispatch(glm::int3((int)destSize.x, (int)destSize.y, 1));

      TextureSubLocation destLocation;
      destLocation.myMipLevel = mip;

      TextureRegion srcRegion;
      srcRegion.myTexelPos = glm::uvec3(0, 0, 0);
      srcRegion.myTexelSize = glm::uvec3((uint)destSize.x, (uint)destSize.y, 1);
      ctx->CopyTextureRegion(aTexture.get(), destLocation, glm::uvec3(0, 0, 0), tempTexResource[1].myTexture, TextureSubLocation(), &srcRegion);

      srcSize = glm::ceil(srcSize * 0.5f);
      destSize = glm::ceil(destSize * 0.5f);
    }

    queue->ExecuteContext(ctx, true);
    RenderCore::FreeContext(ctx);
  }
//---------------------------------------------------------------------------//
