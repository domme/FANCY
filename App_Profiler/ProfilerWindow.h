#pragma once
#include <fancy_core/FancyCoreDefines.h>
#include <cfloat>

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Render(int aScreenSizeX, int aScreendSizeY);
  
  bool myIsPaused = false;

  float myHorizontalOffset = FLT_MAX;
  float myScale = 1.0f;
};
