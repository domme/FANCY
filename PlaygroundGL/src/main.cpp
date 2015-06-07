
#include <windows.h>

#define GLFW_DLL
#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sstream>

#include <Scene.h>
#include <SceneNode.h>
#include <CameraComponent.h>
#include <EngineCommon.h>
#include <SceneImporter.h>
#include <RenderingProcessForward.h>
#include <ObjectName.h>
#include <LightComponent.h>

using namespace Fancy;

Rendering::RenderingProcessForward* pRenderProcessFwd = nullptr;
Scene::CameraComponent* pCameraComponent;

bool bRightMouseButtonDown = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;

glm::vec3 cameraMovement(0.0f, 0.0f, 0.0f);

void startupEngine()
{
  Fancy::EngineCommon::initEngine();

  Fancy::Scene::ScenePtr pScene = std::make_shared<Fancy::Scene::Scene>();
  EngineCommon::setCurrentScene(pScene);

  pRenderProcessFwd = new Rendering::RenderingProcessForward;
  EngineCommon::setRenderingProcess(pRenderProcessFwd);

  Scene::SceneNode* pCameraNode = pScene->getRootNode()->createChildNode(_N(CameraNode));
  pCameraComponent = static_cast<Scene::CameraComponent*>(pCameraNode->addOrRetrieveComponent(_N(CameraComponent)));
  pScene->setActiveCamera(pCameraComponent);
  
  Scene::SceneNode* pModelNode = pScene->getRootNode()->createChildNode(_N(ModelNode));
  IO::SceneImporter::importToSceneGraph("Models/cube.obj", pModelNode);
  // IO::SceneImporter::importToSceneGraph("Models/Sibenik/Sibenik_omme.dae", pModelNode);
  pModelNode->getTransform().setPositionLocal(glm::vec3(0.0f, 0.0f, -10.0f));

  Scene::SceneNode* pLightNode = pScene->getRootNode()->createChildNode(_N(LightNode));
  Scene::LightComponent* pLight = static_cast<Scene::LightComponent*>(pLightNode->addOrRetrieveComponent(_N(LightComponent)));

  EngineCommon::startup();
}

void updateWindowSize(int width, int height)
{
  Fancy::EngineCommon::setWindowSize(width, height);
  pCameraComponent->setProjectionPersp(45.0f, width, height, 1.0f, 1000.0f);
}

void onMouseMove(GLFWwindow* _window, double _posX, double _posY)
{
  double dx = (_posX - lastMouseX) * 10.0f;
  double dy = (_posY - lastMouseY) * 10.0f;

  if (bRightMouseButtonDown)
  {
    Scene::Transform& camTransform = pCameraComponent->getSceneNode()->getTransform();
    camTransform.rotate(glm::vec3(0.0f, 1.0f, 0.0f), -dx);
    camTransform.rotate(camTransform.right(), -dy);
  }

  lastMouseX = _posX;
  lastMouseY = _posY;
}

void onMouseButton(GLFWwindow* _window, int _button, int _action, int _mods)
{
  if (_button == GLFW_MOUSE_BUTTON_RIGHT)
  {
    if (_action == GLFW_PRESS || _action == GLFW_REPEAT)
    {
      glfwGetCursorPos(_window, &lastMouseX, &lastMouseY);
      bRightMouseButtonDown = true;
    }
    else
    {
      bRightMouseButtonDown = false;
    }
  }
}

static void onKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  using namespace Fancy::IO;

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  if (key == GLFW_KEY_W)
  {
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      cameraMovement.z = -1.0f;
    }
    else
    {
      cameraMovement.z = 0.0f;
    }
  }

  if (key == GLFW_KEY_A)
  {
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      cameraMovement.x = -1.0f;
    }
    else
    {
      cameraMovement.x = 0.0f;
    }
  }

  if (key == GLFW_KEY_S)
  {
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      cameraMovement.z = 1.0f;
    }
    else
    {
      cameraMovement.z = 0.0f;
    }
  }

  if (key == GLFW_KEY_D)
  {
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      cameraMovement.x = 1.0f; 
    }
    else
    {
      cameraMovement.x = 0.0f; 
    }
  }

  if (key == GLFW_KEY_E)
  {
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      cameraMovement.y = 1.0f;
    }
    else
    {
      cameraMovement.y = 0.0f;
    }
  }

  if (key == GLFW_KEY_Q)
  {
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      cameraMovement.y = -1.0f; 
    }
    else
    {
      cameraMovement.y = 0.0f; 
    }
  }
}

static void moveCamera()
{
  if (bRightMouseButtonDown)
  {
    pCameraComponent->getSceneNode()->getTransform().translateLocal(cameraMovement * 0.2f);
  }
}

void shutdownEngine()
{
  Fancy::EngineCommon::shutdownEngine();

  if (pRenderProcessFwd)
  {
    delete pRenderProcessFwd;
    pRenderProcessFwd = nullptr;
  }
}

static void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}

int main(void)
{
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
#if defined (_DEBUG)
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif  // _DEBUG
  /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // 
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); */

  window = glfwCreateWindow(1280, 720, "Simple example", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  
  glfwSetCursorPosCallback(window, onMouseMove);
  glfwSetMouseButtonCallback(window, onMouseButton);
  glfwSetKeyCallback(window, onKeyboard);

  glfwMakeContextCurrent(window);

  unsigned int glewInitStatus = glewInit();
  if (glewInitStatus == GLEW_OK)
  {
    std::stringstream ss;
    ss << "Successfully initialized OpenGL subsystem:" << std::endl;
    ss << "   Vendor: " << (const char*) glGetString(GL_VENDOR) << std::endl;
    ss << "   Renderer: " << (const char*) glGetString(GL_RENDERER) << std::endl;
    ss << "   Version: " << (const char*) glGetString(GL_VERSION) << std::endl;
    ss << "   GLSL Version: " << (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    log_Info(ss.str());
  }

  startupEngine();

  double lastTime = glfwGetTime();
  while (!glfwWindowShouldClose(window))
  {
    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    updateWindowSize(width, height);
    
    moveCamera();

    double currTime = glfwGetTime();
    Fancy::EngineCommon::update(currTime - lastTime);
  
    glfwSwapBuffers(window);
    glfwPollEvents();

    lastTime = currTime;
  }

  shutdownEngine();
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}