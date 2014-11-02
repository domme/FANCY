#ifndef INCLUDE_MATERIALPASS_H
#define INCLUDE_MATERIALPASS_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "FixedArray.h"
#include "ObjectName.h"
#include "GpuProgram.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  typedef FixedArray<Texture*, kMaxNumReadTextures> ReadTextureList;
  typedef FixedArray<Texture*, kMaxNumWriteTextures> WriteTextureList;
  typedef FixedArray<GpuBuffer*, kMaxNumReadBuffers> ReadBufferList;
  typedef FixedArray<GpuBuffer*, kMaxNumWriteBuffers> WriteBufferList;
  typedef FixedArray<TextureSampler*, kMaxNumReadTextures> TextureSamplerList;
//---------------------------------------------------------------------------//
  struct MaterialPassData
  {
    ReadTextureList readTextures[(uint32) ShaderStage::NUM];
    WriteTextureList writeTextures[(uint32) ShaderStage::NUM];
    ReadBufferList readBuffers[(uint32) ShaderStage::NUM];
    WriteBufferList writeBuffers[(uint32) ShaderStage::NUM];
    TextureSamplerList textureSamplers[(uint32) ShaderStage::NUM];
  };
//---------------------------------------------------------------------------//
  class MaterialPass
  {
    public:
      MaterialPass();
      ~MaterialPass();

      const ObjectName& getName() const { return m_Name; }
      const MaterialPassData* getMaterialPassData() const {return m_pData;}
      void setMaterialPassData(const MaterialPassData* pData);
      const GpuProgram* getGpuProgram(const ShaderStage eShaderStage) const {return m_pGpuProgram[(uint32) eShaderStage];}
      FillMode getFillMode() const {return m_eFillMode;}
      CullMode getCullMode() const {return m_eCullMode;}
      WindingOrder getWindingOrder() const {return m_eWindingOrder;}
      const BlendState* getBlendState() const {return m_pBlendState;}
      const DepthStencilState* getDepthStencilState() const {return m_pDepthStencilState;}

    private:
      ObjectName m_Name;
      GpuProgram* m_pGpuProgram[(uint32)ShaderStage::NUM];

      FillMode m_eFillMode;
      CullMode m_eCullMode;
      WindingOrder m_eWindingOrder;
      BlendState* m_pBlendState;
      DepthStencilState* m_pDepthStencilState;

      const MaterialPassData* m_pData;
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIALPASS_H