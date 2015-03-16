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
  void RenderingProcess::applyMaterialPass( const MaterialPass* _pMaterialPass, Renderer* _pRenderer )
  {
    _pRenderer->setBlendState(*_pMaterialPass->getBlendState());
    _pRenderer->setDepthStencilState(*_pMaterialPass->getDepthStencilState());

    _pRenderer->setCullMode(_pMaterialPass->getCullMode());
    _pRenderer->setFillMode(_pMaterialPass->getFillMode());
    _pRenderer->setWindingOrder(_pMaterialPass->getWindingOrder());

    for (uint32 i = 0u; i < (uint32) ShaderStage::NUM; ++i)
    {
      ShaderStage eStage = static_cast<ShaderStage>(i);
      _pRenderer->setGpuProgram(_pMaterialPass->getGpuProgram(eStage), eStage);
    }
  }
//---------------------------------------------------------------------------//
  void RenderingProcess::applyMaterialPassInstance( const MaterialPassInstance* _pMaterialPassInstance, Renderer* _pRenderer )
  {
    for (uint32 i = 0u; i < (uint32) ShaderStage::NUM; ++i)
    {
      ShaderStage eStage = static_cast<ShaderStage>(i);

      const Texture* const* ppReadTextures = _pMaterialPassInstance->getReadTextures(eStage);
      for (uint32 iRegIndex = 0u; iRegIndex < kMaxNumReadTextures; ++iRegIndex)
      {
        // TODO: Implement a method for setting multiple textures at once
        _pRenderer->setReadTexture(ppReadTextures[iRegIndex], eStage, iRegIndex);
      }
    }
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering