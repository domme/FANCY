#pragma once
#include "Test.h"

class Test_ImGui : public Test
{
public:
  Test_ImGui(Fancy::AssetManager* anAssetManager, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  void OnUpdate(bool aDrawProperties) override;

private:
  bool myShowTestWindow = false;
};

