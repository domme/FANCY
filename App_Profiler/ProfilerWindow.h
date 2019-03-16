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

  TimelineInfo GetTimelineInfo(uint aTimeline);
  void HandleTimelineAreaScrollZoom(float anAreaMin_x, float anAreaMin_y, float anAreaMax_x, float anAreaMax_y);
  void RenderTimelines(uint& aFirstRenderedFrame, uint& aLastRenderedFrame);
  uint myFocusedTimeline = 0;
};
