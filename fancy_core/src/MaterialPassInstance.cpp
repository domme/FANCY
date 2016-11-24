#include "MaterialPassInstance.h"
#include "MaterialPass.h"
#include "MathUtil.h"
#include "Texture.h"
#include "GpuBuffer.h"
#include "TextureSampler.h"
#include "Serializer.h"
#include "Renderer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  void MaterialPassInstance::getResourceDesc(MpiResourceType aType, std::vector<ResourceStorageEntry>& someEntries) const
  {
    if (aType == MpiResourceType::ReadTexture)
    {
      for (uint32 i = 0u; i < Constants::kMaxNumReadTextures; ++i)
      {
        if (!m_vpReadTextures[i])
          continue;

        ResourceStorageEntry entry;
        entry.myIndex = i;
        entry.myHash = m_vpReadTextures[i]->GetDescription().GetHash();
        someEntries.push_back(entry);
      }
    }

    else if (aType == MpiResourceType::WriteTexture)
    {
      for (uint32 i = 0u; i < Constants::kMaxNumWriteTextures; ++i)
      {
        if (!m_vpWriteTextures[i])
          continue;

        ResourceStorageEntry entry;
        entry.myIndex = i;
        entry.myHash = m_vpWriteTextures[i]->GetDescription().GetHash();
        someEntries.push_back(entry);
      }
    }

    else if (aType == MpiResourceType::ReadBuffer)
    {
      for (uint32 i = 0u; i < Constants::kMaxNumReadBuffers; ++i)
      {
        if (!m_vpReadBuffers[i])
          continue;

        ResourceStorageEntry entry;
        entry.myIndex = i;
        entry.myHash = m_vpReadBuffers[i]->GetDescription().GetHash();
        someEntries.push_back(entry);
      }
    }

    else if (aType == MpiResourceType::WriteBuffer)
    {
      for (uint32 i = 0u; i < Constants::kMaxNumWriteBuffers; ++i)
      {
        if (!m_vpWriteBuffers[i])
          continue;

        ResourceStorageEntry entry;
        entry.myIndex = i;
        entry.myHash = m_vpWriteBuffers[i]->GetDescription().GetHash();
        someEntries.push_back(entry);
      }
    }

    else if (aType == MpiResourceType::TextureSampler)
    {
      for (uint32 i = 0u; i < Constants::kMaxNumTextureSamplers; ++i)
      {
        if (!m_vpTextureSamplers[i])
          continue;

        ResourceStorageEntry entry;
        entry.myIndex = i;
        entry.myHash = m_vpTextureSamplers[i]->GetDescription().GetHash();
        someEntries.push_back(entry);
      }
    }
  }
  //---------------------------------------------------------------------------//
  void MaterialPassInstance::setFromResourceDesc
    (const std::vector<ResourceStorageEntry>& someResources, MpiResourceType aType)
  {
    switch(aType)
    {
    case MpiResourceType::ReadTexture:
      for (const ResourceStorageEntry& entry : someResources)
        m_vpReadTextures[entry.myIndex] = RenderCore::GetTexture(entry.myHash);
      break;
    case MpiResourceType::WriteTexture:
      for (const ResourceStorageEntry& entry : someResources)
        m_vpWriteTextures[entry.myIndex] = RenderCore::GetTexture(entry.myHash);
      break;
    case MpiResourceType::ReadBuffer:
      for (const ResourceStorageEntry entry : someResources)
        m_vpReadBuffers[entry.myIndex] = GpuBuffer::Find(entry.myHash);
      break;
    case MpiResourceType::WriteBuffer:
      for (const ResourceStorageEntry entry : someResources)
        m_vpWriteBuffers[entry.myIndex] = GpuBuffer::Find(entry.myHash);
      break;
    case MpiResourceType::TextureSampler:
      for (const ResourceStorageEntry& entry : someResources)
        m_vpTextureSamplers[entry.myIndex] = TextureSampler::Find(entry.myHash);
      break;
    default:
      ASSERT(false);
    }
  }
//---------------------------------------------------------------------------//
  void ResourceStorageEntry::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&myIndex, "myIndex");
    aSerializer->serialize(&myHash, "myHash");
  }
