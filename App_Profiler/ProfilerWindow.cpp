#include "ProfilerWindow.h"

#include <fancy_imgui/imgui.h>
#include <fancy_core/Profiler.h>
#include <fancy_core/FancyCoreDefines.h>
#include <fancy_core/Log.h>
#include <fancy_core/MathIncludes.h>
#include "fancy_imgui/imgui_internal.h"
#include <list>
#include <fancy_core/TimeManager.h>

using namespace Fancy;

ProfilerWindow::ProfilerWindow()
{
}


ProfilerWindow::~ProfilerWindow()
{
}

char TextBuf[2048];
const float kZoneElementHeight = 20.0f;
const float kZoneElementHeight_WithPadding = 25.0f;
const float kRulerMarkerVerticalSize = 40.0f;
const float kSubRulerMarkerVerticalSize = 15.0f;
const float kDefaultLineWidth = 1.0f;
const uint kFrameBoundaryColor = 0xFFAAAAAA;
const uint kFrameHeaderColor = 0xFFAAAAAA;
const uint kWindowBgColor = 0xAA3A3A3A;
const float kFrameHeaderHeight = 10.0f;
const float kFrameGraphHeightScale = 0.75f;
const ImGuiID kFrameId_FrameGraph[Profiler::TIMELINE_NUM] = { 1, 2 };
const uint kNumGraphFrames = 200;
const float64 kBaseHorizontalScale = 1280.0 / (16.0 * 10.0);

enum TimeUnit
{
  Milliseconds,
  Microseconds,
  Nanoseconds,

  Last = Nanoseconds
};

const char* TimeUnitToString(TimeUnit aUnit)
{
  switch (aUnit)
  {
  case Milliseconds: return "ms";
  case Microseconds: return "us";
  case Nanoseconds: return "ns";
  default: ASSERT(false);
    return "";
  }
}

const char* FormatString(const char* aFmt, ...)
{
  va_list args;
  va_start(args, aFmt);

  const int neededSize = vsnprintf(nullptr, 0u, aFmt, args) + 1;
  ASSERT(neededSize < (int)ARRAY_LENGTH(TextBuf));
  const int offset = vsnprintf(TextBuf, static_cast<size_t>(ARRAY_LENGTH(TextBuf)), aFmt, args);
  TextBuf[offset + 1] = '\0';
  va_end(args);
  return TextBuf;
}

ImVec2 ToLocalPos(const ImVec2& aPos)
{
  ImVec2 localPos = aPos;
  localPos.x -= ImGui::GetWindowPos().x;
  localPos.y -= ImGui::GetWindowPos().y;
  return localPos;
}

ImVec2 ToGlobalPos(const ImVec2& aPos)
{
  ImVec2 globalPos = aPos;
  globalPos.x += ImGui::GetWindowPos().x;
  globalPos.y += ImGui::GetWindowPos().y;
  return globalPos;
}

ImVec4 ToVec4Color(uint aColor)
{
  ImVec4 col;
  col.x = (aColor & 0xFF) / 255.0f;
  col.y = ((aColor & 0xFF00) >> 8) / 255.0f;
  col.z = ((aColor & 0xFF0000) >> 16) / 255.0f;
  col.w = ((aColor & 0xFF000000) >> 24) / 255.0f;
  return col;
}

ImVec4 GetColorForTag(uint8 aTag)
{
  switch (aTag)
  {
  case 0: return ImVec4(0.7f, 0.0f, 0.0f, 0.5f);
  default: return ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
  }
}

void GetFrameInfoAllTimelines(uint aFrame, float64& aMinFrameStartTime, float64& aMaxFrameDuration)
{
  for (uint i = 0u; i < Profiler::TIMELINE_NUM; ++i)
  {
    const auto& frames = Profiler::GetRecordedFrames((Profiler::Timeline) i);
    const Profiler::FrameData& frame = frames[aFrame];
    if (frame.myHasValidTimes)
    {
      aMinFrameStartTime = glm::min(aMinFrameStartTime, frame.myStart.myTime);
      aMaxFrameDuration = glm::max(aMaxFrameDuration, frame.myDuration);
    }
  }
}

