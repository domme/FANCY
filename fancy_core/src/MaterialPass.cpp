#include "MaterialPass.h"
#include "MathUtil.h"
#include "Texture.h"
#include "TextureSampler.h"
#include "GpuBuffer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  bool MaterialPassDesc::operator==(const MaterialPassDesc& anOther) const
  {
    bool equal = true;
    equal &= myName == anOther.myName;
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      equal &= myGpuPrograms[i] == anOther.myGpuPrograms[i];
    }
    equal &= myFillmode == anOther.myFillmode;
    equal &= myCullmode == anOther.myCullmode;
    equal &= myWindingOrder == anOther.myWindingOrder;
    equal &= myBlendState == anOther.myBlendState;
    equal &= myDepthStencilState == anOther.myDepthStencilState;
    
    return equal;
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
  MaterialPassDesc MaterialPass::getDescription() const
  {
    MaterialPassDesc aDesc;
    aDesc.myName = m_Name;
    aDesc.myBlendState = m_pBlendState ? m_pBlendState->getName() : ObjectName::blank;
    aDesc.myDepthStencilState = m_pDepthStencilState ? m_pDepthStencilState->getName() : ObjectName::blank;
    aDesc.myCullmode = m_eCullMode;
    aDesc.myFillmode = m_eFillMode;
    aDesc.myWindingOrder = m_eWindingOrder;
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      aDesc.myGpuPrograms[i] = m_pGpuProgram[i] ? m_pGpuProgram[i]->getName() : ObjectName::blank;
    }

    return aDesc;
  }
//---------------------------------------------------------------------------//
  void MaterialPass::initFromDescription(const MaterialPassDesc& _aDesc)
  {
    ASSERT(m_Name == ObjectName::blank); // Already initialized

    m_Name = _aDesc.myName;
    m_pBlendState = BlendState::getByName(_aDesc.myBlendState);
    m_pDepthStencilState = DepthStencilState::getByName(_aDesc.myDepthStencilState);
    m_eCullMode = _aDesc.myCullmode;
    m_eFillMode = _aDesc.myFillmode;
    m_eWindingOrder = _aDesc.myWindingOrder;
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      m_pGpuProgram[i] = GpuProgram::getByName(_aDesc.myGpuPrograms[i], true);
    }
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
    MaterialPassInstanceDesc aDesc = getDescription();

    uint hash = 0x0;

    for (uint32 iStage = 0u; iStage < (uint32)ShaderStage::NUM; ++iStage)
    {
      for (uint32 i = 0u; i < kMaxNumReadTextures; ++i)
      {
        MathUtil::hash_combine(hash, aDesc.myReadTextures[iStage][i].getHash());
      }

      for (uint32 i = 0u; i < kMaxNumWriteTextures; ++i)
      {
        MathUtil::hash_combine(hash, aDesc.myWriteTextures[iStage][i].getHash());
      }

      for (uint32 i = 0u; i < kMaxNumReadBuffers; ++i)
      {
        MathUtil::hash_combine(hash, aDesc.myReadBuffers[iStage][i].getHash());
      }

      for (uint32 i = 0u; i < kMaxNumWriteBuffers; ++i)
      {
        MathUtil::hash_combine(hash, aDesc.myWriteBuffers[iStage][i].getHash());
      }

      for (uint32 i = 0u; i < kMaxNumTextureSamplers; ++i)
      {
        MathUtil::hash_combine(hash, aDesc.myTextureSamplers[iStage][i].getHash());
      }
    }

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
  void MaterialPassInstance::initFromDescription(const MaterialPassInstanceDesc _aDesc)
  {
    m_Name = _aDesc.myName;

    for (uint32 iStage = 0u; iStage < (uint32)ShaderStage::NUM; ++iStage)
    {
      for (uint32 i = 0u; i < kMaxNumReadTextures; ++i)
      {
        m_vpReadTextures[iStage][i] = Texture::getByName(_aDesc.myReadTextures[iStage][i], true);
      }

      for (uint32 i = 0u; i < kMaxNumWriteTextures; ++i)
      {
        m_vpWriteTextures[iStage][i] = Texture::getByName(_aDesc.myWriteTextures[iStage][i], true);
      }

      for (uint32 i = 0u; i < kMaxNumReadBuffers; ++i)
      {
        // TODO: Should buffers be loadable?
        m_vpReadBuffers[iStage][i] = nullptr;
      }

      for (uint32 i = 0u; i < kMaxNumWriteBuffers; ++i)
      {
        // TODO: Should buffers be loadable?
        m_vpWriteBuffers[iStage][i] = nullptr;
      }

      for (uint32 i = 0u; i < kMaxNumTextureSamplers; ++i)
      {
        m_vpTextureSamplers[iStage][i] = TextureSampler::getByName(_aDesc.myTextureSamplers[iStage][i], true);
      }
    }
  }
//---------------------------------------------------------------------------//
  MaterialPassInstanceDesc MaterialPassInstance::getDescription() const
  {
    MaterialPassInstanceDesc aDesc;

    aDesc.myName = m_Name;

    for (uint32 iStage = 0u; iStage < (uint32)ShaderStage::NUM; ++iStage)
    {
      for (uint32 i = 0u; i < kMaxNumReadTextures; ++i)
      {
        aDesc.myReadTextures[iStage][i] = m_vpReadTextures[iStage][i] ? m_vpReadTextures[iStage][i]->getPath() : ObjectName::blank;
      }

      for (uint32 i = 0u; i < kMaxNumWriteTextures; ++i)
      {
        aDesc.myWriteTextures[iStage][i] = m_vpWriteTextures[iStage][i] ? m_vpWriteTextures[iStage][i]->getPath() : ObjectName::blank;
      }

      for (uint32 i = 0u; i < kMaxNumReadBuffers; ++i)
      {
        aDesc.myReadBuffers[iStage][i] = m_vpReadBuffers[iStage][i] ? m_vpReadBuffers[iStage][i]->getName() : ObjectName::blank;
      }

      for (uint32 i = 0u; i < kMaxNumWriteBuffers; ++i)
      {
        aDesc.myWriteBuffers[iStage][i] = m_vpWriteBuffers[iStage][i] ? m_vpWriteBuffers[iStage][i]->getName() : ObjectName::blank;
      }

      for (uint32 i = 0u; i < kMaxNumTextureSamplers; ++i)
      {
        aDesc.myTextureSamplers[iStage][i] = m_vpTextureSamplers[iStage][i] ? m_vpTextureSamplers[iStage][i]->getName() : ObjectName::blank;
      }
    }

    return aDesc;
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