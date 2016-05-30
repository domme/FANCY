#pragma once

#include <windows.h>
#include "FancyCorePrerequisites.h"
#include "ScopedPtr.h"
#include "Slot.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct WindowParameters
  {
    String myTitle;
    uint myWidth;
    uint myHeight;
  };
//---------------------------------------------------------------------------//
  class DLLEXPORT RenderWindow
  {
    public:
      static SharedPtr<RenderWindow> Create(const WindowParameters& someParams);

      ~RenderWindow();

      HWND GetWindowHandle() const { return myWindowHandle; }
      uint32 GetWidth() const { return myWidth; }
      uint32 GetHeight() const { return myHeight; }

      Slot<void(uint, uint)> myOnResize;

    private:
      explicit RenderWindow(HWND aHandle);

      static std::vector<SharedPtr<RenderWindow>> ourCreatedWindows;
      static LRESULT CALLBACK OnWindowEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

      LRESULT HandleWindowEvent(UINT message, WPARAM wParam, LPARAM lParam);

      HWND myWindowHandle;
      uint32 myHeight;
      uint32 myWidth;
  };
//---------------------------------------------------------------------------//
}
