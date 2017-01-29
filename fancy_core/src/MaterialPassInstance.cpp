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
  void MaterialPassInstance::SetFromDescription(const MaterialPassInstanceDesc& aDesc, GraphicsWorld* aWorld)
  {
    m_pMaterialPass = MaterialPass::FindFromDesc(aDesc.myMaterialPass);
    ASSERT(nullptr != m_pMaterialPass);

    for (uint i = 0u; i < Constants::kMaxNumReadTextures; ++i)
      if (aDesc.myReadTextures[i].myIsExternalTexture)
        m_vpReadTextures[i] = RenderCore::CreateTexture(aDesc.myReadTextures[i].mySourcePath);
      else
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
} }
