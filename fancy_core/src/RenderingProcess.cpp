#include "RenderingProcess.h"
#include "Renderer.h"
#include "MaterialPass.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  RenderingProcess::RenderingProcess()
  {
    
  }
//---------------------------------------------------------------------------//
  RenderingProcess::~RenderingProcess()
  {

  }
//---------------------------------------------------------------------------//
  void RenderingProcess::ApplyMaterialPass(const MaterialPass* _pMaterialPass, RenderContext* aRenderContext)
  {
    aRenderContext->setBlendState(*_pMaterialPass->getBlendState());
    aRenderContext->setDepthStencilState(*_pMaterialPass->getDepthStencilState());

    aRenderContext->setCullMode(_pMaterialPass->getCullMode());
    aRenderContext->setFillMode(_pMaterialPass->getFillMode());
    aRenderContext->setWindingOrder(_pMaterialPass->getWindingOrder());

    aRenderContext->SetGpuProgramPipeline(_pMaterialPass->myProgramPipeline);
  }

//---------------------------------------------------------------------------//    
} }  // end of namespace Fancy::Rendering