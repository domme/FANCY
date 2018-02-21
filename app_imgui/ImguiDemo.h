#pragma once

#include <windows.h>

#include <fancy_core/Fancy_Include.h>
#include <fancy_core/RenderCore.h>
#include <fancy_imgui/imgui.h>
#include <fancy_imgui/imgui_impl_fancy.h>
#include <fancy_core/RenderOutput.h>

class ImguiDemo
{
public:
  ImguiDemo(HINSTANCE anInstanceHandle);
  ~ImguiDemo();

  void Init();
  void Update();
  void Render();
  void Shutdown();

private:
  void OnWindowResized(Fancy::uint aWidth, Fancy::uint aHeight);

  Fancy::FancyRuntime* myRuntime;
  UniquePtr<Fancy::Rendering::RenderOutput> myRenderOutput;
};

