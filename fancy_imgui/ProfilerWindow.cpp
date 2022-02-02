#include "ProfilerWindow.h"

#include <imgui.h>
#include <Common/FancyCoreDefines.h>
#include <Common/MathIncludes.h>
#include <Common/TimeManager.h>
#include <Debug/Annotations.h>
#include <Debug/Log.h>
#include <Debug/Profiler.h>

#include "imgui_internal.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  static char TextBuf[2048];
  static const float kZoneElementHeight = 20.0f;
  static const float kZoneElementHeight_WithPadding = 25.0f;
  static const float kRulerMarkerVerticalSize = 10.0f;
  static const float kSubRulerMarkerVerticalSize = 5.0f;
  static const float kDefaultLineWidth = 1.0f;
  static const uint kFrameBoundaryColor = 0xFFAAAAAA;
  static const uint kFrameHeaderColor = 0xFFAAAAAA;
  static const uint kWindowBgColor = 0xAA3A3A3A;
  static const float kFrameHeaderHeight = 10.0f;
  static const float kFrameGraphHeightScale = 0.45f;
  static const ImGuiID kFrameId_FrameGraph[Profiler::TIMELINE_NUM] = { 1, 2 };
  static const uint kNumGraphFrames = 200;
  static const float kRulerSubMarkerVerticalOffset = (kRulerMarkerVerticalSize - kSubRulerMarkerVerticalSize) * 0.5f;
  static const ImU32 kRulerMarkerColor = ImGui::ColorConvertFloat4ToU32(ImVec4(.5f, .5f, .5f, .8f));
  static const float kRulerMainMarkerThickness = 1.5f;
  static const float kRulerSubMarkerThickness = 1.0f;

  static const char* kTimeUnitLabels[] =
  {
    "s",
    "ms",
    "us",
    "ns"
  };

  static const float64 kMsToTimeUnitFactors[] =
  {
    1.0 / 1000.0,
    1.0,
    1000.0,
    1000.0 * 1000.0,
  };

  static const float64 kTimeUnitToMsFactors[] =
  {
    1000.0,
    1.0,
    1.0 / 1000.0,
    1.0 / (1000.0 * 1000.0),
  };
  static_assert(ARRAYSIZE(kTimeUnitLabels) == ARRAYSIZE(kMsToTimeUnitFactors), "Mismatching array sizes");
  static_assert(ARRAYSIZE(kTimeUnitLabels) == ARRAYSIZE(kTimeUnitToMsFactors), "Mismatching array sizes");
//---------------------------------------------------------------------------//
  static const char* FormatString(const char* aFmt, ...)
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
//---------------------------------------------------------------------------//
  static ImVec2 ToLocalPos(const ImVec2& aPos)
  {
    ImVec2 localPos = aPos;
    localPos.x -= ImGui::GetWindowPos().x;
    localPos.y -= ImGui::GetWindowPos().y;
    return localPos;
  }
//---------------------------------------------------------------------------//
  static ImVec2 ToGlobalPos(const ImVec2& aPos)
  {
    ImVec2 globalPos = aPos;
    globalPos.x += ImGui::GetWindowPos().x;
    globalPos.y += ImGui::GetWindowPos().y;
    return globalPos;
  }
//---------------------------------------------------------------------------//
  static ImVec4 ToVec4Color(uint aColor)
  {
    ImVec4 col;
    col.x = (aColor & 0xFF) / 255.0f;
    col.y = ((aColor & 0xFF00) >> 8) / 255.0f;
    col.z = ((aColor & 0xFF0000) >> 16) / 255.0f;
    col.w = ((aColor & 0xFF000000) >> 24) / 255.0f;
    return col;
  }
