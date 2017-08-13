#pragma once

class ImDrawData;

namespace Fancy {
    class FancyRuntime;
    class RenderWindow;
}

bool ImGui_ImplFancy_Init(Fancy::RenderWindow* aRenderWindow, Fancy::FancyRuntime* aRuntime);
void ImGui_ImplFancy_NewFrame();
void ImGui_ImplFancy_RenderDrawLists(ImDrawData* _draw_data);