void GetTimeRange(float64& aMinTimeOut, float64& aMaxTimeOut)
{
  float64 minStart = DBL_MAX;
  float64 maxEnd = 0.0;
  for (uint i = 0u; i < Profiler::TIMELINE_NUM; ++i)
  {
    const auto& frames = Profiler::GetRecordedFrames((Profiler::Timeline) i);
    if (frames.Size() == 0)
      continue;

    const Profiler::FrameData& firstFrame = frames[0];
    if (!firstFrame.myHasValidTimes)
      continue;

    minStart = glm::min(minStart, firstFrame.myStart.myTime);

    for (int k = frames.Size() - 1; k >= 0; --k)
    {
      if (frames[k].myHasValidTimes)
      {
        maxEnd = glm::max(maxEnd, frames[k].myEnd.myTime);
        break;
      }
    }
  }

  aMinTimeOut = minStart;
  aMaxTimeOut = maxEnd;
}

bool RenderSample(const Profiler::SampleNode& aNode, const ImVec2& aPos, const ImVec2& aSize)
{
  const Profiler::SampleNodeInfo& nodeInfo = Profiler::GetSampleInfo(aNode.myNodeInfo);
  const char* aLabel = FormatString("%s: %.3f", nodeInfo.myName, (float)aNode.myDuration);

  const ImVec2 labelSize = ImGui::CalcTextSize(aLabel);
  if (labelSize.x > aSize.x * 0.75f)
    aLabel = "";

  ImGui::SetCursorPos(ToLocalPos(aPos));
  ImGui::PushStyleColor(ImGuiCol_Button, GetColorForTag(nodeInfo.myTag));
  const bool pressed = ImGui::Button(aLabel, aSize);
  ImGui::PopStyleColor(1);

  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("%s \n  \t Start: %.6fms \n  \t Duration: %.3fms \n", nodeInfo.myName, aNode.myStart,
                      aNode.myDuration);

  return pressed;
}

void RenderNodeRecursive(const Profiler::SampleNode& aNode, float64 aTimeToPixelScale, float64 aFrameStartTime,
                         const ImVec2& aFrameStartPos, int aDepth, Profiler::Timeline aTimeline)
{
  if (!aNode.myHasValidTimes)
    return;

  ImVec2 pos, size;
  pos.x = aFrameStartPos.x + static_cast<float>((aNode.myStart.myTime - aFrameStartTime) * aTimeToPixelScale);
  pos.y = aFrameStartPos.y + kZoneElementHeight_WithPadding * aDepth;
  size.x = static_cast<float>(aNode.myDuration * aTimeToPixelScale);
  size.y = kZoneElementHeight;

  RenderSample(aNode, pos, size);

  const CircularArray<Profiler::SampleNode>& recordedSamples = Profiler::GetRecordedSamples(aTimeline);

  if (aNode.myChild != UINT_MAX)
  {
    const Profiler::SampleNode& firstChild = recordedSamples[aNode.myChild];
    RenderNodeRecursive(firstChild, aTimeToPixelScale, aFrameStartTime, aFrameStartPos, aDepth + 1, aTimeline);
  }

  if (aNode.myNext != UINT_MAX)
  {
    const Profiler::SampleNode& nextNode = recordedSamples[aNode.myNext];
    RenderNodeRecursive(nextNode, aTimeToPixelScale, aFrameStartTime, aFrameStartPos, aDepth, aTimeline);
  }
}

void RenderFrameHeader(float aWidth, float aWholeFrameHeight, const Profiler::FrameData& aFrameData)
{
  ImDrawList* dl = ImGui::GetCurrentWindow()->DrawList;

  bool renderText = aWidth > 10.0f;
  ImVec2 textSize(0.0f, 0.0f);
  const char* text = text = FormatString("Frame %d - %.3fms", aFrameData.myFrame, aFrameData.myDuration);
  textSize = ImGui::CalcTextSize(text);

  if (renderText)
    renderText = aWidth - textSize.x > 20.0f;

  ImGuiWindow* window = ImGui::GetCurrentWindow();

  ImVec2 startPosLocal = ImGui::GetCursorPos();
  const ImVec2 startPosGlobal = ToGlobalPos(startPosLocal);
  if (renderText)
  {
    ImVec2 subPos = startPosGlobal;
    subPos.x += aWidth * 0.5f - textSize.x * 0.5f - 1.0f;
    dl->AddLine(startPosGlobal, subPos, kFrameHeaderColor, kDefaultLineWidth);

    subPos.x += 1.0f;
    dl->AddText(subPos, kFrameHeaderColor, text);

    subPos.x += textSize.x + 1.0f;
    ImVec2 endPos = startPosGlobal;
    endPos.x += aWidth;

    dl->AddLine(subPos, endPos, kFrameHeaderColor, kDefaultLineWidth);
  }
  else
  {
    ImVec2 end = startPosGlobal;
    end.x += aWidth;
    dl->AddLine(startPosGlobal, end, kFrameHeaderColor, kDefaultLineWidth);
  }

  if (ImGui::IsMouseHoveringRect(startPosGlobal,
                                 ImVec2(startPosGlobal.x + aWidth, startPosGlobal.y + aWholeFrameHeight)))
    ImGui::SetTooltip(text);

  startPosLocal.y += textSize.y + 10.0f;
  ImGui::SetCursorPos(startPosLocal);
}