//---------------------------------------------------------------------------//
  static void GetTimeRange(float64& aMinTimeOut, float64& aMaxTimeOut)
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
//---------------------------------------------------------------------------//
  static bool RenderSample(const Profiler::SampleNode& aNode, const ImVec2& aPosLocal, ImVec2 aSize)
  {
    const Profiler::SampleNodeInfo& nodeInfo = Profiler::GetSampleInfo(aNode.myNodeInfo);
    const char* aLabel = FormatString("%s: %.3f", nodeInfo.myName, (float)aNode.myDuration);

    const ImVec2 labelSize = ImGui::CalcTextSize(aLabel);
    if (labelSize.x > aSize.x * 0.75f)
      aLabel = "";

    const AnnotationTagData& tagData = Annotations::GetTagData(nodeInfo.myTag);

    if (aSize.x > 2)
      aSize.x -= 1;

    ImGui::SetCursorPos(aPosLocal);
    ImGui::PushStyleColor(ImGuiCol_Button, tagData.myColor);
    const bool pressed = ImGui::Button(aLabel, aSize);
    ImGui::PopStyleColor(1);

    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("%s \n  \t Start: %.6fms \n  \t Duration: %.3fms \n", nodeInfo.myName, aNode.myStart,
        aNode.myDuration);

    return pressed;
  }
  //---------------------------------------------------------------------------//
  static void RenderSampleRecursive(const Profiler::SampleNode& aNode, float64 aTimeToPixelScale, float64 aMinStartTime, float aPixelOffset, float aFramePosLocalY, int aDepth, Profiler::Timeline aTimeline)
  {
    if (!aNode.myHasValidTimes)
      return;

    ImVec2 pos, size;
    pos.x = (aNode.myStart.myTime - aMinStartTime) * aTimeToPixelScale - aPixelOffset;
    pos.y = aFramePosLocalY + kZoneElementHeight_WithPadding * aDepth;
    size.x = static_cast<float>(aNode.myDuration * aTimeToPixelScale);
    size.y = kZoneElementHeight;

    RenderSample(aNode, pos, size);

    const CircularArray<Profiler::SampleNode>& recordedSamples = Profiler::GetRecordedSamples(aTimeline);

    if (aNode.myChild != UINT_MAX)
    {
      const Profiler::SampleNode& firstChild = recordedSamples[aNode.myChild];
      RenderSampleRecursive(firstChild, aTimeToPixelScale, aMinStartTime, aPixelOffset, aFramePosLocalY, aDepth + 1, aTimeline);
    }

    if (aNode.myNext != UINT_MAX)
    {
      const Profiler::SampleNode& nextNode = recordedSamples[aNode.myNext];
      RenderSampleRecursive(nextNode, aTimeToPixelScale, aMinStartTime, aPixelOffset, aFramePosLocalY, aDepth, aTimeline);
    }
  }
//---------------------------------------------------------------------------//
  static void RenderFrameHeader(float aWidth, float aWholeFrameHeight, const Profiler::FrameData& aFrameData)
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

    if (ImGui::IsMouseHoveringRect(startPosGlobal, ImVec2(startPosGlobal.x + aWidth, startPosGlobal.y + aWholeFrameHeight)))
    {
      text = FormatString("Frame %d - %.3fms\n"
        "Start: %.3fms\n"
        "End: %.3fms",
        aFrameData.myFrame, aFrameData.myDuration,
        (float)aFrameData.myStart.myTime,
        (float)aFrameData.myEnd.myTime);

      ImGui::SetTooltip(text);
    }

    startPosLocal.y += textSize.y + 10.0f;
    ImGui::SetCursorPos(startPosLocal);
  }
//---------------------------------------------------------------------------//
  static void RenderFrameBoundary(float aHeight)
  {
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    ImVec2 start = ToGlobalPos(ImGui::GetCursorPos());
    ImVec2 end = start;
    end.y += aHeight;
    window->DrawList->AddLine(start, end, kFrameBoundaryColor, kDefaultLineWidth);
  }
