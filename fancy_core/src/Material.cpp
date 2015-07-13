#include "Material.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  namespace Internal
  {
    String getParameterSemanticName(uint32 aSemantic)
    {
      static String names[] = {
        "DIFFUSE_REFLECTIVITY",
        "SPECULAR_REFLECTIVITY",
        "SPECULAR_POWER",
        "OPACITY"
      };

      static_assert(_countof(names) == (uint32)EMaterialParameterSemantic::NUM, "Missing names");

      return names[(uint32)aSemantic];
    }
  //---------------------------------------------------------------------------//
    String getMaterialPassName(uint32 aPass)
    {
      static String names[] = {
        "SOLID_GBUFFER",
        "SOLID_FORWARD",
        "TRANSPARENT_FORWARD"
      };

      static_assert(_countof(names) == (uint32)EMaterialPass::NUM, "Missing names");
      return names[(uint32)aPass];
    }
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  Material::Material()
  {
    memset(m_vPasses, 0u, sizeof(m_vPasses));
    memset(m_vParameters, 0x0, sizeof(m_vParameters));
  }
//---------------------------------------------------------------------------//
  Material::~Material()
  {

  }
//---------------------------------------------------------------------------//
  bool Material::operator==(const Material& _other) const
  {
    bool same = memcmp(m_vPasses, _other.m_vPasses, sizeof(m_vPasses)) == 0;

    if (same)
    {
      for (uint32 i = 0u; i < (uint32)EMaterialParameterSemantic::NUM; ++i)
      {
        same &= glm::abs(m_vParameters[i] - _other.m_vParameters[i]) < 1e-3;
      }
    }

    return same;
  }
//---------------------------------------------------------------------------//
  void Material::serialize(IO::Serializer& aSerializer)
  {
    /*
    aSerializer.serialize(_VAL(m_Name));

    uint32 count = aSerializer.beginArray("m_vParameters", (uint32)EMaterialParameterSemantic::NUM);
    for (uint32 i = 0u; i < (uint32)EMaterialParameterSemantic::NUM; ++i)
      aSerializer.serialize(m_vParameters[i]);
    aSerializer.endArray();

    count = aSerializer.beginArray("m_vPasses", (uint32)EMaterialPass::NUM);
    for (uint32 i = 0u; i < (uint32)EMaterialPass::NUM; ++i)
      aSerializer.serialize(m_vPasses[i]);
    aSerializer.endArray();
    */
  }
//---------------------------------------------------------------------------//
  MaterialDesc Material::getDescription() const
  {
    MaterialDesc aDesc;
    aDesc.myName = m_Name;
    memcpy(aDesc.myParameters, m_vParameters, sizeof(m_vParameters));

    for (uint32 i = 0u; i < (uint32)EMaterialPass::NUM; ++i)
    {
      if (m_vPasses[i])
      {
        aDesc.myPasses[i] = m_vPasses[i]->getDescription();
      }
    }

    return aDesc;
  }
//---------------------------------------------------------------------------//
  void Material::initFromDescription(const MaterialDesc& _aDesc)
  {
    m_Name = _aDesc.myName;
    memcpy(m_vParameters, _aDesc.myParameters, sizeof(m_vParameters));

    for (uint32 i = 0u; i < (uint32)EMaterialPass::NUM; ++i)
    {
      const MaterialPassInstanceDesc& mpiDesc = _aDesc.myPasses[i];
      if (mpiDesc.myName == ObjectName::blank)
      {
        continue;
      }

      // Is the referenced MaterialPass already initialized?
      MaterialPass* matPass = MaterialPass::find([mpiDesc](const MaterialPass* currMatPass) -> bool {
        return currMatPass->getDescription() == mpiDesc.myMaterialPass;
      });

      if (matPass == nullptr)
      {
        // There isn't a fitting material pass yet, so we'll create one in-place

        // TODO: Make StaticManagedHeapObject act as factory for their managed classes...
        matPass = FANCY_NEW(MaterialPass, MemoryCategory::MATERIALS);
        matPass->initFromDescription(mpiDesc.myMaterialPass);
        MaterialPass::registerWithName(matPass);
      }

      MaterialPassInstance* mpi = matPass->getMaterialPassInstance(mpiDesc.myName);
      if (mpi == nullptr)
      {
        mpi = matPass->createMaterialPassInstance(mpiDesc.myName);
        mpi->initFromDescription(mpiDesc);
      }

      m_vPasses[i] = mpi;
    }
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering