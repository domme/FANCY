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
const float kFrameGraphHeightScale = 0.33f;
const ImGuiID kFrameId_FrameGraph = 1;
const uint kNumGraphFrames = 200;

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

void RenderNodeRecursive(const Profiler::SampleNode& aNode, float64 aTimeToPixelScale, float64 aFrameStartTime,
                         const ImVec2& aFrameStartPos, int aDepth)
{
  ImVec2 pos, size;
  pos.x = aFrameStartPos.x + static_cast<float>((aNode.myStart - aFrameStartTime) * aTimeToPixelScale);
  pos.y = aFrameStartPos.y + kZoneElementHeight_WithPadding * aDepth;
  size.x = static_cast<float>(aNode.myDuration * aTimeToPixelScale);
  size.y = kZoneElementHeight;

  RenderSample(aNode, pos, size);

  const CircularArray<Profiler::SampleNode>& recordedSamples = Profiler::GetRecordedSamples();

  if (aNode.myChild != UINT_MAX)
  {
    const Profiler::SampleNode& firstChild = recordedSamples[aNode.myChild];
    RenderNodeRecursive(firstChild, aTimeToPixelScale, aFrameStartTime, aFrameStartPos, aDepth + 1);
  }

  if (aNode.myNext != UINT_MAX)
  {
    const Profiler::SampleNode& nextNode = recordedSamples[aNode.myNext];
    RenderNodeRecursive(nextNode, aTimeToPixelScale, aFrameStartTime, aFrameStartPos, aDepth);
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

void RenderFrameTimeGraph(uint aFirstWindowFrame, uint aLastWindowFrame, float64 aMaxFrameTimePixelHeight, float64 aMaxFrameTime)
{
  if (aFirstWindowFrame == UINT_MAX)
    return;

  const CircularArray<Profiler::FrameData>& recordedFrames = Profiler::GetRecordedFrames();

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

void ProfilerWindow::Render(int aScreenSizeX, int aScreendSizeY)
{
  if (ImGui::Checkbox("Pause", &myIsPaused))
    Profiler::ourPauseRequested = myIsPaused;

  ImGui::PushStyleColor(ImGuiCol_WindowBg, ToVec4Color(kWindowBgColor));
  ImGui::Begin("Profiler");

  const ImVec2 frameGraphRect_min = ToGlobalPos(ImGui::GetCursorPos());
  ImVec2 frameGraphRect_max;
  frameGraphRect_max.x = (frameGraphRect_min.x + ImGui::GetWindowWidth()) - ImGui::GetCursorPosX();
  frameGraphRect_max.y = (frameGraphRect_min.y + ImGui::GetWindowSize().y * kFrameGraphHeightScale);
  const ImVec2 frameGraphSize(frameGraphRect_max.x - frameGraphRect_min.x, frameGraphRect_max.y - frameGraphRect_min.y);

  const CircularArray<Profiler::FrameData>& recordedFrames = Profiler::GetRecordedFrames();

  float64 overallFrameDuration = 0.0;
  float64 maxFrameDuration = 0.0;
  for (uint i = 0u, e = recordedFrames.Size(); i < e; ++i)
  {
    const float64 frameTime = recordedFrames[i].myDuration;
    overallFrameDuration += frameTime;
    maxFrameDuration = glm::max(maxFrameDuration, frameTime);
  }

  const float64 baseHorizontalScale = 1280.0 / (16.0 * 4.0);
  float64 timeToPixelScale = baseHorizontalScale * myScale;
  const float overallFrameSize = static_cast<float>(overallFrameDuration * timeToPixelScale);
  const float maxOffset = glm::max(0.0f, overallFrameSize - ImGui::GetWindowWidth() - 1.0f);

  //RenderRuler(timeToPixelScale);

  if (ImGui::IsMouseHoveringRect(frameGraphRect_min, frameGraphRect_max))
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
      timeToPixelScale = baseHorizontalScale * myScale;

      // Adjust offset so the current mouse pos stays centered
      const float frameOrigin = -myHorizontalOffset;
      const float mousePos_FrameSpace = glm::max(0.0f, ToLocalPos(ImGui::GetMousePos()).x - frameOrigin);
      const float mouseAlongFrame = overallFrameSize < FLT_EPSILON ? 0.0f : mousePos_FrameSpace / overallFrameSize;

      const float newOverallFrameSize = static_cast<float>(overallFrameDuration * timeToPixelScale);
      const float newMousePos_FrameSpace = mouseAlongFrame * newOverallFrameSize;

      myHorizontalOffset += newMousePos_FrameSpace - mousePos_FrameSpace;
    }
  }

  if (!myIsPaused)
    myHorizontalOffset = maxOffset;

  ImGui::SliderFloat("Frames", &myHorizontalOffset, 0.0f, maxOffset, "%.1f");

  ImGui::BeginChild(kFrameId_FrameGraph, frameGraphSize, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

  uint firstWindowFrame = UINT_MAX;
  uint lastWindowFrame = UINT_MAX;
  const ImVec2 frameGraphStart = ImGui::GetCursorPos();
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() - myHorizontalOffset);
  const CircularArray<Profiler::SampleNode>& recordedSamples = Profiler::GetRecordedSamples();
  for (uint i = 0u, e = recordedFrames.Size(); i < e; ++i)
  {
    const Profiler::FrameData& frameData = recordedFrames[i];
    const float frameSize = static_cast<float>(frameData.myDuration * timeToPixelScale);
    const float framePos = ImGui::GetCursorPosX();

    if (frameData.myFirstSample == UINT_MAX)
      continue;

    if (framePos + frameSize < 0)
    {
      ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + frameSize, frameGraphStart.y));
      continue;
    }
    if (framePos > ImGui::GetWindowWidth())
    {
      break;
    }

    if (firstWindowFrame == UINT_MAX)
      firstWindowFrame = i;

    lastWindowFrame = i;

    RenderFrameHeader(frameSize, frameGraphSize.y, frameData);

    const Profiler::SampleNode& node = recordedSamples[frameData.myFirstSample];
    RenderNodeRecursive(node, timeToPixelScale, frameData.myStart, ToGlobalPos(ImGui::GetCursorPos()), 0);

    ImGui::SetCursorPos(ImVec2(framePos + frameSize + 1.0f, frameGraphStart.y));

    RenderFrameBoundary(100.0f);
  }

  ImGui::SetCursorPos(ImVec2(frameGraphStart.x, frameGraphStart.y + frameGraphSize.y));
  ImGui::EndChild(); // End frame graph area

  ImGui::Separator();

  RenderFrameTimeGraph(firstWindowFrame, lastWindowFrame, 100.0, maxFrameDuration);
  
  ImGui::End();
  ImGui::PopStyleColor();
}
