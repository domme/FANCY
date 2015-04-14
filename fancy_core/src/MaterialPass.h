#ifndef INCLUDE_MATERIALPASS_H
#define INCLUDE_MATERIALPASS_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "StaticManagedObject.h"

#include "FixedArray.h"
#include "ObjectName.h"
#include "GpuProgram.h"
#include "DepthStencilState.h"
#include "BlendState.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  class MaterialPass : public StaticManagedHeapObject<MaterialPass>
  {
    public:
      MaterialPass();
      ~MaterialPass();
      bool operator==(const MaterialPass& _other) const;

      const ObjectName& getName() const { return m_Name; }
      const GpuProgram* getGpuProgram(const ShaderStage eShaderStage) const 
        {return m_pGpuProgram[(uint32) eShaderStage];}

      FillMode getFillMode() const {return m_eFillMode;}
      CullMode getCullMode() const {return m_eCullMode;}
      WindingOrder getWindingOrder() const {return m_eWindingOrder;}
      const BlendState* getBlendState() const {return m_pBlendState;}
      const DepthStencilState* getDepthStencilState() const {return m_pDepthStencilState;}
      MaterialPassInstance* createMaterialPassInstance(const ObjectName& name);
      MaterialPassInstance* createMaterialPassInstance(const ObjectName& name, const MaterialPassInstance& _template);
      const MaterialPassInstance* getMaterialPassInstance(uint _resourceHash) const;
      bool hasStage(ShaderStage _eStage) const {return getGpuProgram(_eStage) != nullptr;}

      ObjectName m_Name;
      const GpuProgram* m_pGpuProgram[(uint32)ShaderStage::NUM];
      FillMode m_eFillMode;
      CullMode m_eCullMode;
      WindingOrder m_eWindingOrder;
      const BlendState* m_pBlendState;
      const DepthStencilState* m_pDepthStencilState;
      
    private:
      std::vector<MaterialPassInstance*> m_vpMaterialPassInstances;
  };
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  class MaterialPassInstance
  {
    friend class MaterialPass;

    public:
      MaterialPassInstance();
      ~MaterialPassInstance();

      const Texture* const* getReadTextures(ShaderStage eShaderStage) const {return m_vpReadTextures[(uint32) eShaderStage];}
      const Texture* const* getWriteTextures(ShaderStage eShaderStage) const {return m_vpWriteTextures[(uint32) eShaderStage];}
      const GpuBuffer* const* getReadBuffers(ShaderStage eShaderStage) const {return m_vpReadBuffers[(uint32) eShaderStage];}
      const GpuBuffer* const* getWriteBuffers(ShaderStage eShaderStage) const {return m_vpWriteBuffers[(uint32) eShaderStage];}
      const TextureSampler* const* getTextureSamplers(ShaderStage eShaderStage) const {return m_vpTextureSamplers[(uint32) eShaderStage];}

      // Note: Unfortunately, we can't reflect binding points from OpenGL-shaders and we don't want to modify binding in app code...
      // void setReadTexture(ShaderStage _eStage, const ObjectName& _name, const Texture* _pTexture);
      // void setWriteTexture(ShaderStage _eStage, const ObjectName& _name, const Texture* _pTexture);
      // void setReadBuffer(ShaderStage _eStage, const ObjectName& _name, const GpuBuffer* _pBuffer);
      // void setWriteBuffer(ShaderStage _eStage, const ObjectName& _name, const GpuBuffer* _pBuffer);
      // void setTextureSampler(ShaderStage _eStage, const ObjectName& _name, const TextureSampler* _pTextureSampler);

      void setReadTexture(ShaderStage _eStage, uint32 _registerIndex, const Texture* _pTexture) {ASSERT(_registerIndex < kMaxNumReadTextures); m_vpReadTextures[(uint32) _eStage][_registerIndex] = _pTexture; }
      void setWriteTexture(ShaderStage _eStage, uint32 _registerIndex, const Texture* _pTexture) {ASSERT(_registerIndex < kMaxNumWriteTextures); m_vpWriteTextures[(uint32) _eStage][_registerIndex] = _pTexture; }
      void setReadBuffer(ShaderStage _eStage, uint32 _registerIndex, const GpuBuffer* _pBuffer) {ASSERT(_registerIndex < kMaxNumReadBuffers); m_vpReadBuffers[(uint32) _eStage][_registerIndex] = _pBuffer; }
      void setWriteBuffer(ShaderStage _eStage, uint32 _registerIndex, const GpuBuffer* _pBuffer) {ASSERT(_registerIndex < kMaxNumWriteBuffers); m_vpWriteBuffers[(uint32) _eStage][_registerIndex] = _pBuffer; }
      void setTextureSampler(ShaderStage _eStage, uint32 _registerIndex, const TextureSampler* _pTextureSampler) {ASSERT(_registerIndex < kMaxNumTextureSamplers); m_vpTextureSamplers[(uint32) _eStage][_registerIndex] = _pTextureSampler; }

      const MaterialPass* getMaterialPass() const {return m_pMaterialPass;}
      const ObjectName& getName() {return m_Name;}

      uint computeHash() const;

    private:
      const Texture* m_vpReadTextures[(uint32) ShaderStage::NUM][kMaxNumReadTextures];
      const Texture* m_vpWriteTextures[(uint32) ShaderStage::NUM][kMaxNumWriteTextures];
      const GpuBuffer* m_vpReadBuffers[(uint32) ShaderStage::NUM][kMaxNumReadBuffers];
      const GpuBuffer* m_vpWriteBuffers[(uint32) ShaderStage::NUM][kMaxNumWriteBuffers];
      const TextureSampler* m_vpTextureSamplers[(uint32) ShaderStage::NUM][kMaxNumTextureSamplers];

      const MaterialPass* m_pMaterialPass;
      ObjectName m_Name;
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIALPASS_H