#pragma once

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Show();

private:
  int myWidth = 100;
  int myHeight = 100;
  bool myIsPaused = false;
  float myScale = 1.0f;
};