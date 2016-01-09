#include "RendererPrerequisites.h"

#if defined (RENDERER_OPENGL4)
#include "GpuDataInterfaceGL4.h"
#include "MaterialPass.h"
#include "Renderer.h"
#include "MaterialPassInstance.h"
#include "GpuProgramPipeline.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  void GpuDataInterfaceGL4::applyMaterialPassInstance(const MaterialPassInstance* _pMaterialPassInstance, Renderer* _pRenderer)
  {
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      ShaderStage eStage = static_cast<ShaderStage>(i);

      const Texture* const* ppReadTextures = _pMaterialPassInstance->getReadTextures();
      for (uint32 iRegIndex = 0u; iRegIndex < Constants::kMaxNumReadTextures; ++iRegIndex)
      {
        _pRenderer->setReadTexture(ppReadTextures[iRegIndex], iRegIndex);
      }

      const Texture* const* ppWriteTextures = _pMaterialPassInstance->getWriteTextures();
      for (uint32 iRegIndex = 0u; iRegIndex < Constants::kMaxNumWriteTextures; ++iRegIndex)
      {
        _pRenderer->setWriteTexture(ppWriteTextures[iRegIndex], iRegIndex);
      }

      const GpuBuffer* const* ppReadBuffers = _pMaterialPassInstance->getReadBuffers();
      for (uint32 iRegIndex = 0u; iRegIndex < Constants::kMaxNumReadBuffers; ++iRegIndex)
      {
        _pRenderer->setReadBuffer(ppReadBuffers[iRegIndex], iRegIndex);
      }

      // No support in the renderer yet...
      //    const GpuBuffer* const* ppWriteBuffers = _pMaterialPassInstance->getWriteBuffers(eStage);
      //    for (uint32 iRegIndex = 0u; iRegIndex < kMaxNumWriteBuffers; ++iRegIndex)
      //    {
      //      _pRenderer->setWriteBuffer(ppWriteBuffers[iRegIndex], eStage, iRegIndex);
      //    }

      const TextureSampler* const* ppTextureSamplers = _pMaterialPassInstance->getTextureSamplers();
      for (uint32 iRegIndex = 0u; iRegIndex < Constants::kMaxNumTextureSamplers; ++iRegIndex)
      {
        _pRenderer->setTextureSampler(ppTextureSamplers[iRegIndex], iRegIndex);
      }
    }
  }
//---------------------------------------------------------------------------//
  void GpuDataInterfaceGL4::applyMaterialPass(const MaterialPass* _pMaterialPass, Renderer* _pRenderer)
  {
    _pRenderer->setBlendState(*_pMaterialPass->getBlendState());
    _pRenderer->setDepthStencilState(*_pMaterialPass->getDepthStencilState());

    _pRenderer->setCullMode(_pMaterialPass->getCullMode());
    _pRenderer->setFillMode(_pMaterialPass->getFillMode());
    _pRenderer->setWindingOrder(_pMaterialPass->getWindingOrder());
    
    // TODO: Find a nicer place for the PipelineHandle-generation...
    if (_pMaterialPass->myProgramPipeline->myPipelineHandleGL == GLUINT_HANDLE_INVALID)  // Lazy init...
      _pMaterialPass->myProgramPipeline->GeneratePipelineHandleGL();

    _pRenderer->SetGpuProgramPipeline(_pMaterialPass->myProgramPipeline);
  }
//---------------------------------------------------------------------------//
} } }
#endif  // RENDERER_GL4