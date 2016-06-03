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
#include <RenderWindow.h>

Fancy::Rendering::RenderingProcessForward* pRenderProcessFwd = nullptr;
Fancy::Scene::CameraComponent* pCameraComponent;
Fancy::Scene::SceneNode* pModelNode;

void OnWindowResized(uint aWidth, uint aHeight)
{
  pCameraComponent->setProjectionPersp(45.0f, aWidth, aHeight, 1.0f, 1000.0f);
}

void StartupEngine(HINSTANCE anAppInstanceHandle)
{
  Fancy::Init(anAppInstanceHandle);

  Fancy::RenderWindow* window = Fancy::GetCurrentRenderWindow();

  std::function<void(uint, uint)> onResizeCallback = &OnWindowResized;
  window->myOnResize.Connect(onResizeCallback);
  
  Fancy::Scene::ScenePtr pScene = std::make_shared<Fancy::Scene::Scene>();
  Fancy::SetCurrentScene(pScene);

  pRenderProcessFwd = new Fancy::Rendering::RenderingProcessForward;
  Fancy::SetRenderingProcess(pRenderProcessFwd);

  Fancy::Scene::SceneNode* pCameraNode = pScene->getRootNode()->createChildNode(_N(CameraNode));
  pCameraComponent = static_cast<Fancy::Scene::CameraComponent*>(pCameraNode->addOrRetrieveComponent(_N(CameraComponent)));
  pCameraComponent->setProjectionPersp(45.0f, window->GetWidth(), window->GetHeight(), 1.0f, 1000.0f);
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

LRESULT CALLBACK locOnWindowEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  // Handle destroy/shutdown messages.
  switch (message)
  {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }

  // Handle any messages the switch statement didn't.
  return DefWindowProc(hWnd, message, wParam, lParam);
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  StartupEngine(hInstance);

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