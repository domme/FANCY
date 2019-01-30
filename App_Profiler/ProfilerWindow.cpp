#include "ProfilerWindow.h"

#include <fancy_imgui/imgui.h>
#include <fancy_core/Profiling.h>
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
const uint kFrameBoundaryColor = 0xFF38ACFF;
const uint kFrameHeaderColor = 0xFFAAAAAA;
const uint kWindowBgColor = 0x883A3A3A;
const float kFrameHeaderHeight = 10.0f;
const float kFrameGraphHeightScale = 0.25f;
const ImGuiID kFrameId_FrameGraph = 1;

const char* FormatString(const char* aFmt, ...)
{
  va_list args;
  va_start(args, aFmt);

  const int neededSize = vsnprintf(nullptr, 0u, aFmt, args) + 1;
  ASSERT(neededSize < ARRAY_LENGTH(TextBuf));
  const int offset = vsnprintf(TextBuf, ARRAY_LENGTH(TextBuf), aFmt, args);
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

bool RenderSample(const Profiling::SampleNode& aNode, const ImVec2& aPos, const ImVec2& aSize)
{
  const Profiling::SampleNodeInfo& nodeInfo = Profiling::GetSampleInfo(aNode.myNodeInfo);
  const char* aLabel = FormatString("%s: %.3f", nodeInfo.myName, (float)aNode.myDuration);

  const ImVec2 labelSize = ImGui::CalcTextSize(aLabel);
  if (labelSize.x > aSize.x * 0.75f)
    aLabel = "";

  ImGui::SetCursorPos(ToLocalPos(aPos));
  ImGui::PushStyleColor(ImGuiCol_Button, GetColorForTag(nodeInfo.myTag));
  const bool pressed = ImGui::Button(aLabel, aSize);
  ImGui::PopStyleColor(1);

  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("%s \n  \t Start: %.6fms \n  \t Duration: %.3fms \n", nodeInfo.myName, aNode.myStart, aNode.myDuration);

  return pressed;
}

struct ScaleArgs
{
  float myMsToPixelScale = 1.0f;
  float myScale = 1.0f;
};

enum TimeUnit
{
  Milliseconds,
  Microseconds,
  Nanoseconds,

  Last = Nanoseconds
};

const char* TimeUnitToString(TimeUnit aUnit)
{
  switch(aUnit) 
  { 
    case Milliseconds: return "ms";
    case Microseconds: return "us";
    case Nanoseconds: return "ns";
    default: ASSERT(false); return "";
  }
}

struct NodeRenderArgs
{
  float64 myFrameStart;
  ImVec2 myStartPos;
};

void RenderNodeRecursive(const Profiling::SampleNode& aNode, const ScaleArgs& someScaleArgs, const NodeRenderArgs& someRenderArgs, int aDepth)
{
  ImVec2 pos, size;
  pos.x = someRenderArgs.myStartPos.x + (aNode.myStart - someRenderArgs.myFrameStart) * someScaleArgs.myMsToPixelScale;
  pos.y = someRenderArgs.myStartPos.y + kZoneElementHeight_WithPadding * aDepth;
  size.x = aNode.myDuration * someScaleArgs.myMsToPixelScale;
  size.y = kZoneElementHeight;

  RenderSample(aNode, pos, size);

  if (aNode.myChild != UINT_MAX)
  {
    const Profiling::SampleNode& firstChild = Profiling::GetSampleData(aNode.myChild);
    RenderNodeRecursive(firstChild, someScaleArgs, someRenderArgs, aDepth + 1);
  }

  if (aNode.myNext != UINT_MAX)
  {
    const Profiling::SampleNode& nextNode = Profiling::GetSampleData(aNode.myNext);
    RenderNodeRecursive(nextNode, someScaleArgs, someRenderArgs, aDepth);
  }
}

void RenderFrameHeader(float aWidth, uint64 aFrameNumber)
{
  if (aWidth < 1.0)
    return;

  ImDrawList* dl = ImGui::GetCurrentWindow()->DrawList;

  bool renderText = aWidth > 10.0f;
  ImVec2 textSize(0.0f, 0.0f);
  const char* text = nullptr;
  if (renderText)
  {
    text = FormatString("Frame %d", aFrameNumber);
    textSize = ImGui::CalcTextSize(text);
    renderText = aWidth - textSize.x > 20.0f;
  }

  ImGuiWindow* window = ImGui::GetCurrentWindow();

  ImVec2 startPosLocal = ImGui::GetCursorPos();
  ImVec2 pos = ToGlobalPos(startPosLocal);
  if (renderText)
  {
    ImVec2 subPos = pos;
    subPos.x += aWidth * 0.5f - textSize.x * 0.5f - 1.0f;
    dl->AddLine(pos, subPos, kFrameHeaderColor, kDefaultLineWidth);

    subPos.x += 1.0f;
    dl->AddText(subPos, kFrameHeaderColor, text);

    subPos.x += textSize.x + 1.0f;
    ImVec2 endPos = pos;
    endPos.x += aWidth;

    dl->AddLine(subPos, endPos, kFrameHeaderColor, kDefaultLineWidth);
  }
  else
  {
    ImVec2 end = pos;
    end.x += aWidth;
    dl->AddLine(pos, end, kFrameHeaderColor, kDefaultLineWidth);
  }

  startPosLocal.y += textSize.y + 10.0f;
  ImGui::SetCursorPos(startPosLocal);
}

void RenderFrameBoundary(float aHeight)
{
  ImGuiWindow* window = ImGui::GetCurrentWindow();

  ImVec2 start = ToGlobalPos(ImGui::GetCursorPos());
  ImVec2 end = ToGlobalPos(ImGui::GetCursorPos());
  end.y += aHeight;
  window->DrawList->AddLine(start, end, kFrameBoundaryColor, kDefaultLineWidth);
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + kDefaultLineWidth);
}

