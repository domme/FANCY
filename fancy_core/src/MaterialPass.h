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

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct MaterialPassDesc
  {
    bool operator==(const MaterialPassDesc& anOther) const;
    ObjectName myName;
    ObjectName myGpuPrograms[(uint32)ShaderStage::NUM];
    FillMode myFillmode;
    CullMode myCullmode;
    WindingOrder myWindingOrder;
    ObjectName myBlendState;
    ObjectName myDepthStencilState;
  };
//---------------------------------------------------------------------------//
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
  class MaterialPass : public StaticManagedHeapObject<MaterialPass>
  {
    friend class MaterialPassInstance;

    public:
      MaterialPass();
      ~MaterialPass();
      bool operator==(const MaterialPass& _other) const;

      void serialize(IO::Serializer* aSerializer);
      ObjectName getTypeName() const { return _N(MaterialPass); }

      MaterialPassDesc getDescription() const;
      void initFromDescription(const MaterialPassDesc& _aDesc);

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
      MaterialPassInstance* getMaterialPassInstance(const ObjectName& aName);
      MaterialPassInstance* getMaterialPassInstance(const uint& anMpiHash);
      bool hasStage(ShaderStage _eStage) const {return getGpuProgram(_eStage) != nullptr;}

      ObjectName m_Name;
      FillMode m_eFillMode;
      CullMode m_eCullMode;
      WindingOrder m_eWindingOrder;
      BlendState* m_pBlendState;
      DepthStencilState* m_pDepthStencilState;
      GpuProgram* m_pGpuProgram[(uint32)ShaderStage::NUM];

    private:
      std::vector<MaterialPassInstance*> m_vpMaterialPassInstances;

  };
//---------------------------------------------------------------------------//
  struct MaterialPassInstanceDesc
  {
    ObjectName myName;
    ObjectName myReadTextures[(uint32)ShaderStage::NUM][kMaxNumReadTextures];
    ObjectName myWriteTextures[(uint32)ShaderStage::NUM][kMaxNumWriteTextures];
    ObjectName myReadBuffers[(uint32)ShaderStage::NUM][kMaxNumReadBuffers];
    ObjectName myWriteBuffers[(uint32)ShaderStage::NUM][kMaxNumWriteBuffers];
    ObjectName myTextureSamplers[(uint32)ShaderStage::NUM][kMaxNumTextureSamplers];
    MaterialPassDesc myMaterialPass;
  };
//---------------------------------------------------------------------------//
  struct TextureStorageEntry
  {
    uint32 myShaderStage;
    uint32 myIndex;
    String myName;
    void serialize(IO::Serializer* aSerializer);
  };
//---------------------------------------------------------------------------//
  class MaterialPassInstance
  {
    friend class MaterialPass;

    public:
      MaterialPassInstance();
      ~MaterialPassInstance();

      void serialize(IO::Serializer* aSerializer);
      ObjectName getTypeName() const { return _N(MaterialPassInstance); }

      void initFromDescription(const MaterialPassInstanceDesc _aDesc);
      MaterialPassInstanceDesc getDescription() const;

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

      void setReadTexture(ShaderStage _eStage, uint32 _registerIndex, Texture* _pTexture) {ASSERT(_registerIndex < kMaxNumReadTextures); m_vpReadTextures[(uint32) _eStage][_registerIndex] = _pTexture; }
      void setWriteTexture(ShaderStage _eStage, uint32 _registerIndex, Texture* _pTexture) {ASSERT(_registerIndex < kMaxNumWriteTextures); m_vpWriteTextures[(uint32) _eStage][_registerIndex] = _pTexture; }
      void setReadBuffer(ShaderStage _eStage, uint32 _registerIndex, GpuBuffer* _pBuffer) {ASSERT(_registerIndex < kMaxNumReadBuffers); m_vpReadBuffers[(uint32) _eStage][_registerIndex] = _pBuffer; }
      void setWriteBuffer(ShaderStage _eStage, uint32 _registerIndex, GpuBuffer* _pBuffer) {ASSERT(_registerIndex < kMaxNumWriteBuffers); m_vpWriteBuffers[(uint32) _eStage][_registerIndex] = _pBuffer; }
      void setTextureSampler(ShaderStage _eStage, uint32 _registerIndex, TextureSampler* _pTextureSampler) {ASSERT(_registerIndex < kMaxNumTextureSamplers); m_vpTextureSamplers[(uint32) _eStage][_registerIndex] = _pTextureSampler; }

      MaterialPass* getMaterialPass() const {return m_pMaterialPass;}
      const ObjectName& getName() {return m_Name;}

      uint computeHash() const;

    private:
      ObjectName m_Name;
      MaterialPass* m_pMaterialPass;
      
      Texture* m_vpReadTextures[(uint32) ShaderStage::NUM][kMaxNumReadTextures];
      Texture* m_vpWriteTextures[(uint32) ShaderStage::NUM][kMaxNumWriteTextures];
      GpuBuffer* m_vpReadBuffers[(uint32) ShaderStage::NUM][kMaxNumReadBuffers];
      GpuBuffer* m_vpWriteBuffers[(uint32) ShaderStage::NUM][kMaxNumWriteBuffers];
      TextureSampler* m_vpTextureSamplers[(uint32) ShaderStage::NUM][kMaxNumTextureSamplers];
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIALPASS_H