#pragma once
#include <fancy_core/FancyCoreDefines.h>
#include <cfloat>

class ProfilerWindow
{
public:
  ProfilerWindow();
  ~ProfilerWindow();

  void Render();
  
  bool myIsPaused;
  
private:
  void ScrollAndScale(float64 aMinStartTime, float64 aMaxEndTime, float aRectMinX, float aRectMinY, float aRectMaxX, float aRectMaxY);
  void RenderRuler(float64 aMinStartTime);
  void RenderTimelines(float64 aMinStartTime, float64 aMaxEndTime, uint& aFirstRenderedFrame, uint& aLastRenderedFrame);

  bool myIsShowingTimeline[2];
  uint myFocusedTimeline;
  float myHorizontalOffset;
  float myTimeToPixelScale;
};
