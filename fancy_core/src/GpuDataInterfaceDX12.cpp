#include "RendererPrerequisites.h"

#if defined (RENDERER_DX12)
#include "GpuDataInterfaceDX12.h"
#include "MaterialPass.h"
#include "Renderer.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  GpuDataInterfaceDX12::GpuDataInterfaceDX12()
  {
    
  }
//---------------------------------------------------------------------------//
  void GpuDataInterfaceDX12::applyMaterialPassInstance(const MaterialPassInstance* _pMaterialPassInstance, Renderer* _pRenderer)
  {
    // TODO: [Tricky] Depending on the currently active signature, bind the resources to the commandList via the provided Renderer
    //                For descriptor-tables, we need the correct DescriptorHandle+Range from somewhere...
  }
//---------------------------------------------------------------------------//
  void GpuDataInterfaceDX12::applyMaterialPass(const MaterialPass* _pMaterialPass, Renderer* _pRenderer)
  {
    _pRenderer->setBlendState(*_pMaterialPass->getBlendState());
    _pRenderer->setDepthStencilState(*_pMaterialPass->getDepthStencilState());

    _pRenderer->setCullMode(_pMaterialPass->getCullMode());
    _pRenderer->setFillMode(_pMaterialPass->getFillMode());
    _pRenderer->setWindingOrder(_pMaterialPass->getWindingOrder());

    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      ShaderStage eStage = static_cast<ShaderStage>(i);
      _pRenderer->setGpuProgram(_pMaterialPass->getGpuProgram(eStage), eStage);
    }
  }
//---------------------------------------------------------------------------//
} } }
#endif  // RENDERER_DX12