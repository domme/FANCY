#include <windows.h>

#include <Fancy.h>

#include "Prerequisites.h"
#include "WindowDX12.h"
#include "RendererDX12.h"
#include <RenderingProcessForward.h>
#include <Scene.h>
#include <SceneImporter.h>
#include <SceneNode.h>
#include <ObjectName.h>
#include <LightComponent.h>

WindowDX12 ourWindow;
// RendererDX12 ourRenderer;
constexpr uint kWidth = 1280u;
constexpr uint kHeight = 720u;



Fancy::Rendering::RenderingProcessForward* pRenderProcessFwd = nullptr;
Fancy::Scene::CameraComponent* pCameraComponent;
Fancy::Scene::SceneNode* pModelNode;

void StartupEngine()
{
  Fancy::Init(&ourWindow.myWindowHandle);

  Fancy::Scene::ScenePtr pScene = std::make_shared<Fancy::Scene::Scene>();
  Fancy::SetCurrentScene(pScene);

  pRenderProcessFwd = new Fancy::Rendering::RenderingProcessForward;
  Fancy::SetRenderingProcess(pRenderProcessFwd);

  Fancy::Scene::SceneNode* pCameraNode = pScene->getRootNode()->createChildNode(_N(CameraNode));
  pCameraComponent = static_cast<Fancy::Scene::CameraComponent*>(pCameraNode->addOrRetrieveComponent(_N(CameraComponent)));
  pCameraComponent->setProjectionPersp(45.0f, kWidth, kHeight, 1.0f, 1000.0f);
  pScene->setActiveCamera(pCameraComponent);

  pModelNode = pScene->getRootNode()->createChildNode(_N(ModelNode));
  Fancy::IO::SceneImporter::importToSceneGraph("Models/cube.obj", pModelNode);
  pModelNode->getTransform().setPositionLocal(glm::vec3(0.0f, 0.0f, -10.0f));

  Fancy::Scene::SceneNode* pLightNode = pScene->getRootNode()->createChildNode(_N(LightNode));
  Fancy::Scene::LightComponent* pLight = static_cast<Fancy::Scene::LightComponent*>(pLightNode->addOrRetrieveComponent(_N(LightComponent)));

  Fancy::Startup();
}

void onWindowResize(uint aWidth, uint aHeight)
{
	
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	WindowParameters params;
	params.myTitle = "Fancy Engine (DX12)";
	params.myWidth = kWidth;
	params.myHeight = kHeight;

	if (!ourWindow.init(hInstance, nCmdShow, params))
		return -1;

	ourWindow.myResizeCallback = onWindowResize;

  StartupEngine();

	MSG msg = { 0 };
	while (true)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}

    pModelNode->getTransform().rotate(glm::vec3(1.0f, 1.0f, 0.0f), 20.0f);

    Fancy::Update(0.0);
	}

	return 0;
}