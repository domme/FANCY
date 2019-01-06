#include "ProfilerWindow.h"

#include <fancy_imgui/imgui.h>
#include <fancy_core/Profiling.h>
#include <fancy_core/DynamicArray.h>

using namespace Fancy;

ProfilerWindow::ProfilerWindow()
{
}


ProfilerWindow::~ProfilerWindow()
{
}

void ProfilerWindow::Show()
{
  const DynamicArray<Profiling::SampleNode>& frameSamples = Profiling::GetFrameSamples();

  ImGui::Begin("Profiler");
  for ()




}