void RenderFrameBoundary(float aHeight)
{
  ImGuiWindow* window = ImGui::GetCurrentWindow();

  ImVec2 start = ToGlobalPos(ImGui::GetCursorPos());
  ImVec2 end = start;
  end.y += aHeight;
  window->DrawList->AddLine(start, end, kFrameBoundaryColor, kDefaultLineWidth);
}

void RenderRuler(float aTimeToPixelScale)
{
  ImGuiWindow* window = ImGui::GetCurrentWindow();

  TimeUnit mainMarkerUnit = TimeUnit::Milliseconds;
  float mainMarkerOffset = aTimeToPixelScale;
  float mainMarkerDuration = 1; // Duration in mainMarkerUnit
  while (mainMarkerOffset > 500 && mainMarkerUnit < TimeUnit::Last)
  {
    mainMarkerUnit = (TimeUnit)((uint)mainMarkerUnit + 1);
    const float divide = 10.0f;
    mainMarkerOffset *= (1.0f / divide);
    mainMarkerDuration *= (1000.0f / divide);
  }

  const float subMarkerOffset = mainMarkerOffset / 10.0f;

  const float subMarkerVerticalOffset = (kRulerMarkerVerticalSize - kSubRulerMarkerVerticalSize) * 0.5f;
  const ImU32 markerColor = ImGui::ColorConvertFloat4ToU32(ImVec4(.5f, .5f, .5f, .8f));
  const float mainMarkerThickness = 1.5f;
  const float subMarkerThickness = 1.0f;

  const uint numMainMarkers = (uint)(ImGui::GetWindowSize().x / mainMarkerOffset);
  const uint numSubMarkers = 9;
  ImVec2 startPosLocal = ImGui::GetCursorPos();
  ImVec2 pos = ToGlobalPos(startPosLocal);
  int currMainMarkerTime = 0;
  for (uint iMain = 0u; iMain < numMainMarkers; ++iMain)
  {
    ImVec2 start = pos, end = pos;

    end.y += kRulerMarkerVerticalSize;
    window->DrawList->AddLine(start, end, markerColor, mainMarkerThickness);

    ImVec2 labelPos = end;
    labelPos.y += 5;
    window->DrawList->AddText(labelPos, markerColor,
                              FormatString("%d%s", currMainMarkerTime, TimeUnitToString(mainMarkerUnit)));

    for (uint iSub = 0u; iSub < numSubMarkers; ++iSub)
    {
      start.x += subMarkerOffset;
      end.x += subMarkerOffset;
      start.y = pos.y + subMarkerVerticalOffset;
      end.y = pos.y + (kRulerMarkerVerticalSize - subMarkerVerticalOffset);

      window->DrawList->AddLine(start, end, markerColor, subMarkerThickness);
    }

    pos.x += mainMarkerOffset;
    currMainMarkerTime += mainMarkerDuration;
  }

  startPosLocal.y += kRulerMarkerVerticalSize + 20.0f;
  ImGui::SetCursorPos(startPosLocal);
}

