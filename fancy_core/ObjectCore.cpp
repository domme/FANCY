#include "fancy_core_precompile.h"
#include "ObjectCore.h"
#include "RenderCore.h"
#include "ShaderPipeline.h"
#include "PathService.h"
#include "Material.h"
#include "Mesh.h"
#include "GpuBuffer.h"
#include "CommandList.h"
#include "Texture.h"
#include "BinaryCache.h"
#include "ImageLoader.h"
#include "TextureReadbackTask.h"
#include "GraphicsResources.h"
#include "CommandLine.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  std::map<uint64, SharedPtr<TextureView>> ObjectCore::ourTextureCache;
  std::map<uint64, SharedPtr<Mesh>> ObjectCore::ourMeshCache;
  std::map<uint64, SharedPtr<Material>> ObjectCore::ourMaterialCache;
  SharedPtr<ShaderPipeline> ObjectCore::ourMipDownsampleShader;
//---------------------------------------------------------------------------//
  void ObjectCore::Init()
  {
    ShaderPipelineDesc pipelineDesc;
    ShaderDesc& shaderDesc = pipelineDesc.myShader[(uint)ShaderStage::COMPUTE];
    shaderDesc.myPath = "Downsample.hlsl";
    shaderDesc.myShaderStage = (uint)ShaderStage::COMPUTE;
    shaderDesc.myMainFunction = "main";

    ourMipDownsampleShader = RenderCore::CreateShaderPipeline(pipelineDesc);
    ASSERT(ourMipDownsampleShader != nullptr);
  }
 //---------------------------------------------------------------------------//
  SharedPtr<Mesh> ObjectCore::GetMesh(const MeshDesc& aDesc)
  {
    auto it = ourMeshCache.find(aDesc.myHash);
    if (it != ourMeshCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> ObjectCore::CreateMesh(const MeshData& aMeshData)
  {
    SharedPtr<Mesh> cachedMesh = GetMesh(aMeshData.myDesc);
    if (cachedMesh)
      return cachedMesh;

    SharedPtr<Mesh> mesh(new Mesh());
    mesh->myDesc = aMeshData.myDesc;

    for (uint i = 0u; i < aMeshData.myParts.size(); ++i)
    {
      const MeshPartData& partData = aMeshData.myParts[i];
      SharedPtr<MeshPart> meshPart(new MeshPart());

      const VertexInputLayoutProperties& vertexLayoutProperties = partData.myVertexLayoutProperties;
      meshPart->myVertexInputLayout = RenderCore::CreateVertexInputLayout(vertexLayoutProperties);

      const uint64 overallVertexSize = vertexLayoutProperties.GetOverallVertexSize();

      const uint8* ptrToVertexData = partData.myVertexData.data();
      const uint64 numVertices = DYN_ARRAY_BYTESIZE(partData.myVertexData) / overallVertexSize;;

      GpuBufferProperties bufferParams;
      bufferParams.myBindFlags = (uint)GpuBufferBindFlags::VERTEX_BUFFER;
      bufferParams.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
      bufferParams.myNumElements = numVertices;
      bufferParams.myElementSizeBytes = overallVertexSize;

      StaticString<256> name("VertexBuffer_Mesh_%d_%s_%lld", i, aMeshData.myDesc.myName.c_str(), aMeshData.myDesc.myHash);
      meshPart->myVertexBuffer = RenderCore::CreateBuffer(bufferParams, name, ptrToVertexData);

      const uint8* ptrToIndexData = partData.myIndexData.data();
      const uint64 numIndices = (partData.myIndexData.size() * sizeof(uint8)) / sizeof(uint);

      GpuBufferProperties indexBufParams;
      indexBufParams.myBindFlags = (uint)GpuBufferBindFlags::INDEX_BUFFER;
      indexBufParams.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
      indexBufParams.myNumElements = numIndices;
      indexBufParams.myElementSizeBytes = sizeof(uint);

      name.Format("IndexBuffer_Mesh_%d_%s_%lld", i, aMeshData.myDesc.myName.c_str(), aMeshData.myDesc.myHash);
      meshPart->myIndexBuffer = RenderCore::CreateBuffer(indexBufParams, name, ptrToIndexData);

      mesh->myParts.push_back(meshPart);
    }

    ourMeshCache[aMeshData.myDesc.myHash] = mesh;

    return mesh;
  }
//---------------------------------------------------------------------------//
  uint64 ObjectCore::ComputeMeshVertexHash(const MeshPartData* someMeshPartDatas, uint aNumParts)
  {
    MathUtil::BeginMultiHash();

    for (uint i = 0u; i < aNumParts; ++i)
      MathUtil::AddToMultiHash(someMeshPartDatas[i].myVertexData.data(), DYN_ARRAY_BYTESIZE(someMeshPartDatas[i].myVertexData));

    return MathUtil::EndMultiHash();
  }
//---------------------------------------------------------------------------//
  SharedPtr<Material> ObjectCore::GetMaterial(const MaterialDesc& aDesc)
  {
    const uint64 hash = aDesc.GetHash();

    auto it = ourMaterialCache.find(hash);
    if (it != ourMaterialCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Material> ObjectCore::CreateMaterial(const MaterialDesc& aDesc)
  {
    SharedPtr<Material> mat = GetMaterial(aDesc);
    if (mat)
      return mat;

    mat.reset(new Material());

    for (uint i = 0u; i < (uint)MaterialTextureType::NUM; ++i)
      if (!aDesc.myTextures[i].empty())
        mat->myTextures[i] = LoadTexture(aDesc.myTextures[i].c_str());

    static_assert(sizeof(mat->myParameters) == sizeof(aDesc.myParameters), "Mismatch in parameter data size");
    memcpy(mat->myParameters, aDesc.myParameters, sizeof(mat->myParameters));

    ourMaterialCache[aDesc.GetHash()] = mat;
    return mat;
  }
//---------------------------------------------------------------------------//
  SharedPtr<TextureView> ObjectCore::GetTexture(const char* aPath, uint someFlags)
  {
    String texPathAbs = aPath;
    String texPathRel = aPath;
    if (!Path::IsPathAbsolute(texPathAbs))
      texPathAbs = Path::GetAbsoluteResourcePath(texPathAbs);
    else
      texPathRel = Path::GetRelativeResourcePath(texPathAbs);

    uint64 texPathRelHash = MathUtil::Hash(texPathRel);
    MathUtil::hash_combine(texPathRelHash, ((uint64)someFlags & SHADER_WRITABLE));

    auto it = ourTextureCache.find(texPathRelHash);
    if (it != ourTextureCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<TextureView> ObjectCore::LoadTexture(const char* aPath, uint someLoadFlags /* = 0 */)
  {
    if (strlen(aPath) == 0)
      return nullptr;

    String texPathAbs = aPath;
    String texPathRel = aPath;
    if (!Path::IsPathAbsolute(texPathAbs))
      texPathAbs = Path::GetAbsoluteResourcePath(texPathAbs);
    else
      texPathRel = Path::GetRelativeResourcePath(texPathAbs);

    if ((someLoadFlags & NO_MEM_CACHE) == 0)
      if (SharedPtr<TextureView> texFromMemCache = GetTexture(texPathRel.c_str()))
        return texFromMemCache;

    uint64 texPathRelHash = MathUtil::Hash(texPathRel);
    MathUtil::hash_combine(texPathRelHash, ((uint64)someLoadFlags & SHADER_WRITABLE));

    if ((someLoadFlags & NO_DISK_CACHE) == 0 && !CommandLine::GetInstance()->HasArgument("noDiskCache"))
    {
      TextureData textureData;
      TextureProperties texProps;
      if (BinaryCache::ReadTextureData(texPathRel.c_str(), texProps, textureData))
      {
        texProps.myIsShaderWritable = (someLoadFlags & SHADER_WRITABLE) != 0;
        SharedPtr<Texture> texFromDiskCache = RenderCore::CreateTexture(texProps, texProps.path.c_str(), textureData.mySubDatas.data(), (uint)textureData.mySubDatas.size());
        ASSERT(texFromDiskCache != nullptr);

        TextureViewProperties viewProps;
        viewProps.mySubresourceRange = texFromDiskCache->mySubresources;
        viewProps.myFormat = texFromDiskCache->GetProperties().myFormat;
        SharedPtr<TextureView> texView = RenderCore::CreateTextureView(texFromDiskCache, viewProps, aPath);
        ASSERT(texView != nullptr);

        ourTextureCache[texPathRelHash] = texView;
        return texView;
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
    texProps.myWidth = (uint)image.mySize.x;
    texProps.myHeight = (uint)image.mySize.y;
    texProps.myAccessType = CpuMemoryAccessType::NO_CPU_ACCESS;
    texProps.myIsShaderWritable = (someLoadFlags & SHADER_WRITABLE) != 0;

    if (!(image.myBitsPerChannel == 8u || image.myBitsPerChannel == 16u))
    {
      LOG_ERROR("Unsupported bits per channel in texture %s (has %d bits per channel)", texPathAbs.c_str(), image.myBitsPerChannel);
      return nullptr;
    }

    switch (image.myNumChannels)
    {
    case 1: texProps.myFormat = image.myBitsPerChannel == 8 ? DataFormat::R_8 : DataFormat::R_16; break;
    case 2: texProps.myFormat = image.myBitsPerChannel == 8 ? DataFormat::RG_8 : DataFormat::RG_16; break;
      // 3-channels unsupported in all modern rendering-APIs. Image-importer lib should deal with it to convert it to 4 channels      
    case 4: texProps.myFormat = image.myBitsPerChannel == 8 ? DataFormat::SRGB_8_A_8 : DataFormat::RGBA_16; break;
    default: ASSERT(false, "Unsupported channels");
      return nullptr;
    }

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(texProps.myFormat);
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
      TextureData textureData;

      if (texProps.myNumMipLevels > 1)
      {
        ComputeMipmaps(tex);
        SubresourceRange subresourceRange = tex->GetSubresources();
        subresourceRange.myFirstMipLevel = 1;
        subresourceRange.myNumMipLevels -= 1;

        TextureReadbackTask readbackTask = RenderCore::ReadbackTexture(tex.get(), subresourceRange);
        readbackTask.Wait();

        readbackTask.GetData(textureData);
      }

      textureData.mySubDatas.insert(textureData.mySubDatas.begin(), dataFirstMip);
      BinaryCache::WriteTextureData(texPathRel.c_str(), const_cast<TextureProperties&>(tex->GetProperties()), textureData);

      // Debug for testing binary cache by reloading the texture right away:
      //tex = RenderCore::CreateTexture(texProps, texPathRel.c_str(), textureData.mySubDatas.data(), textureData.mySubDatas.size());

      TextureViewProperties viewProps;
      viewProps.mySubresourceRange = tex->mySubresources;
      viewProps.myFormat = tex->GetProperties().myFormat;
      SharedPtr<TextureView> texView = RenderCore::CreateTextureView(tex, viewProps, aPath);

      ourTextureCache[texPathRelHash] = texView;
      return texView;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  void ObjectCore::ComputeMipmaps(const SharedPtr<Texture>& aTexture, ResampleFilter aFilter)
  {
    const TextureProperties& texProps = aTexture->GetProperties();
    const uint numMips = texProps.myNumMipLevels;

    SharedPtr<Texture> texture = aTexture;
    if (!texProps.myIsShaderWritable)
    {
      TextureProperties props = aTexture->GetProperties();
      props.myIsRenderTarget = false;
      props.myIsShaderWritable = true;
      texture = RenderCore::CreateTexture(props, "Mipmap target UAV copy");
    }

    TextureResourceProperties tempTexProps;
    tempTexProps.myIsShaderWritable = true;
    tempTexProps.myIsRenderTarget = false;
    tempTexProps.myIsTexture = true;
    tempTexProps.myTextureProperties = texProps;
    tempTexProps.myTextureProperties.myWidth = texProps.myWidth / 2;
    tempTexProps.myTextureProperties.myHeight = texProps.myHeight / 2;
    tempTexProps.myTextureProperties.myNumMipLevels = 1u;
    TempTextureResource tempTexture = RenderCore::AllocateTempTexture(tempTexProps, 0u, "Temp mipmapping texture");

    const uint kMaxNumMips = 17;
    ASSERT(numMips <= kMaxNumMips);
    FixedArray<SharedPtr<TextureView>, kMaxNumMips> readViews;
    FixedArray<SharedPtr<TextureView>, kMaxNumMips> writeViews;

    TextureViewProperties props;
    props.mySubresourceRange.myNumMipLevels = 1;
    for (uint mip = 0u; mip < numMips; ++mip)
    {
      props.mySubresourceRange.myFirstMipLevel = mip;
      props.myIsShaderWritable = false;
      props.myFormat = texture->GetProperties().myFormat;
      readViews[mip] = RenderCore::CreateTextureView(texture, props);

      props.myIsShaderWritable = true;
      props.myFormat = DataFormatInfo::GetNonSRGBformat(props.myFormat);
      writeViews[mip] = RenderCore::CreateTextureView(texture, props);
    }

    CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
    if (texture != aTexture)
      ctx->CopyTexture(texture.get(), SubresourceLocation(0), aTexture.get(), SubresourceLocation(0));

    ctx->SetShaderPipeline(ourMipDownsampleShader.get());

    struct CBuffer
    {
      glm::int2 mySrcTextureSize;
      int myIsSRGB;
    } cBuffer;

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(texProps.myFormat);
    cBuffer.myIsSRGB = formatInfo.mySRGB ? 1 : 0;

    const glm::int2 fullSize(texProps.myWidth, texProps.myHeight);
    for (uint mip = 1u; mip < numMips; ++mip)
    {
      const glm::int2 srcSize = fullSize >> (int)(mip - 1);
      const glm::int2 dstSize = fullSize >> (int)mip;

      cBuffer.mySrcTextureSize = glm::int2((int)srcSize.x, (int)srcSize.y);
      ctx->BindConstantBuffer(&cBuffer, sizeof(cBuffer), "CB0");
      ctx->BindResourceView(readViews[mip - 1].get(), "SrcTexture");
      ctx->BindResourceView(writeViews[mip].get(), "DestTexture");
      ctx->Dispatch(glm::int3(dstSize.x, dstSize.y, 1));
      ctx->ResourceUAVbarrier();
    }

    if (aTexture != texture)
      ctx->CopyResource(aTexture.get(), texture.get());

    RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
  }
//---------------------------------------------------------------------------//
}
