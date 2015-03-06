#include "RenderingProcessForward.h"

#include "ShaderConstantsManager.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  RenderingProcessForward::RenderingProcessForward()
  {

  }
//---------------------------------------------------------------------------//
  RenderingProcessForward::~RenderingProcessForward()
  {

  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::startup()
  {
    // ShaderConstantsManager::update(ConstantBufferType::PER_LAUNCH);
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::tick(float _dt)
  {
    ShaderConstantsManager::update(ConstantBufferType::PER_FRAME);


  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering