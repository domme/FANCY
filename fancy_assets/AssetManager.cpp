#include "fancy_assets_precompile.h"
#include "AssetManager.h"

#include <fancy_core/Mesh.h>
#include <fancy_core/Texture.h>
#include <fancy_core/CommandList.h>
#include <fancy_core/Log.h>
#include <fancy_core/RenderCore.h>
#include <fancy_core/PathService.h>
#include <fancy_core/DataFormat.h>
#include <fancy_core/BinaryCache.h>
#include <fancy_core/GraphicsResources.h>
#include <fancy_core/TempResourcePool.h>
#include <fancy_core/FixedArray.h>
#include <fancy_core/CommandQueue.h>

#include "ImageLoader.h"
#include "MaterialDesc.h"
#include "ModelDesc.h"
#include "Model.h"

#include "Material.h"

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

      TextureData textureData;
      TextureProperties texProps;
      if (BinaryCache::ReadTextureData(texPathRel.c_str(), timestamp, texProps, textureData))
      {
        texProps.myIsShaderWritable = (someLoadFlags & SHADER_WRITABLE) != 0;
        SharedPtr<Texture> texFromDiskCache = RenderCore::CreateTexture(texProps, texProps.path.c_str(), textureData.mySubDatas.data(), (uint) textureData.mySubDatas.size());

        ASSERT(texFromDiskCache != nullptr);

        myTextures[texPathRelHash] = texFromDiskCache;
        return texFromDiskCache;
      }
    }

    Image image;
    if (!ImageLoader::Load(texPathAbs.c_str(), image))
    {
      LOG_ERROR("Failed to load texture at path %s", texPathAbs.c_str());
      return nullptr;
    }

    TextureProperties texProps;
    texProps.myDimension = GpuResourceDimension::TEXTURE_2D;
    texProps.path = texPathRel;
    texProps.bIsDepthStencil = false;
    texProps.myWidth = (uint) image.mySize.x;
    texProps.myHeight = (uint) image.mySize.y;
    texProps.myDepthOrArraySize = 0u;
    texProps.myAccessType = CpuMemoryAccessType::NO_CPU_ACCESS;
    texProps.myIsShaderWritable = (someLoadFlags & SHADER_WRITABLE) != 0;

    if (!(image.myBitsPerChannel == 8u || image.myBitsPerChannel == 16u))
    {
      LOG_ERROR("Unsupported bits per channel in texture %s (has %d bits per channel)", texPathAbs.c_str(), image.myBitsPerChannel);
      return nullptr;
    }

    switch(image.myNumChannels)
    {
      case 1: texProps.eFormat = image.myBitsPerChannel == 8 ? DataFormat::R_8 : DataFormat::R_16; break;
      case 2: texProps.eFormat = image.myBitsPerChannel == 8 ? DataFormat::RG_8 : DataFormat::RG_16; break;
      // 3-channels unsupported in all modern rendering-APIs. Image-importer lib should deal with it to convert it to 4 channels      
      case 4: texProps.eFormat = image.myBitsPerChannel == 8 ? DataFormat::SRGB_8_A_8 : DataFormat::RGBA_16; break;
      default: ASSERT(false, "Unsupported channels");
      return nullptr;
    }

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(texProps.eFormat);
    const uint expectedDataSize = formatInfo.mySizeBytes * texProps.myWidth * texProps.myHeight;
    if (expectedDataSize != image.myByteSize)
    {
      LOG_ERROR("Invalid pixel data loaded from texture %s. Expected: %d, Actual: %d", texPathAbs.c_str(), expectedDataSize, image.myByteSize);
      return nullptr;
    }
    
    TextureSubData dataFirstMip;
    dataFirstMip.myData = image.myData.get();
    dataFirstMip.myPixelSizeBytes = (image.myBitsPerChannel * image.myNumChannels) / 8u;
    dataFirstMip.myRowSizeBytes = image.mySize.x * dataFirstMip.myPixelSizeBytes;
    dataFirstMip.mySliceSizeBytes = image.mySize.x * image.mySize.y * dataFirstMip.myPixelSizeBytes;
    dataFirstMip.myTotalSizeBytes = dataFirstMip.mySliceSizeBytes;
    SharedPtr<Texture> tex = RenderCore::CreateTexture(texProps, texPathRel.c_str(), &dataFirstMip, 1u);

    if (tex != nullptr)
    {
      ComputeMipmaps(tex);

      TextureData textureData;
      bool success = RenderCore::ReadbackTextureData(tex.get(), TextureSubLocation(1), tex->myNumSubresources - 1, textureData);
      textureData.mySubDatas.insert(textureData.mySubDatas.begin(), dataFirstMip);

      BinaryCache::WriteTextureData(tex->GetProperties(), textureData.mySubDatas.data(), (uint) textureData.mySubDatas.size());

      // Test:
      //tex = RenderCore::CreateTexture(texProps, texPathRel.c_str(), textureData.mySubDatas.data(), textureData.mySubDatas.size());

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

    CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
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

    ctx->ResourceBarrier(aTexture.get(), aTexture->GetProperties().myDefaultState, GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE);
    ctx->ResourceBarrier(tempTexResource[0].myTexture, tempTexResource[0].myTexture->GetProperties().myDefaultState, GpuResourceUsageState::WRITE_COMPUTE_SHADER_UAV);
    ctx->ResourceBarrier(tempTexResource[1].myTexture, tempTexResource[1].myTexture->GetProperties().myDefaultState, GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE);
    glm::float2 destSize = glm::ceil(srcSize * 0.5f);
    for (uint mip = 1u; mip < numMips; ++mip)
    {
      const GpuResourceView* resourceViews[] = { nullptr, nullptr };

      // Resize horizontal
      glm::float2 tempDestSize(destSize.x, srcSize.y);
      cBuffer.mySrcSize = srcSize;
      cBuffer.myDestSize = tempDestSize;
      cBuffer.mySrcScale = tempDestSize / srcSize;
      cBuffer.myDestScale = srcSize / tempDestSize;
      cBuffer.myAxis = glm::float2(1.0f, 0.0f);
      ctx->BindConstantBuffer(&cBuffer, sizeof(cBuffer), 0u);
      resourceViews[0] = readViews[mip - 1].get();
      resourceViews[1] = tempTexResource[0].myWriteView;
      ctx->ResourceBarrier(tempTexResource[0].myTexture, GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE, GpuResourceUsageState::WRITE_COMPUTE_SHADER_UAV);
      ctx->BindResourceSet(resourceViews, 2, 1u);
      ctx->Dispatch(glm::int3((int)destSize.x, (int)srcSize.y, 1));
      ctx->ResourceUAVbarrier();

      // Resize vertical
      cBuffer.mySrcSize = tempDestSize;
      cBuffer.myDestSize = destSize;
      cBuffer.mySrcScale = destSize / tempDestSize;
      cBuffer.myDestScale = tempDestSize / destSize;
      cBuffer.myAxis = glm::float2(0.0f, 1.0f);
      ctx->BindConstantBuffer(&cBuffer, sizeof(cBuffer), 0u);
      resourceViews[0] = tempTexResource[0].myReadView;
      resourceViews[1] = tempTexResource[1].myWriteView;
      ctx->ResourceBarrier(tempTexResource[0].myTexture, GpuResourceUsageState::WRITE_COMPUTE_SHADER_UAV, GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE);
      ctx->ResourceBarrier(tempTexResource[1].myTexture, GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE, GpuResourceUsageState::WRITE_COMPUTE_SHADER_UAV);
      ctx->BindResourceSet(resourceViews, 2, 1u);
      ctx->Dispatch(glm::int3((int)destSize.x, (int)destSize.y, 1));
      ctx->ResourceUAVbarrier();

      TextureSubLocation destLocation;
      destLocation.myMipLevel = mip;
      const uint16 subresourceIndex = aTexture->GetSubresourceIndex(destLocation);
      
      ctx->ResourceBarrier(tempTexResource[1].myTexture, GpuResourceUsageState::WRITE_COMPUTE_SHADER_UAV, GpuResourceUsageState::READ_COPY_SOURCE);
      ctx->SubresourceBarrier(aTexture.get(), &subresourceIndex, 1u, GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE, GpuResourceUsageState::WRITE_COPY_DEST);

      TextureRegion srcRegion;
      srcRegion.myTexelPos = glm::uvec3(0, 0, 0);
      srcRegion.myTexelSize = glm::uvec3((uint)destSize.x, (uint)destSize.y, 1);
      ctx->CopyTextureRegion(aTexture.get(), destLocation, glm::uvec3(0, 0, 0), tempTexResource[1].myTexture, TextureSubLocation(), &srcRegion);

      ctx->SubresourceBarrier(aTexture.get(), &subresourceIndex, 1u, GpuResourceUsageState::WRITE_COPY_DEST, GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE);
      ctx->ResourceBarrier(tempTexResource[1].myTexture, GpuResourceUsageState::WRITE_COPY_DEST, GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE);

      srcSize = glm::ceil(srcSize * 0.5f);
      destSize = glm::ceil(destSize * 0.5f);
    }

    RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
  }
//---------------------------------------------------------------------------//
