#pragma once

#include <windows.h>

#include <fancy_core/Fancy_Include.h>
#include <fancy_core/RenderCore.h>
#include <fancy_imgui/imgui.h>
#include <fancy_imgui/imgui_impl_fancy.h>
#include <fancy_core/RenderOutput.h>

class App_Imgui
{
public:
  App_Imgui(HINSTANCE anInstanceHandle);
  ~App_Imgui();

  void Init();
  void Update();
  void Render();
  void Shutdown();

private:
  void OnWindowResized(Fancy::uint aWidth, Fancy::uint aHeight);

  Fancy::FancyRuntime* myRuntime;
};

