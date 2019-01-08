#include "ProfilerWindow.h"

#include <fancy_imgui/imgui.h>
#include <fancy_core/Profiling.h>
#include <fancy_core/DynamicArray.h>
#include <fancy_core/FancyCoreDefines.h>
#include <fancy_core/Log.h>

using namespace Fancy;

ProfilerWindow::ProfilerWindow()
{
}


ProfilerWindow::~ProfilerWindow()
{
}

char TextBuf[2048];

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

bool ColorButton(const char* aLabel, const ImVec2& aPos, const ImVec2& aSize, const ImVec4& aColor)
{
  ImGui::SetCursorPos(aPos);
  ImGui::PushStyleColor(ImGuiCol_Button, aColor);
  const bool pressed = ImGui::Button(aLabel, aSize);
  ImGui::PopStyleColor(1);
  return pressed;
}

ImVec4 GetColorForTag(uint8 aTag) 
{
  switch(aTag)
  {
  case 0: return ImVec4(0.7f, 0.0f, 0.0f, 0.5f);
  default: return ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
  }
}

struct RenderArgs
{
  float64 myFrameStart = 0.0;
  ImVec2 myStartPos = ImVec2(0,0);
  float myElementHeight = 20.0f;
  float myElementHeightWithPadding = 25.0f;
  float myDurationToPixelScale = 1.0f;
};

void RenderNodeRecursive(const Profiling::SampleNode& aNode, const RenderArgs& someArgs, int aDepth)
{
  ImVec2 pos, size;
  pos.x = someArgs.myStartPos.x + (aNode.myStart - someArgs.myFrameStart) * someArgs.myDurationToPixelScale;
  pos.y = someArgs.myStartPos.y + someArgs.myElementHeightWithPadding * aDepth;
  size.x = aNode.myDuration * someArgs.myDurationToPixelScale;
  size.y = someArgs.myElementHeight;

  ColorButton(FormatString("%s: %.3f", aNode.myName.c_str(), (float)aNode.myDuration), pos, size, GetColorForTag(aNode.myTag));

  for (const Profiling::SampleNode& childNode : aNode.myChildren)
  {
    RenderNodeRecursive(childNode, someArgs, aDepth + 1);
  }
}

void ProfilerWindow::Show()
{
  ImGuiTextBuffer textBuffer;

  ImGui::Begin("Profiler");

  if (ImGui::Checkbox("Pause", &myIsPaused))
    Profiling::SetPause(myIsPaused);

  ImGui::SliderFloat("Scale", &myScale, 0.1f, 10.0f);

  const float64 frameDuration = Profiling::GetLastFrameDuration();

  RenderArgs args;
  args.myFrameStart = Profiling::GetLastFrameStart();
  args.myDurationToPixelScale = (float)((float64)ImGui::GetWindowSize().x / frameDuration) * myScale;
  args.myElementHeight = (20.0f * myScale);
  args.myElementHeightWithPadding = (25.0f * myScale);
  args.myStartPos = ImGui::GetCursorPos();

  ImVec2 pos = args.myStartPos;
  ImVec2 size(0, args.myElementHeight);
  size.x = frameDuration * args.myDurationToPixelScale;
  ColorButton(FormatString("Frame: %.3f", (float)frameDuration), pos, size, ImVec4(0.4f, 0.4f, 0.4f, 0.5f));
  
  const DynamicArray<Profiling::SampleNode>& frameSamples = Profiling::GetLastFrameSamples();
  for (const Profiling::SampleNode& rootSample : frameSamples)
  {
    RenderNodeRecursive(rootSample, args, 1);
  }
    
  ImGui::End();
}
