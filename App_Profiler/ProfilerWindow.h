#pragma once

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Render();

  bool myIsPaused = false;
  float myScale = 1.0f;

};