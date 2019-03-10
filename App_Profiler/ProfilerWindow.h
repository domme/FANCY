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

private:
  struct TimelineInfo
  {
    float64 myMaxFrameDuration = 0.0;
    float64 myOverallDuration = 0.0;
  };

  TimelineInfo GetTimelineInfo(uint aTimeline);
  void RenderTimeline(uint aTimeline, float64 aTimeToPixelScale, const TimelineInfo& aTimelineInfo, uint& aFirstRenderedFrame, uint& aLastRenderedFrame);
  uint myFocusedTimeline = 0;
};
