#pragma once
#include <fancy_core/FancyCoreDefines.h>

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Render();
  
  bool myIsPaused = false;

  float64 myStartTime = 0.0;
  float64 myDuration = 0.0;
};
