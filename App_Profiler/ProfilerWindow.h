#pragma once
#include <fancy_core/FancyCoreDefines.h>

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Render();
  
  bool myIsPaused = false;

  float myScale = 1.0f;
};