//---------------------------------------------------------------------------//
  static void RenderFrameTimeGraph(uint aFirstWindowFrame, uint aLastWindowFrame, float64 aMaxFrameTimePixelHeight, float64 aMaxFrameTime, Profiler::Timeline aTimeline)
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

    const float itemWidth = (float)ImGui::GetWindowWidth() / numGraphFrames;
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
//---------------------------------------------------------------------------//
  static const char* GetTimeLabel(float64 aTimeMs)
  {
    const float64 unitThreshold = 2.0 / 1000.0;
    const char* bestTimeLabel = kTimeUnitLabels[0];
    float64 bestTime = aTimeMs * kMsToTimeUnitFactors[0];

    if (bestTime < unitThreshold)
    {
      for (uint i = 1; i < ARRAYSIZE(kTimeUnitLabels); ++i)
      {
        const float64 timeInUnit = aTimeMs * kMsToTimeUnitFactors[i];
        if (timeInUnit < unitThreshold)
        {
          bestTime = timeInUnit;
          bestTimeLabel = kTimeUnitLabels[i];
        }
      }
    }

    return FormatString("%.3f%s", bestTime, bestTimeLabel);
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  ProfilerWindow::ProfilerWindow()
    : myIsPaused(false)
    , myIsShowingTimeline{ true, true }
    , myFocusedTimeline(0)
    , myHorizontalOffset(0.0f)
    , myTimeToPixelScale(40.0f)
  {
  }
//---------------------------------------------------------------------------//
  ProfilerWindow::~ProfilerWindow()
  {
  }
//---------------------------------------------------------------------------//
  void ProfilerWindow::ScrollAndScale(float64 aMinStartTime, float64 aMaxEndTime, float aRectMinX, float aRectMinY, float aRectMaxX, float aRectMaxY)
  {
    const ImVec2 frameGraphRect_min(aRectMinX, aRectMinY);
    const ImVec2 frameGraphRect_max(aRectMaxX, aRectMaxY);

    const float64 overallTimelineDuration = glm::max(0.0, aMaxEndTime - aMinStartTime);
    const float overallTimelineWidth = static_cast<float>(overallTimelineDuration * myTimeToPixelScale);
    const float maxHorizontalOffset = glm::max(0.0f, overallTimelineWidth - ImGui::GetWindowWidth() - 1.0f);

    if (ImGui::IsMouseHoveringRect(frameGraphRect_min, frameGraphRect_max))
    {
      // Scrolling
      if (ImGui::IsMouseDragging(0))
      {
        const float scrollChange = ImGui::GetIO().MouseDelta.x;
        myHorizontalOffset = glm::clamp(myHorizontalOffset - scrollChange, 0.0f, maxHorizontalOffset);
      }

      // Scaling
      if (glm::abs(ImGui::GetIO().MouseWheel) > 0.1f)
      {
        const float scaleChange = ImGui::GetIO().MouseWheel * (ImGui::GetIO().KeyShift ? 2.0f : 0.25f);
        myTimeToPixelScale = glm::clamp(myTimeToPixelScale + scaleChange, 0.001f, 1000.0f);

        // Adjust offset so the current mouse pos stays centered
        const float timelineSpace = -myHorizontalOffset;
        const float mousePos_TimelineSpace = glm::max(0.0f, ToLocalPos(ImGui::GetMousePos()).x - timelineSpace);
        const float mouseAlongFrame = overallTimelineWidth < FLT_EPSILON ? 0.0f : mousePos_TimelineSpace / overallTimelineWidth;

        const float newTimelineSize = static_cast<float>(overallTimelineDuration * myTimeToPixelScale);
        const float newMousePos_TimelineSpace = mouseAlongFrame * newTimelineSize;

        myHorizontalOffset += newMousePos_TimelineSpace - mousePos_TimelineSpace;
      }
    }
  }
//---------------------------------------------------------------------------//
  void ProfilerWindow::RenderRuler(float64 aMinStartTime)
  {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImVec2 initialCursorPos = ImGui::GetCursorPos();

    const char* mainMarkerTimeUnitLabel = kTimeUnitLabels[0];
    float mainMarkerDurationMs = kTimeUnitToMsFactors[0];
    int mainMarkerUnitIdx = 0;
    while (mainMarkerDurationMs * myTimeToPixelScale > 500 && mainMarkerUnitIdx < ARRAYSIZE(kMsToTimeUnitFactors) - 1)
    {
      mainMarkerDurationMs /= 10.0f;
      if (mainMarkerDurationMs <= kTimeUnitToMsFactors[mainMarkerUnitIdx])
      {
        ++mainMarkerUnitIdx;
        mainMarkerDurationMs = kTimeUnitToMsFactors[mainMarkerUnitIdx];
        mainMarkerTimeUnitLabel = kTimeUnitLabels[mainMarkerUnitIdx];
      }
    }

    const float64 timeOffset = glm::max(0.0f, myHorizontalOffset / myTimeToPixelScale);
    const int mainMarkerIndex = static_cast<int>(glm::floor((aMinStartTime + timeOffset) / mainMarkerDurationMs));
    const float64 firstMarkerTime = static_cast<float>(mainMarkerIndex) * mainMarkerDurationMs;
    const float64 mainMarkerSize = mainMarkerDurationMs * myTimeToPixelScale;
    const float subMarkerSize = mainMarkerSize / 10.0f;

    const float kLabelVerticalOffset = 5.0f;

    // First time label above the ruler to show the passed time
    ImVec2 posGlobal = ToGlobalPos(ImGui::GetCursorPos());
    const char* firstTimeLabel = GetTimeLabel(firstMarkerTime);
    const ImVec2 firstTimeLabelPos(posGlobal.x, posGlobal.y);
    window->DrawList->AddText(firstTimeLabelPos, kRulerMarkerColor, firstTimeLabel);
    posGlobal.y += kLabelVerticalOffset * 4.0f;

    const float expectedLabelSize = 40.0f;
    const int labelFrequency = glm::max(1, (int)glm::ceil(expectedLabelSize / mainMarkerSize));

    float64 currMainMarkerTime = firstMarkerTime;
    posGlobal.x = ImGui::GetWindowPos().x + (currMainMarkerTime - aMinStartTime) * myTimeToPixelScale - myHorizontalOffset;
    const float windowEndGlobalX = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() + mainMarkerSize;
    int labelIndex = -(mainMarkerIndex % labelFrequency);
    while (posGlobal.x < windowEndGlobalX)
    {
      const ImVec2 markerLineStartG = posGlobal;
      const ImVec2 markerLineEndG(markerLineStartG.x, markerLineStartG.y + kRulerMarkerVerticalSize);

      // Small submarkers
      if (subMarkerSize > 2)
      {
        for (uint iSub = 0u; iSub < 9; ++iSub)
        {
          const ImVec2 subLineStartGlobal(markerLineStartG.x + (iSub + 1) * subMarkerSize, markerLineStartG.y + kRulerSubMarkerVerticalOffset);
          const ImVec2 subLineEndGlobal(subLineStartGlobal.x, markerLineEndG.y - 1.0f);
          window->DrawList->AddLine(subLineStartGlobal, subLineEndGlobal, kRulerMarkerColor, kRulerSubMarkerThickness);
        }
      }

      // Big main markers and label
      if (posGlobal.x > ImGui::GetWindowPos().x)
      {
        window->DrawList->AddLine(markerLineStartG, markerLineEndG, kRulerMarkerColor, kRulerMainMarkerThickness);

        if (labelIndex == 0)
        {
          labelIndex -= labelFrequency;
          const char* label = FormatString("%d%s", (int)(currMainMarkerTime - firstMarkerTime), mainMarkerTimeUnitLabel);
          const float textWidth = ImGui::CalcTextSize(label).x;
          const ImVec2 labelPos(markerLineEndG.x - textWidth * 0.5f, markerLineEndG.y + kLabelVerticalOffset);
          window->DrawList->AddText(labelPos, kRulerMarkerColor, label);
        }

        ++labelIndex;
      }

      currMainMarkerTime += mainMarkerDurationMs;
      posGlobal.x += mainMarkerSize;
    }

    ImGui::SetCursorPos(ImVec2(initialCursorPos.x, (posGlobal.y - ImGui::GetWindowPos().y) + kRulerMarkerVerticalSize + 20.0f));
  }
//---------------------------------------------------------------------------//
  void ProfilerWindow::RenderTimelines(float64 aMinStartTime, float64 aMaxEndTime, uint& aFirstRenderedFrame, uint& aLastRenderedFrame)
  {
    const ImVec2 timelineRectSize(ImGui::GetWindowWidth(), (ImGui::GetWindowHeight() * kFrameGraphHeightScale) / Profiler::TIMELINE_NUM);
    const char* timelineNames[Profiler::TIMELINE_NUM] =
    {
      "Main",
      "GPU"
    };

    uint firstWindowFrame = UINT_MAX;
    uint lastWindowFrame = UINT_MAX;
    for (uint iTimeline = 0u; iTimeline < Profiler::TIMELINE_NUM; ++iTimeline)
    {
      ImGui::SetCursorPosX(0.0f);
      float timelineMinY = ImGui::GetCursorPosY();
      myIsShowingTimeline[iTimeline] = ImGui::TreeNode(timelineNames[iTimeline]);
      if (myIsShowingTimeline[iTimeline])
      {
        timelineMinY = ImGui::GetCursorPosY();
        const ImVec2 timelineRectMinLocal = ImGui::GetCursorPos();
        const ImVec2 timelineRectMinGlobal = ToGlobalPos(timelineRectMinLocal);
        ScrollAndScale(aMinStartTime, aMaxEndTime, timelineRectMinGlobal.x, timelineRectMinGlobal.y, timelineRectMinGlobal.x + timelineRectSize.x, timelineRectMinGlobal.y + timelineRectSize.y);

        const Profiler::Timeline timeline = static_cast<Profiler::Timeline>(iTimeline);
        const CircularArray<Profiler::FrameData>& recordedFrames = Profiler::GetRecordedFrames(timeline);
        const CircularArray<Profiler::SampleNode>& recordedSamples = Profiler::GetRecordedSamples(timeline);

        for (uint i = 0; i < recordedFrames.Size(); ++i)
        {
          const Profiler::FrameData& frameData = recordedFrames[i];
          if (!frameData.myHasValidTimes)
            continue;

          const float frameMinX = (frameData.myStart.myTime - aMinStartTime) * myTimeToPixelScale - myHorizontalOffset;
          if (frameMinX > ImGui::GetWindowWidth())
            break;

          const float frameSize = frameData.myDuration * myTimeToPixelScale;
          if (frameMinX + frameSize < 0)
            continue;

          if (iTimeline == myFocusedTimeline)
          {
            if (firstWindowFrame == UINT_MAX)
              firstWindowFrame = i;

            lastWindowFrame = i;
          }

          ImGui::SetCursorPos(ImVec2(frameMinX, timelineMinY));
          RenderFrameBoundary(timelineRectSize.y);
          RenderFrameHeader(frameSize, timelineRectSize.y, frameData);

          if (frameData.myNumSamples > 0u)
          {
            ASSERT(frameData.myFirstSample != UINT_MAX);
            const Profiler::SampleNode& node = recordedSamples[frameData.myFirstSample];
            RenderSampleRecursive(node, myTimeToPixelScale, aMinStartTime, myHorizontalOffset, ImGui::GetCursorPosY(), 0, timeline);
          }

          ImGui::SetCursorPos(ImVec2(frameMinX + frameSize, timelineMinY));
          RenderFrameBoundary(timelineRectSize.y);
        }  // end frames

        ImGui::SetCursorPosY(timelineMinY + timelineRectSize.y);

        ImGui::TreePop();
      }  // end tree node
    }  // end timelines

    aFirstRenderedFrame = firstWindowFrame;
    aLastRenderedFrame = lastWindowFrame;
  }
//---------------------------------------------------------------------------//
  void ProfilerWindow::Render()
  {
    ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToVec4Color(kWindowBgColor));
    ImGui::Begin("Profiler");

    if (!Profiler::GetRecordedFrames((Profiler::Timeline)0).IsEmpty())
    {
      float64 minStartTime, maxEndTime;
      GetTimeRange(minStartTime, maxEndTime);
      const float maxOffset = static_cast<float>(glm::max(0.0, (maxEndTime - minStartTime) * myTimeToPixelScale - ImGui::GetWindowWidth() - 1.0f));

      if (!myIsPaused)
        myHorizontalOffset = maxOffset;

      RenderRuler(minStartTime);

      uint firstRenderedFrame, lastRenderedFrame = 0u;

      RenderTimelines(minStartTime, maxEndTime, firstRenderedFrame, lastRenderedFrame);

      ImGui::SliderFloat("Frames", &myHorizontalOffset, 0.0f, maxOffset, "%.1f");
      if (ImGui::Checkbox("Pause", &myIsPaused))
        Profiler::ourPauseRequested = myIsPaused;
      ImGui::Separator();
      //RenderFrameTimeGraph(firstRenderedFrame, lastRenderedFrame, 100.0, timelineInfos[myFocusedTimeline].myOverallDuration, (Profiler::Timeline) myFocusedTimeline);  
    }

    ImGui::End();
    ImGui::PopStyleColor();
  }
//---------------------------------------------------------------------------//
}
