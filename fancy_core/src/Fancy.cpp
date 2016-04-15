#include "Fancy.h"

#include "Scene.h"
#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Renderer.h"
#include "SceneNodeComponentFactory.h"

#include "ModelComponent.h"
#include "CameraComponent.h"
#include "CameraControllerComponent.h"
#include "Texture.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "PathService.h"
#include "SceneImporter.h"
#include "TimeManager.h"
#include "RenderingProcess.h"
#include "LightComponent.h"
#include "TextureLoader.h"

namespace Fancy {
  Scene::ScenePtr m_pCurrScene = nullptr;
  Rendering::RenderingProcess* m_pRenderingProcess = nullptr;
  Rendering::Renderer* ourRenderer = nullptr;

  void initComponentSubsystem()
  {
    Scene::SceneNodeComponentFactory::registerFactory(_N(ModelComponent), Scene::ModelComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(CameraComponent), Scene::CameraComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(LightComponent), Scene::LightComponent::create);
    // Scene::SceneNodeComponentFactory::registerFactory(_N(CameraControllerComponent), Scene::CameraControllerComponent::create);
  }

  void initRenderingSubsystem(void* aNativeWindowHandle)
  {
    ourRenderer = new Rendering::Renderer(aNativeWindowHandle);

    Rendering::RenderCore::InitPlatform();
    Rendering::RenderCore::Init();

    ourRenderer->postInit();
  }

  void initIOsubsystem()
  {
    IO::PathService::SetResourceLocation("../../../resources/");
    IO::SceneImporter::initLogger();
  }

  bool Init(void* aNativeWindowHandle)
  {
    initIOsubsystem();
    initComponentSubsystem();
    initRenderingSubsystem(aNativeWindowHandle);
    
    // DEBUG: Test texture loading
    String texPathAbs = IO::PathService::convertToAbsPath("Textures\\Sibenik\\mramor6x6.png");

    // Load and decode the texture to memory
    std::vector<uint8> vTextureBytes;
    IO::TextureLoadInfo texLoadInfo;
    if (!IO::TextureLoader::loadTexture(texPathAbs, vTextureBytes, texLoadInfo))
    {
      LOG_ERROR("Failed to load texture at path %", texPathAbs);
      return false;
    }

    if (texLoadInfo.bitsPerPixel / texLoadInfo.numChannels != 8u)
    {
      LOG_ERROR("Unsupported texture format: %", texPathAbs);
      return false;
    }

    Rendering::Texture* tex = FANCY_NEW(Rendering::Texture, MemoryCategory::TEXTURES);

    Rendering::TextureParams texParams;
    texParams.myIsExternalTexture = true;
    texParams.path = texPathAbs;
    texParams.bIsDepthStencil = false;
    texParams.eFormat = texLoadInfo.numChannels == 3u ? Rendering::DataFormat::SRGB_8 : Rendering::DataFormat::SRGB_8_A_8;
    texParams.u16Width = texLoadInfo.width;
    texParams.u16Height = texLoadInfo.height;
    texParams.u16Depth = 0u;
    texParams.uAccessFlags = (uint32)Rendering::GpuResourceAccessFlags::NONE;

    Rendering::TextureUploadData uploadData;
    uploadData.myData = &vTextureBytes[0];
    uploadData.myPixelSizeBytes = texLoadInfo.bitsPerPixel / 8u;
    uploadData.myRowSizeBytes = texLoadInfo.width * uploadData.myPixelSizeBytes;
    uploadData.mySliceSizeBytes = texLoadInfo.width * texLoadInfo.height * uploadData.myPixelSizeBytes;
    uploadData.myTotalSizeBytes = uploadData.mySliceSizeBytes;

    tex->create(texParams, &uploadData, 1u);
    
    return true;
  }

  void ShutdownRenderingSubsystem()
  {
    Rendering::RenderCore::Shutdown();
    Rendering::RenderCore::ShutdownPlatform();

    SAFE_DELETE(ourRenderer);
  }
//---------------------------------------------------------------------------//
  void Shutdown()
  {
    IO::SceneImporter::destroyLogger();
    ShutdownRenderingSubsystem();
  }
//---------------------------------------------------------------------------//
  void SetWindowSize(uint32 _uWidth, uint32 _uHeight)
  {
    const glm::uvec4& viewportParams = ourRenderer->GetDefaultContext()->getViewport();
    if(viewportParams.z != _uWidth || viewportParams.w != _uHeight)
    {
      ourRenderer->GetDefaultContext()->setViewport(glm::uvec4(viewportParams.x, viewportParams.y, _uWidth, _uHeight));
    }
  }
//---------------------------------------------------------------------------//
  Rendering::Renderer* GetRenderer()
  {
    return ourRenderer;
  }
//---------------------------------------------------------------------------//
  Rendering::RenderingProcess* GetRenderingProcess()
  {
    return m_pRenderingProcess;
  }
//---------------------------------------------------------------------------//
  void Startup()
  {
    ASSERT(m_pCurrScene, "No scene set");
    ASSERT(m_pRenderingProcess, "No rendering process set");

    m_pRenderingProcess->startup();
    m_pCurrScene->startup();
  }
//---------------------------------------------------------------------------//
  void Update(double _dt)
  {
    ASSERT(m_pCurrScene, "No scene set");
    ASSERT(m_pRenderingProcess, "No rendering process set");

    Time::update(_dt);
    const float deltaTime = Time::getDeltaTime();

    m_pCurrScene->update(deltaTime);

    ourRenderer->beginFrame();
    m_pRenderingProcess->tick(deltaTime);
    ourRenderer->endFrame();
  }
//---------------------------------------------------------------------------//
  void SetCurrentScene( const Scene::ScenePtr& _pScene )
  {
    m_pCurrScene = _pScene;
  }
//---------------------------------------------------------------------------//
  const Scene::ScenePtr& GetCurrentScene()
  {
    return m_pCurrScene;
  }
//---------------------------------------------------------------------------//
  void SetRenderingProcess( Rendering::RenderingProcess* _pRenderingProcess )
  {
    m_pRenderingProcess = _pRenderingProcess;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy