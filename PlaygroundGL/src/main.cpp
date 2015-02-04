
#include <windows.h>

#define GLFW_DLL
#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sstream>

#include <Scene.h>
#include <EngineCommon.h>
#include <SceneImporter.h>

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
    ss << "Succesfully initialized OpenGL subsystem:" << std::endl;
    ss << "   Vendor: " << (const char*) glGetString(GL_VENDOR) << std::endl;
    ss << "   Renderer: " << (const char*) glGetString(GL_RENDERER) << std::endl;
    ss << "   Version: " << (const char*) glGetString(GL_VERSION) << std::endl;
    ss << "   GLSL Version: " << (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    log_Info(ss.str());
  }

  // Init the engine
  Fancy::EngineCommon::initEngine();
  Fancy::Scene::ScenePtr pScene = std::make_shared<Fancy::Scene::Scene>();

  Fancy::IO::SceneImporter::_shaderTest();
  // Fancy::IO::SceneImporter::importToSceneGraph("Models/cube.obj", pScene->getRootNode());

  glfwSetKeyCallback(window, key_callback);
  while (!glfwWindowShouldClose(window))
  {
    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
  

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  Fancy::EngineCommon::shutdownEngine();
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}