//---------------------------------------------------------------------------//
  MaterialPassInstance::MaterialPassInstance() :
    m_pMaterialPass(nullptr)
  {
    memset(m_vpReadTextures, 0x0, sizeof(m_vpReadTextures));
    memset(m_vpWriteTextures, 0x0, sizeof(m_vpWriteTextures));
    memset(m_vpReadBuffers, 0x0, sizeof(m_vpReadBuffers));
    memset(m_vpWriteBuffers, 0x0, sizeof(m_vpWriteBuffers));
    memset(m_vpTextureSamplers, 0x0, sizeof(m_vpTextureSamplers));
  }
//---------------------------------------------------------------------------//
  MaterialPassInstance::~MaterialPassInstance()
  {

  }
//---------------------------------------------------------------------------//
  bool MaterialPassInstance::operator==(const MaterialPassInstanceDesc& aDesc) const 
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  MaterialPassInstanceDesc MaterialPassInstance::GetDescription() const
  {
    MaterialPassInstanceDesc desc;

    desc.myMaterialPass = m_pMaterialPass->GetDescription();
    
    for (uint i = 0u; i < Constants::kMaxNumReadTextures; ++i)
      if (nullptr != m_vpReadTextures[i])
        desc.myReadTextures[i] = m_vpReadTextures[i]->GetDescription();

    for (uint i = 0u; i < Constants::kMaxNumWriteTextures; ++i)
      if (nullptr != m_vpWriteTextures[i])
        desc.myWriteTextures[i] = m_vpWriteTextures[i]->GetDescription();

    for (uint i = 0u; i < Constants::kMaxNumReadBuffers; ++i)
      if (nullptr != m_vpReadBuffers[i])
        desc.myReadBuffers[i] = m_vpReadBuffers[i]->GetDescription();

    for (uint i = 0u; i < Constants::kMaxNumWriteBuffers; ++i)
      if (nullptr != m_vpWriteBuffers[i])
        desc.myWriteBuffers[i] = m_vpWriteBuffers[i]->GetDescription();

    for (uint i = 0u; i < Constants::kMaxNumTextureSamplers; ++i)
      if (nullptr != m_vpTextureSamplers[i])
        desc.myTextureSamplers[i] = m_vpTextureSamplers[i]->GetDescription();

    return desc;
  }
//---------------------------------------------------------------------------//
  void MaterialPassInstance::SetFromDescription(const MaterialPassInstanceDesc& aDesc)
  {
    m_pMaterialPass = MaterialPass::FindFromDesc(aDesc.myMaterialPass);
    ASSERT(nullptr != m_pMaterialPass);

    for (uint i = 0u; i < Constants::kMaxNumReadTextures; ++i)
      m_vpReadTextures[i] = RenderCore::GetTexture(aDesc.myReadTextures[i].GetHash());

    for (uint i = 0u; i < Constants::kMaxNumWriteTextures; ++i)
      m_vpWriteTextures[i] = RenderCore::GetTexture(aDesc.myWriteTextures[i].GetHash());

    for (uint i = 0u; i < Constants::kMaxNumReadBuffers; ++i)
      m_vpReadBuffers[i] = GpuBuffer::FindFromDesc(aDesc.myReadBuffers[i]);

    for (uint i = 0u; i < Constants::kMaxNumWriteBuffers; ++i)
      m_vpWriteBuffers[i] = GpuBuffer::FindFromDesc(aDesc.myWriteBuffers[i]);

    for (uint i = 0u; i < Constants::kMaxNumTextureSamplers; ++i)
      m_vpTextureSamplers[i] = TextureSampler::FindFromDesc(aDesc.myTextureSamplers[i]);
  }
