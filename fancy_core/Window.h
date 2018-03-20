#pragma once

#include "FancyCorePrerequisites.h"
#include "Slot.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct WindowParameters
  {
    WindowParameters() : myWidth(0), myHeight(0), myCaptureMouse(true) {}
    String myTitle;
    uint myWidth;
    uint myHeight;
    bool myCaptureMouse;
  };
//---------------------------------------------------------------------------//
  class Window
  {
    public:
      static SharedPtr<Window> Create(HINSTANCE anInstanceHandle, const WindowParameters& someParams);
      
      ~Window();

      HWND GetWindowHandle() const { return myWindowHandle; }
      uint GetWidth() const { return myWidth; }
      uint GetHeight() const { return myHeight; }
      void SetSize(uint aWidth, uint aHeight);

      Slot<void(uint, uint)> myOnResize;
      Slot<void(UINT, WPARAM, LPARAM, bool*)> myWindowEventHandler;

      LRESULT HandleWindowEvent(UINT message, WPARAM wParam, LPARAM lParam);

    private:
      explicit Window(HWND aHandle);

      HWND myWindowHandle;
      uint myHeight;
      uint myWidth;
  };
//---------------------------------------------------------------------------//
}
