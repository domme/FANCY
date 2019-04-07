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
  bool myIsShowingTimeline[2] = { true };
  bool myIsShowingGpuFrames = true;

  float myHorizontalOffset = FLT_MAX;
  float myScale = 1.0f;
  float myTimeToPixelScale = 1.0f;

private:
  void ScrollAndScale(float maxOffset, float overallGraphDuration, float aRectMinX, float aRectMinY, float aRectMaxX, float aRectMaxY);;
  void RenderRuler(float64 aMinStartTime);
  void RenderTimelines(float64 aMinStartTime, float64 aMaxEndTime, uint& aFirstRenderedFrame, uint& aLastRenderedFrame, float& aMaxHorizontalOffset);
  uint myFocusedTimeline = 0;
};