void RenderFrameTimeGraph(uint aFirstWindowFrame, uint aLastWindowFrame, float64 aMaxFrameTimePixelHeight, float64 aMaxFrameTime, Profiler::Timeline aTimeline)
{
  if (aFirstWindowFrame == UINT_MAX)
    return;

  const CircularArray<Profiler::FrameData>& recordedFrames = Profiler::GetRecordedFrames(aTimeline);

  // Determine the frame-range to display in the time-graph
  uint firstGraphFrame = aFirstWindowFrame;
  uint lastGraphFrame = aLastWindowFrame;
  uint numGraphFrames = (aLastWindowFrame - aFirstWindowFrame) + 1u;

  while (numGraphFrames < kNumGraphFrames && (firstGraphFrame != 0 || lastGraphFrame != recordedFrames.Size() - 1u))
  {
    if (firstGraphFrame != 0)
    {
      --firstGraphFrame;
      ++numGraphFrames;
    }
    if (lastGraphFrame != recordedFrames.Size() - 1u)
    {
      ++lastGraphFrame;
      ++numGraphFrames;
    }
  }

  const float itemWidth = (float) ImGui::GetWindowWidth() / numGraphFrames;
  const float itemHeightScale = aMaxFrameTimePixelHeight / aMaxFrameTime;

  const ImVec4 kWindowFrameColor = ImVec4(0.6f, 0.59f, 0.98f, 0.40f);
  const ImVec4 kDefaultColor = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  
  uint selectedFrame = UINT_MAX;

  for (uint i = firstGraphFrame; i <= lastGraphFrame; ++i)
  {
    const bool isWindowFrame = i >= aFirstWindowFrame && i <= aLastWindowFrame;
    const ImVec4 color = isWindowFrame ? kWindowFrameColor : kDefaultColor;
    ImGui::PushStyleColor(ImGuiCol_Button, color);

    const Profiler::FrameData& frameData = recordedFrames[i];
    if (ImGui::Button("", ImVec2(itemWidth, itemHeightScale * frameData.myDuration)))
      selectedFrame = i;

    ImGui::PopStyleColor();
    ImGui::SameLine(0, 0.1f);
  }
}

ProfilerWindow::TimelineInfo ProfilerWindow::GetTimelineInfo(uint aTimeline)
{
  const CircularArray<Profiler::FrameData>& recordedFrames = Profiler::GetRecordedFrames((Profiler::Timeline) aTimeline);
  TimelineInfo timelineInfo;
  for (uint i = 0u, e = recordedFrames.Size(); i < e; ++i)
  {
    const float64 frameTime = recordedFrames[i].myDuration;
    timelineInfo.myOverallDuration += frameTime;
    timelineInfo.myMaxFrameDuration = glm::max(timelineInfo.myMaxFrameDuration, frameTime);
  }
  return timelineInfo;
}

