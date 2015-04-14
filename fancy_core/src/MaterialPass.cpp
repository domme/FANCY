#include "MaterialPass.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
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
  MaterialPassInstance* MaterialPass::createMaterialPassInstance( const ObjectName& name )
  {
    return createMaterialPassInstance(name, MaterialPassInstance());
  }
//---------------------------------------------------------------------------//
  MaterialPassInstance* MaterialPass::createMaterialPassInstance(const ObjectName& name, const MaterialPassInstance& _template)
  {
    MaterialPassInstance* mpi = FANCY_NEW(MaterialPassInstance(_template), MemoryCategory::MATERIALS);

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
  uint MaterialPassInstance::computeHash() const
  {
    uint hash = 0x0;

    for (uint32 iStage = 0u; iStage < (uint32)ShaderStage::NUM; ++iStage)
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

    MathUtil::hash_combine(hash, reinterpret_cast<uint>(m_pMaterialPass));

    return hash;
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