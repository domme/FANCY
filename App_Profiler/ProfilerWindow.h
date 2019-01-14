#pragma once

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Render();
  
  bool myIsPaused = false;
  float myScale = 1.0f;
  float myOffset = 0.0f;
};