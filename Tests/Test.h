#pragma once

#include <fancy_core/FancyCoreDefines.h>
#include <string>

namespace Fancy {
  class Window;
  class FancyRuntime;
  struct InputState;
  class RenderOutput;
}

class Test
{
public:
  Test(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState, const char* aName)
    : myRuntime(aRuntime)
    , myWindow(aWindow)
    , myOutput(aRenderOutput)
    , myInput(anInputState)
    , myName(aName)
  { }
  virtual ~Test() = default;

  virtual void OnWindowResized(uint /*aWidth*/, uint /*aHeight*/) {};
  virtual void OnUpdate(bool /*aDrawProperties*/) {}
  virtual void OnRender() {}

  const char* GetName() const { return myName.c_str(); }

private:
  Fancy::FancyRuntime* myRuntime;
  Fancy::Window* myWindow;
  Fancy::RenderOutput* myOutput;
  Fancy::InputState* myInput;
  std::string myName;
};
