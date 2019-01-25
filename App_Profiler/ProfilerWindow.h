#pragma once
#include <fancy_core/FancyCoreDefines.h>

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Render();
  
  bool myIsPaused = false;

  float myHorizontalOffset = 0.0f;
  float myScale = 1.0f;
};