void RenderRuler(const ScaleArgs& someScaleArgs)
{
  ImGuiWindow* window = ImGui::GetCurrentWindow();

  TimeUnit mainMarkerUnit = TimeUnit::Milliseconds;
  float mainMarkerOffset = someScaleArgs.myMsToPixelScale;
  float mainMarkerDuration = 1;  // Duration in mainMarkerUnit
  while (mainMarkerOffset > 500 && mainMarkerUnit < TimeUnit::Last)
  {
    mainMarkerUnit = (TimeUnit) ((uint) mainMarkerUnit + 1);
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
    window->DrawList->AddText(labelPos, markerColor, FormatString("%d%s", currMainMarkerTime, TimeUnitToString(mainMarkerUnit)));

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

void ProfilerWindow::Render(int aScreenSizeX, int aScreendSizeY)
{
  if (ImGui::Checkbox("Pause", &myIsPaused))
    Profiling::SetPaused(myIsPaused);

  if (Time::ourFrameIdx > 50)  // Hack for testing: stop collecting after 50 frames
  {
    myIsPaused = true;
    Profiling::SetPaused(true);
  }
  
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ToVec4Color(kWindowBgColor));
  ImGui::Begin("Profiler");

  const float baseHorizontalScale = ImGui::GetWindowWidth() / (16.0f * 4.0f);  // Fit 4 16ms-frames to the image-width
  ScaleArgs scaleArgs;
  scaleArgs.myMsToPixelScale = baseHorizontalScale * myScale;
  scaleArgs.myScale = myScale;
  const float64 timeOffset = myHorizontalOffset / scaleArgs.myMsToPixelScale;
  const Profiling::FrameData& lastFrame = Profiling::GetFrameData(Profiling::GetLastFrame());
  const float64 maxTime = glm::max(16.0, lastFrame.myStart + timeOffset);
  const float64 minTime = glm::max(0.0, maxTime - ImGui::GetWindowWidth() / scaleArgs.myMsToPixelScale);

  RenderRuler(scaleArgs);
  
  const ImVec2 frameGraphRect_min = ToGlobalPos(ImGui::GetCursorPos());
  ImVec2 frameGraphRect_max;
  frameGraphRect_max.x = (frameGraphRect_min.x + ImGui::GetWindowWidth()) - ImGui::GetCursorPosX();
  frameGraphRect_max.y = (frameGraphRect_min.y + ImGui::GetWindowSize().y * kFrameGraphHeightScale);
  const ImVec2 frameGraphSize(frameGraphRect_max.x - frameGraphRect_min.x, frameGraphRect_max.y - frameGraphRect_min.y);
  ImGui::BeginChild(kFrameId_FrameGraph, frameGraphSize, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

  const Profiling::FrameId firstFrame = Profiling::GetFirstFrame();
  const Profiling::FrameId endFrame = Profiling::GetLastFrame() + 1u;
  Profiling::FrameId frame = firstFrame;
  
  const ImVec2 frameGraphStartPosLocal = ImGui::GetCursorPos();
  ImVec2 firstFrameStartPosLocal = frameGraphStartPosLocal;
  firstFrameStartPosLocal.x -= myHorizontalOffset;
  ImGui::SetCursorPos(firstFrameStartPosLocal);
  while(frame != endFrame)
  {
    const Profiling::FrameData& frameData = Profiling::GetFrameData(frame);
    if (frameData.myFirstSample == UINT_MAX || frameData.myStart + frameData.myDuration < minTime)
    {
      ++frame;
      continue;
    }

    //if (frameData.myStart > maxTime)
    //  break;

    RenderFrameHeader(frameData.myDuration * scaleArgs.myMsToPixelScale, frameData.myFrame);

    NodeRenderArgs nodeRenderArgs;
    nodeRenderArgs.myFrameStart = frameData.myStart;
    nodeRenderArgs.myStartPos = ToGlobalPos(ImGui::GetCursorPos());
    
    const Profiling::SampleNode& node = Profiling::GetSampleData(frameData.myFirstSample);
    RenderNodeRecursive(node, scaleArgs, nodeRenderArgs, 0);

    ImGui::SetCursorPos(ImVec2(
      firstFrameStartPosLocal.x + static_cast<float>(frameData.myDuration * scaleArgs.myMsToPixelScale),
      firstFrameStartPosLocal.y
    ));
    
    RenderFrameBoundary(300.0f);

    ++frame;
  }

  ImGui::SetCursorPos(ImVec2(frameGraphStartPosLocal.x, frameGraphStartPosLocal.y + frameGraphSize.y));

  ImGui::EndChild();  // End frame graph area

  ImGui::Separator();
  ImGui::SliderFloat("Frames", &myHorizontalOffset, 0.0f, (maxTime - minTime) * scaleArgs.myMsToPixelScale, "%.0f");
  ImGui::SliderFloat("Scale", &myScale, 0.1f, 100.0f, "%.0f");
    
  ImGui::End();
  ImGui::PopStyleColor();
}
