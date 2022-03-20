#pragma once

#include "Common/FancyCoreDefines.h"
#include "EASTL/string.h"

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

protected:
  Fancy::FancyRuntime* myRuntime;
  Fancy::Window* myWindow;
  Fancy::RenderOutput* myOutput;
  Fancy::InputState* myInput;
  eastl::string myName;
};
