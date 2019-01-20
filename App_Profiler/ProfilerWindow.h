#pragma once
#include <fancy_core/DynamicArray.h>
#include <fancy_core/FancyCoreDefines.h>

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Render();
  
  Fancy::DynamicArray<uint> mySampledFrames;
  bool myIsPaused = false;
  float myScale = 1.0f;
  float myOffset = 0.0f;
};
