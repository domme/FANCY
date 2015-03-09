
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
#include <Camera.h>
#include <CameraComponent.h>
#include <EngineCommon.h>
#include <SceneImporter.h>
#include <RenderingProcessForward.h>
#include <ObjectName.h>

using namespace Fancy;

Rendering::RenderingProcessForward* pRenderProcessFwd = nullptr;
Scene::CameraComponent* pCameraComponent;

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
  pModelNode->getTransform().setLocal(glm::translate(glm::vec3(0.0f, 0.0f, -10.0f)));

  EngineCommon::startup();
}

void updateWindowSize(int width, int height)
{
  Fancy::EngineCommon::setWindowSize(width, height);
  pCameraComponent->getCamera()->setProjectionPersp(45.0f, width, height, 1.0f, 1000.0f);
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
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(void)
{
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
  /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); */

  window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  
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

  glfwSetKeyCallback(window, key_callback);
  double lastTime = glfwGetTime();
  while (!glfwWindowShouldClose(window))
  {
    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    updateWindowSize(width, height);
    
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