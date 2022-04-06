#pragma once

#include "Common/FancyCoreDefines.h"
#include "EASTL/string.h"

namespace Fancy {
  class AssetManager;
  class Window;
  struct InputState;
  class RenderOutput;
}

class Test
{
public:
  Test(Fancy::AssetManager* anAssetManager, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState, const char* aName)
    : myAssetManager(anAssetManager)
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
  Fancy::AssetManager* myAssetManager;
  Fancy::Window* myWindow;
  Fancy::RenderOutput* myOutput;
  Fancy::InputState* myInput;
  eastl::string myName;
};
