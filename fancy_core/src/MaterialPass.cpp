#include "MaterialPass.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  MaterialPassDescription::MaterialPassDescription() :
    eFillMode(FillMode::SOLID),
    eCullMode(CullMode::BACK),
    eWindingOrder(WindingOrder::CCW),
    blendState(_N(BlendState_Solid)),
    depthStencilState(_N(DepthStencilState_DefaultDepthState))
  {
    
  }
//---------------------------------------------------------------------------//
  uint MaterialPassDescription::getHash() const
  {
    // The name is irrelevant for the hash
    uint hash = 0x0;

    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      MathUtil::hash_combine(hash, gpuProgram[i].getHash());
    }

    MathUtil::hash_combine(hash, (uint32)eFillMode);
    MathUtil::hash_combine(hash, (uint32)eCullMode);
    MathUtil::hash_combine(hash, (uint32)eWindingOrder);
    MathUtil::hash_combine(hash, (uint32)blendState);
    MathUtil::hash_combine(hash, (uint32)depthStencilState);

    return hash;
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
  void MaterialPass::init(const MaterialPassDescription& _desc)
  {
    m_Name = _desc.name;
    for (uint32 i = 0u; i < (uint32) ShaderStage::NUM; ++i)
    {
      m_pGpuProgram[i] = GpuProgram::getByName(_desc.gpuProgram[i]);
    }
    m_eFillMode = _desc.eFillMode;
    m_eCullMode = _desc.eCullMode;
    m_eWindingOrder = _desc.eWindingOrder;
    m_pBlendState = BlendState::getByName(_desc.blendState);
    m_pDepthStencilState = DepthStencilState::getByName(_desc.depthStencilState);
  }
//---------------------------------------------------------------------------//
  MaterialPassDescription MaterialPass::getDescription() const
  {
    MaterialPassDescription desc;
    desc.name = m_Name;
    for (uint32 i = 0u; i < (uint32) ShaderStage::NUM; ++i)
    {
      desc.gpuProgram[i] = m_pGpuProgram[i] == nullptr ? ObjectName::blank : m_pGpuProgram[i]->getName();
    }
    desc.eFillMode = m_eFillMode;
    desc.blendState = m_pBlendState->getName();
    desc.depthStencilState = m_pDepthStencilState->getName();
    desc.eWindingOrder = m_eWindingOrder;
    desc.eCullMode = m_eCullMode;

    return desc;
  }
//---------------------------------------------------------------------------//
  void MaterialPassInstance::copyFrom( const MaterialPassInstance& _other )
  {
    memcpy(m_vpReadTextures, _other.m_vpReadTextures, sizeof(m_vpReadTextures));
    memcpy(m_vpWriteTextures, _other.m_vpWriteTextures, sizeof(m_vpWriteTextures));
    memcpy(m_vpReadBuffers, _other.m_vpReadBuffers, sizeof(m_vpReadBuffers));
    memcpy(m_vpWriteBuffers, _other.m_vpWriteBuffers, sizeof(m_vpWriteBuffers));
    memcpy(m_vpTextureSamplers, _other.m_vpTextureSamplers, sizeof(m_vpTextureSamplers));
  }
//---------------------------------------------------------------------------//


//---------------------------------------------------------------------------//
  MaterialPassInstance* MaterialPass::createMaterialPassInstance( const ObjectName& name )
  {
    MaterialPassInstance* mpi = FANCY_NEW(MaterialPassInstance, MemoryCategory::MATERIALS);

    mpi->m_Name = name;
    mpi->m_pMaterialPass = this;
    m_vpMaterialPassInstances.push_back(mpi);
    
    return mpi;
  }
//---------------------------------------------------------------------------//
  const MaterialPassInstance* MaterialPass::getMaterialPassInstance( uint _resourceHash ) const
  {
    for (uint32 i = 0u; i < m_vpMaterialPassInstances.size(); ++i)
    {
      if (m_vpMaterialPassInstances[i]->computeHash() == _resourceHash)
      {
        return m_vpMaterialPassInstances[i];
      }
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
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
  uint MaterialPassInstance::computeHash() const
  {
    uint hash = 0x0;

    for (uint32 iStage = 0u; iStage < (uint32) ShaderStage::NUM; ++iStage)
    {
      const Texture* const* ppReadTextures = m_vpReadTextures[iStage];
      for (uint32 i = 0u; i < kMaxNumReadTextures; ++i)
      {
        MathUtil::hash_combine(hash, reinterpret_cast<uint>(ppReadTextures[i]));
      }

      const Texture* const* ppWriteTextures = m_vpWriteTextures[iStage];
      for (uint32 i = 0u; i < kMaxNumWriteTextures; ++i)
      {
        MathUtil::hash_combine(hash, reinterpret_cast<uint>(ppWriteTextures[i]));
      }

      const GpuBuffer* const* ppReadBuffers = m_vpReadBuffers[iStage];
      for (uint32 i = 0u; i < kMaxNumReadBuffers; ++i)
      {
        MathUtil::hash_combine(hash, reinterpret_cast<uint>(ppReadBuffers[i]));
      }

      const GpuBuffer* const* ppWriteBuffers = m_vpWriteBuffers[iStage];
      for (uint32 i = 0u; i < kMaxNumWriteBuffers; ++i)
      {
        MathUtil::hash_combine(hash, reinterpret_cast<uint>(ppWriteBuffers[i]));
      }

      const TextureSampler* const* ppTextureSamplers = m_vpTextureSamplers[iStage];
      for (uint32 i = 0u; i < kMaxNumTextureSamplers; ++i)
      {
        MathUtil::hash_combine(hash, reinterpret_cast<uint>(ppTextureSamplers[i]));
      }
    }

    return hash;
  }
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