//---------------------------------------------------------------------------//
  void MaterialPassInstance::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_pMaterialPass, "m_pMaterialPass");

    /*
    std::vector<ResourceStorageEntry> readTextures;
    getResourceDesc(MpiResourceType::ReadTexture, readTextures);
    aSerializer->serialize(&readTextures, "readTextures");
    */
    aSerializer->serialize(&m_vpReadTextures, "m_vpReadTextures"); 

    std::vector<ResourceStorageEntry> writeTextures;
    getResourceDesc(MpiResourceType::WriteTexture, writeTextures);
    aSerializer->serialize(&writeTextures, "writeTextures");

    std::vector<ResourceStorageEntry> textureSamplers;
    getResourceDesc(MpiResourceType::TextureSampler, textureSamplers);
    aSerializer->serialize(&textureSamplers, "textureSamplers");

    if (aSerializer->getMode() == IO::ESerializationMode::LOAD)
    {
      // setFromResourceDesc(readTextures, MpiResourceType::ReadTexture);
      setFromResourceDesc(writeTextures, MpiResourceType::WriteTexture);
      setFromResourceDesc(textureSamplers, MpiResourceType::TextureSampler);
    }
  }
  //---------------------------------------------------------------------------//

  //---------------------------------------------------------------------------//
  // Note: Unfortunately, we can't reflect binding points from OpenGL-shaders and we don't want to modify binding in app code...

  // void MaterialPassInstance::setReadTexture( ShaderStage _eStage, const ObjectName& _name, const Texture* _pTexture )
  // {
  //   const GpuProgram* pProgram = m_pMaterialPass->getGpuProgram(_eStage);
  //   ASSERT(pProgram);
  // 
  //   const GpuResourceInfoList& vReadTextureInfos = pProgram->getReadTextureInfoList();
  //   for (uint32 i = 0u; i < vReadTextureInfos.size(); ++i)
  //   {
  //     if (vReadTextureInfos[i].name == _name)
  //     {
  //       setReadTexture(_eStage, vReadTextureInfos[i].u32RegisterIndex, _pTexture);
  //       break;
  //     }
  //   }
  // }
  //// ---------------------------------------------------------------------------//
  // void MaterialPassInstance::setWriteTexture( ShaderStage _eStage, const ObjectName& _name, const Texture* _pTexture )
  // {
  //   const GpuProgram* pProgram = m_pMaterialPass->getGpuProgram(_eStage);
  //   ASSERT(pProgram);
  // 
  //   const GpuResourceInfoList& vWriteTextureInfos = pProgram->getWriteTextureInfoList();
  //   for (uint32 i = 0u; i < vWriteTextureInfos.size(); ++i)
  //   {
  //     if (vWriteTextureInfos[i].name == _name)
  //     {
  //       setWriteTexture(_eStage, vWriteTextureInfos[i].u32RegisterIndex, _pTexture);
  //       break;
  //     }
  //   }
  // }
  //// ---------------------------------------------------------------------------//
  // void MaterialPassInstance::setReadBuffer( ShaderStage _eStage, const ObjectName& _name, const GpuBuffer* _pBuffer )
  // {
  //   const GpuProgram* pProgram = m_pMaterialPass->getGpuProgram(_eStage);
  //   ASSERT(pProgram);
  // 
  //   const GpuResourceInfoList& vReadBufferInfos = pProgram->getReadBufferInfoList();
  //   for (uint32 i = 0u; i < vReadBufferInfos.size(); ++i)
  //   {
  //     if (vReadBufferInfos[i].name == _name)
  //     {
  //       setReadBuffer(_eStage, vReadBufferInfos[i].u32RegisterIndex, _pBuffer);
  //       break;
  //     }
  //   }
  // }
  //// ---------------------------------------------------------------------------//
  // void MaterialPassInstance::setWriteBuffer( ShaderStage _eStage, const ObjectName& _name, const GpuBuffer* _pBuffer )
  // {
  //   const GpuProgram* pProgram = m_pMaterialPass->getGpuProgram(_eStage);
  //   ASSERT(pProgram);
  // 
  //   const GpuResourceInfoList& vWriteBufferInfos = pProgram->getWriteBufferInfoList();
  //   for (uint32 i = 0u; i < vWriteBufferInfos.size(); ++i)
  //   {
  //     if (vWriteBufferInfos[i].name == _name)
  //     {
  //       setWriteBuffer(_eStage, vWriteBufferInfos[i].u32RegisterIndex, _pBuffer);
  //       break;
  //     }
  //   }
  // }
  //// ---------------------------------------------------------------------------//
  // void MaterialPassInstance::setTextureSampler( ShaderStage _eStage, const ObjectName& _name, const TextureSampler* _pTextureSampler )
  // {
  //   const GpuProgram* pProgram = m_pMaterialPass->getGpuProgram(_eStage);
  //   ASSERT(pProgram);
  // 
  //   const GpuResourceInfoList& vTextureSamplerInfos = pProgram->getReadTextureInfoList();
  //   for (uint32 i = 0u; i < vTextureSamplerInfos.size(); ++i)
  //   {
  //     if (vTextureSamplerInfos[i].name == _name)
  //     {
  //       setTextureSampler(_eStage, vTextureSamplerInfos[i].u32RegisterIndex, _pTextureSampler);
  //       break;
  //     }
  //   }
  // }
//---------------------------------------------------------------------------//
} }
