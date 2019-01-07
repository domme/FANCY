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
  int offset = vsnprintf(TextBuf, ARRAY_LENGTH(TextBuf), aFmt, args);
  TextBuf[offset + 1] = '\0';
  va_end(args);
  return TextBuf;
}

bool ColorButton(const ImVec4& aColor, const ImVec2& aSize, const char* aLabel)
{
  ImGui::PushStyleColor(ImGuiCol_Button, aColor);
  bool pressed = ImGui::Button(aLabel, aSize);
  ImGui::PopStyleColor(1);
  return pressed;
}

void ProfilerWindow::Show()
{
  ImGuiTextBuffer textBuffer;

  ImGui::Begin("Profiler");

  const float zoom = 1.0f;

  const float64 frameDuration = Profiling::GetLastFrameDuration();
  const float64 durationToPixel = (float64)ImGui::GetWindowSize().x / (frameDuration * zoom);
  const int elementHeight = (int) (20 * zoom);

  ImVec2 size(0, elementHeight);
  size.x = frameDuration * durationToPixel;
  ColorButton(ImVec4(0.4f, 0.4f, 0.4f, 0.5f), size, FormatString("Frame: %f", (float)frameDuration));

  const DynamicArray<Profiling::SampleNode>& frameSamples = Profiling::GetLastFrameSamples();
  for (const Profiling::SampleNode& rootSample : frameSamples)
  {
      
  }
  ImGui::End();
}
