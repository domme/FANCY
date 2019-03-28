#pragma once
#include <fancy_core/FancyCoreDefines.h>
#include <cfloat>

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Render();
  
  bool myIsPaused = false;

  float myHorizontalOffset = FLT_MAX;
  float myScale = 1.0f;
  float myTimeToPixelScale = 1.0f;

private:
  struct TimelineInfo
  {
    float64 myMaxFrameDuration = 0.0;
    float64 myOverallDuration = 0.0;
  };

  void RenderTimelines(uint& aFirstRenderedFrame, uint& aLastRenderedFrame, float& aMaxHorizontalOffset);
  uint myFocusedTimeline = 0;
};
