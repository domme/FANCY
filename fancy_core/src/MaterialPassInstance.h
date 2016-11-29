#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "MaterialPassInstanceDesc.h"

#include "Serializable.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class MpiResourceType
  {
    ReadTexture,
    WriteTexture,
    ReadBuffer,
    WriteBuffer,
    TextureSampler
  };
//---------------------------------------------------------------------------//
  struct ResourceStorageEntry
  {
    SERIALIZABLE(ResourceStorageEntry)

    uint32 myIndex;
    uint64 myHash;

    void Serialize(IO::Serializer* aSerializer);
    ObjectName getTypeName() const { return _N(ResourceStorageEntry); }
    uint64 GetHash() const { return 0u; }
  };
//---------------------------------------------------------------------------//
  class MaterialPassInstance : public StaticManagedHeapObject<MaterialPassInstance>
  {
    friend class MaterialPass;

  public:
    SERIALIZABLE(MaterialPassInstance)

    MaterialPassInstance();
    ~MaterialPassInstance();
    bool operator==(const MaterialPassInstanceDesc& aDesc) const;

    MaterialPassInstanceDesc GetDescription() const;
    void SetFromDescription(const MaterialPassInstanceDesc& aDesc);

    uint64 GetHash() const { return GetDescription().GetHash(); }
    void Serialize(IO::Serializer* aSerializer);
    static ObjectName getTypeName() { return _N(MaterialPassInstance); }

    const SharedPtr<Texture>* getReadTextures() const { return m_vpReadTextures; }
    const SharedPtr<Texture>* getWriteTextures() const { return m_vpWriteTextures; }
    const GpuBuffer* const* getReadBuffers() const { return m_vpReadBuffers; }
    const GpuBuffer* const* getWriteBuffers() const { return m_vpWriteBuffers; }
    const TextureSampler* const* getTextureSamplers() const { return m_vpTextureSamplers; }

    // Note: Unfortunately, we can't reflect binding points from OpenGL-shaders and we don't want to modify binding in app code...
    // void setReadTexture(ShaderStage _eStage, const ObjectName& _name, const Texture* _pTexture);
    // void setWriteTexture(ShaderStage _eStage, const ObjectName& _name, const Texture* _pTexture);
    // void setReadBuffer(ShaderStage _eStage, const ObjectName& _name, const GpuBuffer* _pBuffer);
    // void setWriteBuffer(ShaderStage _eStage, const ObjectName& _name, const GpuBuffer* _pBuffer);
    // void setTextureSampler(ShaderStage _eStage, const ObjectName& _name, const TextureSampler* _pTextureSampler);

    void setReadTexture(uint32 aRegisterIndex, const SharedPtr<Texture>& aTexture) { ASSERT(aRegisterIndex < Constants::kMaxNumReadTextures); m_vpReadTextures[aRegisterIndex] = aTexture; }
    void setWriteTexture(uint32 aRegisterIndex, const SharedPtr<Texture>& aTexture) { ASSERT(aRegisterIndex < Constants::kMaxNumWriteTextures); m_vpWriteTextures[aRegisterIndex] = aTexture; }
    void setReadBuffer(uint32 aRegisterIndex, GpuBuffer* aBuffer) { ASSERT(aRegisterIndex < Constants::kMaxNumReadBuffers); m_vpReadBuffers[aRegisterIndex] = aBuffer; }
    void setWriteBuffer(uint32 aRegisterIndex, GpuBuffer* aBuffer) { ASSERT(aRegisterIndex < Constants::kMaxNumWriteBuffers); m_vpWriteBuffers[aRegisterIndex] = aBuffer; }
    void setTextureSampler(uint32 aRegisterIndex, TextureSampler* aSampler) { ASSERT(aRegisterIndex < Constants::kMaxNumTextureSamplers); m_vpTextureSamplers[aRegisterIndex] = aSampler; }

    void getResourceDesc(MpiResourceType aType, std::vector<ResourceStorageEntry>& someEntries) const;
    void setFromResourceDesc(const std::vector<ResourceStorageEntry>& someResources, MpiResourceType aType);

    MaterialPass* getMaterialPass() const { return m_pMaterialPass; }

  private:
    MaterialPass* m_pMaterialPass;

    // TODO: Make these FixedArrays
    SharedPtr<Texture> m_vpReadTextures[Constants::kMaxNumReadTextures];
    SharedPtr<Texture> m_vpWriteTextures[Constants::kMaxNumWriteTextures];
    GpuBuffer* m_vpReadBuffers[Constants::kMaxNumReadBuffers];
    GpuBuffer* m_vpWriteBuffers[Constants::kMaxNumWriteBuffers];
    TextureSampler* m_vpTextureSamplers[Constants::kMaxNumTextureSamplers];
  };
//---------------------------------------------------------------------------//
} }