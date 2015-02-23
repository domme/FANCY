#include "MaterialPass.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  MaterialPassDescription::MaterialPassDescription() :
    eFillMode(FillMode::SOLID),
    eCullMode(CullMode::BACK),
    eWindingOrder(WindingOrder::CCW),
    blendState(_N(BlendState_Solid)),
    depthStencilState(_N(DepthStencilState_DefaultDepthState))
  {
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  MaterialPass::MaterialPass() : 
    m_pBlendState(nullptr),
    m_pDepthStencilState(nullptr),
    m_eFillMode(FillMode::SOLID),
    m_eCullMode(CullMode::BACK),
    m_eWindingOrder(WindingOrder::CCW)
  {
    memset(m_pGpuProgram, 0x0, sizeof(m_pGpuProgram));
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
  void MaterialPass::init(const MaterialPassDescription& _desc)
  {
    m_Name = _desc.name;
    for (uint32 i = 0u; i < (uint32) ShaderStage::NUM; ++i)
    {
      m_pGpuProgram[i] = GpuProgram::getByName(_desc.gpuProgram[i]);
    }
    m_eFillMode = _desc.eFillMode;
    m_eCullMode = _desc.eCullMode;
    m_eWindingOrder = _desc.eWindingOrder;
    m_pBlendState = BlendState::getByName(_desc.blendState);
    m_pDepthStencilState = DepthStencilState::getByName(_desc.depthStencilState);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  MaterialPassInstance* MaterialPass::createMaterialPassInstance( const ObjectName& name )
  {
    MaterialPassInstance* mpi = FANCY_NEW(MaterialPassInstance, MemoryCategory::MATERIALS);

    mpi->m_Name = name;
    mpi->m_pMaterialPass = this;
    m_vpMaterialPassInstances.push_back(mpi);
    
    return mpi;
  }
//---------------------------------------------------------------------------//
  MaterialPassInstance::MaterialPassInstance() :
    m_pMaterialPass(nullptr)
  {

  }
 //---------------------------------------------------------------------------//
  MaterialPassInstance::~MaterialPassInstance()
  {
    
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering