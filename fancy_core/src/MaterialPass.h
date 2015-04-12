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
  struct MaterialPassDescription
  {
    MaterialPassDescription();
    uint getHash() const;
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
  class MaterialPass : public StaticManagedHeapObject<MaterialPass>
  {
    public:
      MaterialPass();
      ~MaterialPass();
      void init (const MaterialPassDescription& _desc);
      MaterialPassDescription getDescription() const;

      const ObjectName& getName() const { return m_Name; }
      const GpuProgram* getGpuProgram(const ShaderStage eShaderStage) const 
        {return m_pGpuProgram[(uint32) eShaderStage];}

      FillMode getFillMode() const {return m_eFillMode;}
      CullMode getCullMode() const {return m_eCullMode;}
      WindingOrder getWindingOrder() const {return m_eWindingOrder;}
      const BlendState* getBlendState() const {return m_pBlendState;}
      const DepthStencilState* getDepthStencilState() const {return m_pDepthStencilState;}
      MaterialPassInstance* createMaterialPassInstance(const ObjectName& name);
      const MaterialPassInstance* getMaterialPassInstance(uint _resourceHash) const;
      bool hasStage(ShaderStage _eStage) const {return getGpuProgram(_eStage) != nullptr;}
      
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
//---------------------------------------------------------------------------//

  //TODO: Implement these two... basically, texture- and bufferDescs will contain the actual data. That way, we don't need to load the original file from disk again
  class TextureDesc{};
  class GpuBufferDesc {};

  struct MaterialPassInstanceDesc
  {
    ObjectName name;
    MaterialPassDescription materialPassDesc;
    TextureDesc readTextureDescs[(uint32)ShaderStage::NUM][kMaxNumReadTextures];
    TextureDesc writeTextureDescs[(uint32)ShaderStage::NUM][kMaxNumWriteTextures];
    GpuBufferDesc readBufferDescs[(uint32)ShaderStage::NUM][kMaxNumReadBuffers];
    GpuBufferDesc writeBufferDescs[(uint32)ShaderStage::NUM][kMaxNumWriteBuffers];
    ObjectName textureSamplers[(uint32)ShaderStage::NUM][kMaxNumTextureSamplers];
  };
//---------------------------------------------------------------------------//
  class MaterialPassInstance
  {
    friend class MaterialPass;

    public:
      MaterialPassInstance();
      ~MaterialPassInstance();

      void copyFrom(const MaterialPassInstance& _other);

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