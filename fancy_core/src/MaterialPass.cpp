#include "MaterialPass.h"
#include "Serializer.h"
#include "StringUtil.h"
#include "MaterialPassInstance.h"
#include "GpuProgramPipeline.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  String logGetShaderStageName(uint32 aShaderStage)
  {
    static String names[] = {
      "VERTEX",
      "FRAGMENT",
      "GEOMETRY",
      "TESS_HULL",
      "TESS_DOMAIN",
      "COMPUTE"
    };

    static_assert(_countof(names) == (uint32)ShaderStage::NUM, "Missing names");
    return names[aShaderStage];
  }
//---------------------------------------------------------------------------//
  MaterialPass::MaterialPass() : 
    m_pBlendState(nullptr),
    m_pDepthStencilState(nullptr),
    m_eFillMode(FillMode::SOLID),
    m_eCullMode(CullMode::BACK),
    m_eWindingOrder(WindingOrder::CCW)
  {
    
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
    same &= myProgramPipeline->myShaderHash == _other.myProgramPipeline->myShaderHash;
    same &= m_eFillMode == _other.m_eFillMode;
    same &= m_eCullMode == _other.m_eCullMode;
    same &= m_eWindingOrder == _other.m_eWindingOrder;
    same &= m_pBlendState == _other.m_pBlendState;
    same &= m_pDepthStencilState == _other.m_pDepthStencilState;

    return same;
  }
//---------------------------------------------------------------------------//
  void MaterialPass::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serialize(&myProgramPipeline, "myProgramPipeline");
    aSerializer->serialize(&m_eFillMode, "m_eFillMode");
    aSerializer->serialize(&m_eCullMode, "m_eCullMode");
    aSerializer->serialize(&m_eWindingOrder, "m_eWindingOrder");
    aSerializer->serialize(&m_pBlendState, "m_pBlendState");
    aSerializer->serialize(&m_pDepthStencilState, "m_pDepthStencilState");
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
} } // end of namespace Fancy::Rendering