void ProfilerWindow::RenderTimelines(uint& aFirstRenderedFrame, uint& aLastRenderedFrame, float& aMaxHorizontalOffset)
{
  // FramegraphRect: Rectangular area over all timelines
  const ImVec2 frameGraphRect_min = ToGlobalPos(ImGui::GetCursorPos());
  ImVec2 frameGraphRect_max;
  frameGraphRect_max.x = (frameGraphRect_min.x + ImGui::GetWindowWidth()) - ImGui::GetCursorPosX();
  frameGraphRect_max.y = (frameGraphRect_min.y + ImGui::GetWindowSize().y * kFrameGraphHeightScale);
  const ImVec2 frameGraphSize(frameGraphRect_max.x - frameGraphRect_min.x, frameGraphRect_max.y - frameGraphRect_min.y);

  const ImVec2 timelineRectSize(frameGraphSize.x, frameGraphSize.y / Profiler::TIMELINE_NUM);
  ImVec2 timelineRectsMin[Profiler::TIMELINE_NUM];
  for (uint i = 0u; i < Profiler::TIMELINE_NUM; ++i)
  {
    timelineRectsMin[i].x = frameGraphRect_min.x;
    timelineRectsMin[i].y = frameGraphRect_min.y + i * timelineRectSize.y;
  }

  float64 overallStartTime, overallEndTime;
  GetTimeRange(overallStartTime, overallEndTime);
  const float64 overallDuration = glm::max(0.0, overallEndTime - overallStartTime);
  const float overallFrameSize = static_cast<float>(overallDuration * myTimeToPixelScale);
  const float maxOffset = glm::max(0.0f, overallFrameSize - ImGui::GetWindowWidth() - 1.0f);
  aMaxHorizontalOffset = maxOffset;

  if (!myIsPaused)
  {
    myHorizontalOffset = maxOffset;
  }
  else if (ImGui::IsMouseHoveringRect(frameGraphRect_min, frameGraphRect_max))
  {
    // Scrolling
    if (ImGui::IsMouseDragging(0))
    {
      const float scrollChange = ImGui::GetIO().MouseDelta.x;
      myHorizontalOffset = glm::clamp(myHorizontalOffset - scrollChange, 0.0f, maxOffset);
    }

    // Scaling
    if (glm::abs(ImGui::GetIO().MouseWheel) > 0.1f)
    {
      const float scaleChange = ImGui::GetIO().MouseWheel * 0.1f;

      myScale = glm::clamp(myScale + scaleChange, 0.01f, 100.0f);
      myTimeToPixelScale = kBaseHorizontalScale * myScale;

      // Adjust offset so the current mouse pos stays centered
      const float frameOrigin = -myHorizontalOffset;
      const float mousePos_FrameSpace = glm::max(0.0f, ToLocalPos(ImGui::GetMousePos()).x - frameOrigin);
      const float mouseAlongFrame = overallFrameSize < FLT_EPSILON ? 0.0f : mousePos_FrameSpace / overallFrameSize;

      const float newOverallFrameSize = static_cast<float>(overallDuration * myTimeToPixelScale);
      const float newMousePos_FrameSpace = mouseAlongFrame * newOverallFrameSize;

      myHorizontalOffset += newMousePos_FrameSpace - mousePos_FrameSpace;
    }
  }

  uint firstWindowFrame = UINT_MAX;
  uint lastWindowFrame = UINT_MAX;
  for (uint iTimeline = 0u; iTimeline < Profiler::TIMELINE_NUM; ++iTimeline)
  {
    const ImVec2 timelineRectMin = timelineRectsMin[iTimeline];
    const ImVec2 timelineRectMax(timelineRectMin.x + timelineRectSize.x, timelineRectMin.y + timelineRectSize.y);
    if (ImGui::IsMouseHoveringRect(timelineRectMin, timelineRectMax) && ImGui::IsMouseDown(0))
      myFocusedTimeline = iTimeline;

    const ImVec2 timelineRectMinLocal = ToLocalPos(timelineRectMin);

    ImGui::SetCursorPos(timelineRectMinLocal);
    ImGui::BeginChild(kFrameId_FrameGraph[iTimeline], timelineRectSize, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    const Profiler::Timeline timeline = static_cast<Profiler::Timeline>(iTimeline);
    const CircularArray<Profiler::FrameData>& recordedFrames = Profiler::GetRecordedFrames(timeline);
    const CircularArray<Profiler::SampleNode>& recordedSamples = Profiler::GetRecordedSamples(timeline);

    for (uint i = 0; i < recordedFrames.Size(); ++i)
    {
      const Profiler::FrameData& frameData = recordedFrames[i];
      if (!frameData.myHasValidTimes)
        continue;

      const float frameMinX = -myHorizontalOffset + (frameData.myStart.myTime - overallStartTime) * myTimeToPixelScale;
      const float frameSize = frameData.myDuration * myTimeToPixelScale;
      
      if (frameMinX + frameSize < 0 || frameMinX > ImGui::GetWindowWidth())
        continue;

      if (iTimeline == myFocusedTimeline)
      {
        if (firstWindowFrame == UINT_MAX)
          firstWindowFrame = i;

        lastWindowFrame = i;
      }

      ImGui::SetCursorPos(ImVec2(frameMinX, timelineRectMinLocal.y));
      RenderFrameBoundary(timelineRectSize.y);
      RenderFrameHeader(frameSize, timelineRectSize.y, frameData);

      /*
      if (frameData.myNumSamples > 0u)
      {
        ASSERT(frameData.myFirstSample != UINT_MAX);
        const Profiler::SampleNode& node = recordedSamples[frameData.myFirstSample];
        RenderNodeRecursive(node, myTimeToPixelScale, frameData.myStart.myTime, ToGlobalPos(ImGui::GetCursorPos()), 0, timeline);
      }
      */

      ImGui::SetCursorPos(ImVec2(frameMinX + frameSize, timelineRectMinLocal.y));
      RenderFrameBoundary(timelineRectSize.y);
    }

    ImGui::EndChild(); // End frame graph area
  }

  aFirstRenderedFrame = firstWindowFrame;
  aLastRenderedFrame = lastWindowFrame;
}

void ProfilerWindow::Render()
{
  if (ImGui::Checkbox("Pause", &myIsPaused))
    Profiler::ourPauseRequested = myIsPaused;

  myTimeToPixelScale = kBaseHorizontalScale * myScale;

  ImGui::PushStyleColor(ImGuiCol_WindowBg, ToVec4Color(kWindowBgColor));
  ImGui::Begin("Profiler");
  
  uint firstRenderedFrame, lastRenderedFrame = 0u;
  float maxOffset = 0.0f;
  RenderTimelines(firstRenderedFrame, lastRenderedFrame, maxOffset);
  ImGui::SliderFloat("Frames", &myHorizontalOffset, 0.0f, maxOffset, "%.1f");
  ImGui::Separator();
  //RenderFrameTimeGraph(firstRenderedFrame, lastRenderedFrame, 100.0, timelineInfos[myFocusedTimeline].myOverallDuration, (Profiler::Timeline) myFocusedTimeline);
  
  ImGui::End();
  ImGui::PopStyleColor();
}
