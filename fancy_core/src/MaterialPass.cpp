#include "MaterialPass.h"

namespace Fancy { namespace Rendering {
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

  }
//---------------------------------------------------------------------------//
  void MaterialPass::setMaterialPassData( const MaterialPassData* pData )
  {
    m_pData = pData;
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering