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
  MaterialPassProperties::MaterialPassProperties() :
    m_pDepthStencilState(nullptr),
    m_eFillMode(FillMode::SOLID),
    m_eCullMode(CullMode::BACK),
    m_eWindingOrder(WindingOrder::CCW)
  {
    
  }

//---------------------------------------------------------------------------//
  MaterialPass::MaterialPass()   
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
    same &= myProperties.myProgramPipeline->myShaderHash == _other.myProperties.myProgramPipeline->myShaderHash;
    same &= myProperties.m_eFillMode == _other.myProperties.m_eFillMode;
    same &= myProperties.m_eCullMode == _other.myProperties.m_eCullMode;
    same &= myProperties.m_eWindingOrder == _other.myProperties.m_eWindingOrder;
    same &= myProperties.m_pBlendState == _other.myProperties.m_pBlendState;
    same &= myProperties.m_pDepthStencilState == _other.myProperties.m_pDepthStencilState;

    return same;
  }
//---------------------------------------------------------------------------//
  void MaterialPass::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serialize(&myProperties.myProgramPipeline, "myProgramPipeline");
    aSerializer->serialize(&myProperties.m_eFillMode, "m_eFillMode");
    aSerializer->serialize(&myProperties.m_eCullMode, "m_eCullMode");
    aSerializer->serialize(&myProperties.m_eWindingOrder, "m_eWindingOrder");
    aSerializer->serialize(&myProperties.m_pBlendState, "m_pBlendState");
    aSerializer->serialize(&myProperties.m_pDepthStencilState, "m_pDepthStencilState");
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