#include "RendererPrerequisites.h"
#include "MaterialPassInstance.h"

#if defined (RENDERER_DX12)
#include "GpuDataInterfaceDX12.h"
#include "MaterialPass.h"
#include "Renderer.h"
#include "GpuProgramPipeline.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  GpuDataInterfaceDX12::GpuDataInterfaceDX12()
  {
    
  }
//---------------------------------------------------------------------------//
  void GpuDataInterfaceDX12::applyMaterialPass(const MaterialPass* _pMaterialPass, RenderContext* aRenderContext)
  {
    aRenderContext->setBlendState(*_pMaterialPass->getBlendState());
    aRenderContext->setDepthStencilState(*_pMaterialPass->getDepthStencilState());
    
    aRenderContext->setCullMode(_pMaterialPass->getCullMode());
    aRenderContext->setFillMode(_pMaterialPass->getFillMode());
    aRenderContext->setWindingOrder(_pMaterialPass->getWindingOrder());
    
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      ShaderStage eStage = static_cast<ShaderStage>(i);
      aRenderContext->setGpuProgram(_pMaterialPass->myProgramPipeline->myGpuPrograms[(uint32)eStage], eStage);
    }
  }
//---------------------------------------------------------------------------//
  void GpuDataInterfaceDX12::applyMaterialPassInstance(const MaterialPassInstance* aMaterialPassInstance, RenderContext* aRenderContext)
  {
    // TODO: [Tricky] Depending on the currently active signature, bind the resources to the commandList via the provided Renderer
    //                For descriptor-tables, we need the correct DescriptorHandle+Range from somewhere...

    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      ShaderStage eStage = static_cast<ShaderStage>(i);

      const Texture* const* ppReadTextures = aMaterialPassInstance->getReadTextures();
      for (uint32 iRegIndex = 0u; iRegIndex < Constants::kMaxNumReadTextures; ++iRegIndex)
      {
        aRenderContext->setReadTexture(ppReadTextures[iRegIndex], iRegIndex);
      }

      const TextureSampler* const* ppTextureSamplers = aMaterialPassInstance->getTextureSamplers();
      for (uint32 iRegIndex = 0u; iRegIndex < Constants::kMaxNumTextureSamplers; ++iRegIndex)
      {
        aRenderContext->setTextureSampler(ppTextureSamplers[iRegIndex], iRegIndex);
      }
    }
  }
//---------------------------------------------------------------------------//
} } }
#endif  // RENDERER_DX12