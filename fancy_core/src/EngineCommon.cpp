#include "EngineCommon.h"

#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Renderer.h"
#include "SceneNodeComponentFactory.h"

#include "ModelComponent.h"
#include "CameraComponent.h"
#include "Texture.h"

#include "DepthStencilState.h"
#include "BlendState.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  EngineCommon::EngineCommon()
  {

  }
//---------------------------------------------------------------------------//
  EngineCommon::~EngineCommon()
  {

  }
//---------------------------------------------------------------------------//
  bool EngineCommon::initEngine()
  {
    initComponentSubsystem();
    initRenderingSubsystem();

    return true;
  }
//---------------------------------------------------------------------------//
  void EngineCommon::initComponentSubsystem()
  {
    Scene::SceneNodeComponentFactory::registerFactory(_N(Model), Scene::ModelComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(Camera), Scene::CameraComponent::create);
  }
//---------------------------------------------------------------------------//
  void EngineCommon::initRenderingSubsystem()
  {
    // Init common blend- and depthstencil states
    Rendering::BlendState blendState(_N(BlendState_Solid));
    blendState.setBlendStatePerRT(false);
    blendState.setBlendEnabled(0u, false);
    blendState.setRTwriteMask(0u, UINT_MAX);
    Rendering::BlendState::registerWithName(blendState.getName(), blendState);

    Rendering::DepthStencilState depthStencilState(_N(DepthStencilState_DefaultDepthState));
    depthStencilState.setStencilEnabled(false);
    depthStencilState.setDepthTestEnabled(true);
    depthStencilState.setDepthWriteEnabled(true);
    depthStencilState.setDepthCompFunc(Rendering::CompFunc::LESS);
    Rendering::DepthStencilState::registerWithName(depthStencilState.getName(), depthStencilState);

    Rendering::Renderer& rnd = Rendering::Renderer::getInstance();
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy