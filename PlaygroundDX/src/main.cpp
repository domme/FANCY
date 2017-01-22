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
#include "RenderView.h"
#include "GraphicsWorld.h"

Fancy::Scene::CameraComponent* pCameraComponent;
Fancy::Scene::SceneNode* pModelNode;
Fancy::FancyRuntime* pRuntime = nullptr;

void OnWindowResized(uint aWidth, uint aHeight)
{
  pCameraComponent->setProjectionPersp(45.0f, aWidth, aHeight, 1.0f, 1000.0f);
}

void StartupEngine(HINSTANCE anAppInstanceHandle)
{
  if (pRuntime != nullptr)
    return;

  Fancy::EngineParameters params;
  params.myResourceFolder = "../../../resources/";
  params.myRenderingTechnique = Fancy::RenderingTechnique::FORWARD;

  pRuntime = Fancy::FancyRuntime::Init(anAppInstanceHandle, params);

  Fancy::RenderView* mainView = pRuntime->GetMainView();
  Fancy::RenderWindow* window = mainView->GetRenderWindow();
  std::function<void(uint, uint)> onResizeCallback = &OnWindowResized;
  window->myOnResize.Connect(onResizeCallback);
  
  Fancy::Scene::Scene* pScene = pRuntime->GetMainWorld()->GetScene();
  Fancy::Scene::SceneNode* pCameraNode = pScene->getRootNode()->createChildNode(_N(CameraNode));
  pCameraComponent = static_cast<Fancy::Scene::CameraComponent*>(pCameraNode->addOrRetrieveComponent(_N(CameraComponent)));
  pCameraComponent->setProjectionPersp(45.0f, window->GetWidth(), window->GetHeight(), 1.0f, 1000.0f);
  pScene->setActiveCamera(pCameraComponent);

  pModelNode = pRuntime->GetMainWorld()->Import("Models/cube.obj");
  pModelNode->getTransform().setPositionLocal(glm::vec3(0.0f, 0.0f, 10.0f));

  Fancy::Scene::SceneNode* pLightNode = pScene->getRootNode()->createChildNode(_N(LightNode));
  Fancy::Scene::LightComponent* pLight = static_cast<Fancy::Scene::LightComponent*>(pLightNode->addOrRetrieveComponent(_N(LightComponent)));
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

    pRuntime->Update(0.016f);
	}

	return 0;
}