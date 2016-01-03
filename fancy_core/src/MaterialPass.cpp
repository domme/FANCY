#include "MaterialPass.h"
#include "MathUtil.h"
#include "Texture.h"
#include "TextureSampler.h"
#include "GpuBuffer.h"
#include "Serializer.h"
#include "StringUtil.h"

namespace Fancy { namespace Rendering {
  namespace Internal
  {
    String getShaderStageName(uint32 aShaderStage)
    {
      static String names[] = {
        "VERTEX",
        "FRAGMENT",
        "GEOMETRY",
        "TESS_HULL",
        "TESS_DOMAIN",
        "COMPUTE"
      };

      static_assert(_countof(names) == (uint32)ShaderStage::NUM, "Missing names");
      return names[aShaderStage];
    }
  }
  
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  MaterialPass::MaterialPass() : 
    m_pBlendState(nullptr),
    m_pDepthStencilState(nullptr),
    m_eFillMode(FillMode::SOLID),
    m_eCullMode(CullMode::BACK),
    m_eWindingOrder(WindingOrder::CCW)
  {
    memset(m_pGpuProgram, 0x0, sizeof(m_pGpuProgram));
  }
//---------------------------------------------------------------------------//
  MaterialPass::~MaterialPass()
  {
    for (uint32 i = 0; i < m_vpMaterialPassInstances.size(); ++i)
    {
      FANCY_DELETE(m_vpMaterialPassInstances[i], MemoryCategory::MATERIALS);
    }
    m_vpMaterialPassInstances.clear();
  }
//---------------------------------------------------------------------------//
  bool MaterialPass::operator==(const MaterialPass& _other) const
  {
    bool same = true;
    for (uint32 i = 0u; same && i < (uint32)ShaderStage::NUM; ++i)
    {
      same &= m_pGpuProgram[i] == _other.m_pGpuProgram[i];
    }

    same &= m_eFillMode == _other.m_eFillMode;
    same &= m_eCullMode == _other.m_eCullMode;
    same &= m_eWindingOrder == _other.m_eWindingOrder;
    same &= m_pBlendState == _other.m_pBlendState;
    same &= m_pDepthStencilState == _other.m_pDepthStencilState;

    return same;
  }
//---------------------------------------------------------------------------//
  void MaterialPass::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serializeArray(m_pGpuProgram, "m_pGpuProgram");

