#pragma once
#include <fancy_core/FancyCoreDefines.h>

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Render(int aScreenSizeX, int aScreendSizeY);
  
  bool myIsPaused = false;

  float myHorizontalOffset = 0.0f;
  float myVerticalOffset = 0.0f;
  float myScale = 1.0f;
};
