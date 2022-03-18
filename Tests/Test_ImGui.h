#pragma once
#include "Common/Application.h"

class Test_ImGui : public Application
{
public:
  Test_ImGui(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  void OnUpdate(bool aDrawProperties) override;

private:
  bool myShowTestWindow = false;
};

