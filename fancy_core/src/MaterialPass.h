#ifndef INCLUDE_MATERIALPASS_H
#define INCLUDE_MATERIALPASS_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "FixedArray.h"
#include "ObjectName.h"
#include "GpuProgram.h"
#include "DepthStencilState.h"
#include "BlendState.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
  struct MaterialPassDescription
  {
    MaterialPassDescription();
    // TODO: Change these values to ObjectNames and grab them from the managers after a global init
    ObjectName name;
    ObjectName gpuProgram[(uint32)ShaderStage::NUM];
    FillMode eFillMode;
    CullMode eCullMode;
    WindingOrder eWindingOrder;
    ObjectName blendState;
    ObjectName depthStencilState;
  };
//---------------------------------------------------------------------------//
  class MaterialPass
  {
    public:
      MaterialPass();
      ~MaterialPass();
      void init (const MaterialPassDescription& _desc);

      const ObjectName& getName() const { return m_Name; }
      const GpuProgram* getGpuProgram(const ShaderStage eShaderStage) const 
        {return m_pGpuProgram[(uint32) eShaderStage];}

      FillMode getFillMode() const {return m_eFillMode;}
      CullMode getCullMode() const {return m_eCullMode;}
      WindingOrder getWindingOrder() const {return m_eWindingOrder;}
      const BlendState* getBlendState() const {return m_pBlendState;}
      const DepthStencilState* getDepthStencilState() const {return m_pDepthStencilState;}
      MaterialPassInstance* createMaterialPassInstance(const ObjectName& name);

    private:
      ObjectName m_Name;
      const GpuProgram* m_pGpuProgram[(uint32)ShaderStage::NUM];
      std::vector<MaterialPassInstance*> m_vpMaterialPassInstances;

      FillMode m_eFillMode;
      CullMode m_eCullMode;
      WindingOrder m_eWindingOrder;
      const BlendState* m_pBlendState;
      const DepthStencilState* m_pDepthStencilState;
  };
//---------------------------------------------------------------------------//
  typedef FixedArray<Texture*, kMaxNumReadTextures> ReadTextureList;
  typedef FixedArray<Texture*, kMaxNumWriteTextures> WriteTextureList;
  typedef FixedArray<GpuBuffer*, kMaxNumReadBuffers> ReadBufferList;
  typedef FixedArray<GpuBuffer*, kMaxNumWriteBuffers> WriteBufferList;
  typedef FixedArray<TextureSampler*, kMaxNumReadTextures> TextureSamplerList;
//---------------------------------------------------------------------------//
  class MaterialPassInstance
  {
    friend class MaterialPass;

    public:
      ~MaterialPassInstance();

      const ReadTextureList& getReadTextures(ShaderStage eShaderStage) const {return m_vpReadTextures[(uint32) eShaderStage];}
      const WriteTextureList& getWriteTextures(ShaderStage eShaderStage) const {return m_vpWriteTextures[(uint32) eShaderStage];}
      const ReadBufferList& getReadBuffers(ShaderStage eShaderStage) const {return m_vpReadBuffers[(uint32) eShaderStage];}
      const WriteBufferList& getWriteBuffers(ShaderStage eShaderStage) const {return m_vpWriteBuffers[(uint32) eShaderStage];}
      const TextureSamplerList& getTextureSamplers(ShaderStage eShaderStage) const {return m_vpTextureSamplers[(uint32) eShaderStage];}

      const MaterialPass* getMaterialPass() const {return m_pMaterialPass;}
      const ObjectName& getName() {return m_Name;}

    private:
      MaterialPassInstance();

      ReadTextureList m_vpReadTextures[(uint32) ShaderStage::NUM];
      WriteTextureList m_vpWriteTextures[(uint32) ShaderStage::NUM];
      ReadBufferList m_vpReadBuffers[(uint32) ShaderStage::NUM];
      WriteBufferList m_vpWriteBuffers[(uint32) ShaderStage::NUM];
      TextureSamplerList m_vpTextureSamplers[(uint32) ShaderStage::NUM];

      MaterialPass* m_pMaterialPass;
      ObjectName m_Name;
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIALPASS_H