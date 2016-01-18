#include "MaterialPassInstance.h"
#include "MaterialPass.h"
#include "MathUtil.h"
#include "Texture.h"
#include "GpuBuffer.h"
#include "TextureSampler.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  uint MaterialPassInstance::computeHash() const
  {
    uint hash = 0x0;

    for (uint32 i = 0u; i < Constants::kMaxNumReadTextures; ++i)
    {
      Texture* tex = m_vpReadTextures[i];
      MathUtil::hash_combine(hash, tex ? tex->getPath().getHash() : 0u);
    }

    for (uint32 i = 0u; i < Constants::kMaxNumWriteTextures; ++i)
    {
      Texture* tex = m_vpWriteTextures[i];
      MathUtil::hash_combine(hash, tex ? tex->getPath().getHash() : 0u);
    }

    for (uint32 i = 0u; i < Constants::kMaxNumReadBuffers; ++i)
    {
      GpuBuffer* buf = m_vpReadBuffers[i];
      MathUtil::hash_combine(hash, buf ? buf->getName().getHash() : 0u);
    }

    for (uint32 i = 0u; i < Constants::kMaxNumWriteBuffers; ++i)
    {
      GpuBuffer* buf = m_vpWriteBuffers[i];
      MathUtil::hash_combine(hash, buf ? buf->getName().getHash() : 0u);
    }

    for (uint32 i = 0u; i < Constants::kMaxNumTextureSamplers; ++i)
    {
      TextureSampler* sampler = m_vpTextureSamplers[i];
      MathUtil::hash_combine(hash, sampler ? sampler->getName().getHash() : 0u);
    }

    return hash;
  }
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
        entry.myName = m_vpReadTextures[i]->getPath();
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
        entry.myName = m_vpWriteTextures[i]->getPath();
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
        entry.myName = m_vpReadBuffers[i]->getName();
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
        entry.myName = m_vpWriteBuffers[i]->getName();
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
        entry.myName = m_vpTextureSamplers[i]->getName();
        someEntries.push_back(entry);
      }
    }
  }
  //---------------------------------------------------------------------------//
  void MaterialPassInstance::setFromResourceDesc
    (const std::vector<ResourceStorageEntry>& someResources, MpiResourceType aType)
  {
    if (aType == MpiResourceType::ReadTexture)
    {
      for (const ResourceStorageEntry& entry : someResources)
      {
        m_vpReadTextures[entry.myIndex] = Texture::getByName(entry.myName);
      }
    }

    else if (aType == MpiResourceType::WriteTexture)
    {
      for (const ResourceStorageEntry& entry : someResources)
      {
        m_vpWriteTextures[entry.myIndex] = Texture::getByName(entry.myName);
      }
    }

    //else if (aType == MpiResourceType::ReadBuffer)
    //{
    //  for (const ResourceStorageEntry entry : someResources)
    //  {
    //    m_vpReadBuffers[entry.myShaderStage][entry.myIndex] = GpuBuffer::getByName(entry.myName);
    //  }
    //}
    //
    //else if (aType == MpiResourceType::WriteBuffer)
    //{
    //  for (const ResourceStorageEntry entry : someResources)
    //  {
    //    m_vpWriteBuffers[entry.myShaderStage][entry.myIndex] = GpuBuffer::getByName(entry.myName);
    //  }
    //}

    else if (aType == MpiResourceType::TextureSampler)
    {
      for (const ResourceStorageEntry& entry : someResources)
      {
        m_vpTextureSamplers[entry.myIndex] = TextureSampler::getByName(entry.myName);
      }
    }
  }
//---------------------------------------------------------------------------//
  void ResourceStorageEntry::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&myIndex, "myIndex");
    aSerializer->serialize(&myName, "myName");
  }
//---------------------------------------------------------------------------//
  MaterialPassInstance::MaterialPassInstance(MaterialPass* aMaterialPass) :
    m_pMaterialPass(aMaterialPass)
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
  void MaterialPassInstance::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serialize(&m_pMaterialPass, "m_pMaterialPass");

    std::vector<ResourceStorageEntry> readTextures;
    getResourceDesc(MpiResourceType::ReadTexture, readTextures);
    aSerializer->serialize(&readTextures, "readTextures");

    std::vector<ResourceStorageEntry> writeTextures;
    getResourceDesc(MpiResourceType::WriteTexture, writeTextures);
    aSerializer->serialize(&writeTextures, "writeTextures");

    std::vector<ResourceStorageEntry> textureSamplers;
    getResourceDesc(MpiResourceType::TextureSampler, textureSamplers);
    aSerializer->serialize(&textureSamplers, "textureSamplers");

    if (aSerializer->getMode() == IO::ESerializationMode::LOAD)
    {
      setFromResourceDesc(readTextures, MpiResourceType::ReadTexture);
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