    aSerializer->serialize(&m_eFillMode, "m_eFillMode");
    aSerializer->serialize(&m_eCullMode, "m_eCullMode");
    aSerializer->serialize(&m_eWindingOrder, "m_eWindingOrder");
    aSerializer->serialize(&m_pBlendState, "m_pBlendState");
    aSerializer->serialize(&m_pDepthStencilState, "m_pDepthStencilState");
  }
//---------------------------------------------------------------------------//
  MaterialPassInstance* MaterialPass::createMaterialPassInstance( const ObjectName& name )
  {
    ASSERT(getMaterialPassInstance(name) == nullptr);
    return createMaterialPassInstance(name, MaterialPassInstance());
  }
//---------------------------------------------------------------------------//
  MaterialPassInstance* MaterialPass::createMaterialPassInstance(const ObjectName& name, const MaterialPassInstance& _template)
  {
    ASSERT(getMaterialPassInstance(name) == nullptr);
    MaterialPassInstance* mpi = FANCY_NEW(MaterialPassInstance(_template), MemoryCategory::MATERIALS);

    mpi->m_Name = name;
    mpi->m_pMaterialPass = this;

    m_vpMaterialPassInstances.push_back(mpi);

    return mpi;
  }
//---------------------------------------------------------------------------//
  MaterialPassInstance* MaterialPass::getMaterialPassInstance(const ObjectName& aName)
  {
    for (MaterialPassInstance* mpi : m_vpMaterialPassInstances)
    {
      if (mpi->m_Name == aName)
      {
        return mpi;
      }
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  MaterialPassInstance* MaterialPass::getMaterialPassInstance(const uint& anMpiHash)
  {
    for (MaterialPassInstance* mpi : m_vpMaterialPassInstances)
    {
      if (mpi->computeHash() == anMpiHash)
      {
        return mpi;
      }
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  uint MaterialPassInstance::computeHash() const
  {
    uint hash = 0x0;

    for (uint32 iStage = 0u; iStage < (uint32)ShaderStage::NUM; ++iStage)
    {
      for (uint32 i = 0u; i < Constants::kMaxNumReadTextures; ++i)
      {
        Texture* tex = m_vpReadTextures[iStage][i];
        MathUtil::hash_combine(hash, tex ? tex->getPath().getHash() : 0u);
      }

      for (uint32 i = 0u; i < Constants::kMaxNumWriteTextures; ++i)
      {
        Texture* tex = m_vpWriteTextures[iStage][i];
        MathUtil::hash_combine(hash, tex ? tex->getPath().getHash() : 0u);
      }

      for (uint32 i = 0u; i < Constants::kMaxNumReadBuffers; ++i)
      {
        GpuBuffer* buf = m_vpReadBuffers[iStage][i];
        MathUtil::hash_combine(hash, buf ? buf->getName().getHash() : 0u);
      }

      for (uint32 i = 0u; i < Constants::kMaxNumWriteBuffers; ++i)
      {
        GpuBuffer* buf = m_vpWriteBuffers[iStage][i];
        MathUtil::hash_combine(hash, buf ? buf->getName().getHash() : 0u);
      }

      for (uint32 i = 0u; i < Constants::kMaxNumTextureSamplers; ++i)
      {
        TextureSampler* sampler = m_vpTextureSamplers[iStage][i];
        MathUtil::hash_combine(hash, sampler ? sampler->getName().getHash() : 0u);
      }
    }

    return hash;
  }
//---------------------------------------------------------------------------//
  void MaterialPassInstance::getResourceDesc(MpiResourceType aType, std::vector<ResourceStorageEntry>& someEntries) const
  {
    if (aType == MpiResourceType::ReadTexture)
    {
      for (uint32 iStage = 0u; iStage < (uint32)ShaderStage::NUM; ++iStage)
      {
        for (uint32 i = 0u; i < Constants::kMaxNumReadTextures; ++i)
        {
          if (!m_vpReadTextures[iStage][i])
            continue;

          ResourceStorageEntry entry;
          entry.myShaderStage = iStage;
          entry.myIndex = i;
          entry.myName = m_vpReadTextures[iStage][i]->getPath();
          someEntries.push_back(entry);
        }
      }
    }

    else if (aType == MpiResourceType::WriteTexture)
    {
      for (uint32 iStage = 0u; iStage < (uint32)ShaderStage::NUM; ++iStage)
      {
        for (uint32 i = 0u; i < Constants::kMaxNumWriteTextures; ++i)
        {
          if (!m_vpWriteTextures[iStage][i])
            continue;

          ResourceStorageEntry entry;
          entry.myShaderStage = iStage;
          entry.myIndex = i;
          entry.myName = m_vpWriteTextures[iStage][i]->getPath();
          someEntries.push_back(entry);
        }
      }
    }

    else if (aType == MpiResourceType::ReadBuffer)
    {
      for (uint32 iStage = 0u; iStage < (uint32)ShaderStage::NUM; ++iStage)
      {
        for (uint32 i = 0u; i < Constants::kMaxNumReadBuffers; ++i)
        {
          if (!m_vpReadBuffers[iStage][i])
            continue;

          ResourceStorageEntry entry;
          entry.myShaderStage = iStage;
          entry.myIndex = i;
          entry.myName = m_vpReadBuffers[iStage][i]->getName();
          someEntries.push_back(entry);
        }
      }
    }

    else if (aType == MpiResourceType::WriteBuffer)
    {
      for (uint32 iStage = 0u; iStage < (uint32)ShaderStage::NUM; ++iStage)
      {
        for (uint32 i = 0u; i < Constants::kMaxNumWriteBuffers; ++i)
        {
          if (!m_vpWriteBuffers[iStage][i])
            continue;

          ResourceStorageEntry entry;
          entry.myShaderStage = iStage;
          entry.myIndex = i;
          entry.myName = m_vpWriteBuffers[iStage][i]->getName();
          someEntries.push_back(entry);
        }
      }
    }

    else if (aType == MpiResourceType::TextureSampler)
    {
      for (uint32 iStage = 0u; iStage < (uint32)ShaderStage::NUM; ++iStage)
      {
        for (uint32 i = 0u; i < Constants::kMaxNumTextureSamplers; ++i)
        {
          if (!m_vpTextureSamplers[iStage][i])
            continue;

          ResourceStorageEntry entry;
          entry.myShaderStage = iStage;
          entry.myIndex = i;
          entry.myName = m_vpTextureSamplers[iStage][i]->getName();
          someEntries.push_back(entry);
        }
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
        m_vpReadTextures[entry.myShaderStage][entry.myIndex] = Texture::getByName(entry.myName);
      }
    }

    else if (aType == MpiResourceType::WriteTexture)
    {
      for (const ResourceStorageEntry& entry : someResources)
      {
        m_vpWriteTextures[entry.myShaderStage][entry.myIndex] = Texture::getByName(entry.myName);
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
        m_vpTextureSamplers[entry.myShaderStage][entry.myIndex] = TextureSampler::getByName(entry.myName);
      }
    }
  }
//---------------------------------------------------------------------------//
  void ResourceStorageEntry::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&myShaderStage, "myShaderStage");
    aSerializer->serialize(&myIndex, "myIndex");
    aSerializer->serialize(&myName, "myName");
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
} } // end of namespace Fancy::Rendering