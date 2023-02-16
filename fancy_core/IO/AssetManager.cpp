#include "fancy_core_precompile.h"
#include "AssetManager.h"
#include "Rendering//RenderCore.h"
#include "Rendering/ShaderPipeline.h"
#include "PathService.h"
#include "Material.h"
#include "Mesh.h"
#include "Rendering/GpuBuffer.h"
#include "Rendering/CommandList.h"
#include "Rendering/Texture.h"
#include "BinaryCache.h"
#include "ImageLoader.h"
#include "Rendering/TextureReadbackTask.h"
#include "Rendering/GraphicsResources.h"
#include "Common/CommandLine.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  AssetManager::AssetManager()
  {
    ShaderPipelineDesc pipelineDesc;
    ShaderDesc& shaderDesc = pipelineDesc.myShader[(uint)ShaderStage::SHADERSTAGE_COMPUTE];
    shaderDesc.myPath = "fancy/resources/shaders/Downsample.hlsl";
    shaderDesc.myShaderStage = (uint)ShaderStage::SHADERSTAGE_COMPUTE;
    shaderDesc.myMainFunction = "main";

    ourMipDownsampleShader = RenderCore::CreateShaderPipeline(pipelineDesc);
    ASSERT(ourMipDownsampleShader != nullptr);
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> AssetManager::GetMesh(const MeshDesc& aDesc)
  {
    auto it = ourMeshCache.find(aDesc.myHash);
    if (it != ourMeshCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> AssetManager::CreateMesh(const MeshData& aMeshData)
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
      const uint64 numVertices = VECTOR_BYTESIZE(partData.myVertexData) / overallVertexSize;;

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
  uint64 AssetManager::ComputeMeshVertexHash(const MeshPartData* someMeshPartDatas, uint aNumParts)
  {
    MathUtil::BeginMultiHash();

    for (uint i = 0u; i < aNumParts; ++i)
      MathUtil::AddToMultiHash(someMeshPartDatas[i].myVertexData.data(), VECTOR_BYTESIZE(someMeshPartDatas[i].myVertexData));

    return MathUtil::EndMultiHash();
  }
//---------------------------------------------------------------------------//
  SharedPtr<Material> AssetManager::GetMaterial(const MaterialDesc& aDesc)
  {
    const uint64 hash = aDesc.GetHash();

    auto it = ourMaterialCache.find(hash);
    if (it != ourMaterialCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Material> AssetManager::CreateMaterial(const MaterialDesc& aDesc)
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
  SharedPtr<TextureView> AssetManager::GetTexture(const char* aPath, uint someFlags)
  {
    eastl::string texPathAbs = aPath;
    eastl::string texPathRel = aPath;
    if (!Path::IsPathAbsolute(texPathAbs))
      texPathAbs = Path::GetAbsolutePath(texPathAbs);
    else
      texPathRel = Path::GetRelativePath(texPathAbs);

    uint64 texPathRelHash = MathUtil::Hash(texPathRel);
    MathUtil::hash_combine(texPathRelHash, ((uint64)someFlags & SHADER_WRITABLE));

    auto it = ourTextureCache.find(texPathRelHash);
    if (it != ourTextureCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<TextureView> AssetManager::LoadTexture(const char* aPath, uint someLoadFlags /* = 0 */)
  {
    if (strlen(aPath) == 0)
      return nullptr;

    eastl::string texPathAbs = aPath;
    eastl::string texPathRel = aPath;
    if (!Path::IsPathAbsolute(texPathAbs))
      texPathAbs = Path::GetAbsolutePath(texPathAbs);
    else
      texPathRel = Path::GetRelativePath(texPathAbs);

    if ((someLoadFlags & NO_MEM_CACHE) == 0)
      if (SharedPtr<TextureView> texFromMemCache = GetTexture(texPathRel.c_str()))
        if(texFromMemCache->GetProperties().myIsShaderWritable == ((someLoadFlags & SHADER_WRITABLE) != 0))
          return texFromMemCache;

    uint64 texPathRelHash = MathUtil::Hash(texPathRel);
    MathUtil::hash_combine(texPathRelHash, ((uint64)someLoadFlags & SHADER_WRITABLE));

    SharedPtr<Texture> tex;

    {
      ImageData image;
      if (!ImageLoader::Load(texPathAbs.c_str(), someLoadFlags, image))
      {
        LOG_ERROR("Failed to load texture at path %s", texPathAbs.c_str());
        return nullptr;
      }

      tex = RenderCore::CreateTexture(image.myProperties, texPathRel.c_str(),
        image.myData.mySubDatas.data(), image.myData.mySubDatas.size());
    }
    
    if ( tex == nullptr )
    {
      LOG_ERROR("Failed to create loaded texture at path %s", texPathAbs.c_str());
      return nullptr;
    }

    if ( tex->GetProperties().myNumMipLevels == 1 && ( someLoadFlags & NO_MIP_GENERATION ) == 0 )
    {
      ComputeMipmaps(tex);
    }

    TextureViewProperties viewProps;
    viewProps.mySubresourceRange = tex->mySubresources;
    viewProps.myFormat = tex->GetProperties().myFormat;
    SharedPtr<TextureView> texView = RenderCore::CreateTextureView(tex, viewProps, aPath);

    ourTextureCache[texPathRelHash] = texView;
    return texView;
  }
//---------------------------------------------------------------------------//
  void AssetManager::ComputeMipmaps(const SharedPtr<Texture>& aTexture, ResampleFilter /*aFilter*/)
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

    eastl::fixed_vector<SharedPtr<TextureView>, 10> readViews;
    eastl::fixed_vector<SharedPtr<TextureView>, 10> writeViews;

    readViews.resize(numMips);
    writeViews.resize(numMips);

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
      int mySrcTextureIdx;
      int myDstTextureIdx;
    } cBuffer;

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(texProps.myFormat);
    cBuffer.myIsSRGB = formatInfo.mySRGB ? 1 : 0;

    const glm::int2 fullSize(texProps.myWidth, texProps.myHeight);
    for (uint mip = 1u; mip < numMips; ++mip)
    {
      const glm::int2 srcSize = fullSize >> (int)(mip - 1);
      const glm::int2 dstSize = fullSize >> (int)mip;

      cBuffer.mySrcTextureIdx = readViews[mip - 1]->GetGlobalDescriptorIndex();
      cBuffer.myDstTextureIdx = writeViews[mip]->GetGlobalDescriptorIndex();
      cBuffer.mySrcTextureSize = glm::int2((int)srcSize.x, (int)srcSize.y);
      ctx->BindConstantBuffer(&cBuffer, sizeof(cBuffer), 0);
      ctx->PrepareResourceShaderAccess(readViews[mip - 1].get());
      ctx->PrepareResourceShaderAccess(writeViews[mip].get());
      ctx->Dispatch(glm::int3(dstSize.x, dstSize.y, 1));
      ctx->ResourceUAVbarrier();
    }

    if (aTexture != texture)
      ctx->CopyResource(aTexture.get(), texture.get());

    RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
  }
//---------------------------------------------------------------------------//
}
