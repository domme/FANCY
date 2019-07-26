#pragma once
#include "Test.h"

class Test_Mipmapping : public Test
{
public:
  Test_Mipmapping(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_Mipmapping() override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;
};

