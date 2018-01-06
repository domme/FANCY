#pragma once

#include "FancyCorePrerequisites.h"
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
  class RenderWindow
  {
    public:
      static SharedPtr<RenderWindow> Create(HINSTANCE anInstanceHandle, const WindowParameters& someParams);
      
      ~RenderWindow();

      HWND GetWindowHandle() const { return myWindowHandle; }
      uint GetWidth() const { return myWidth; }
      uint GetHeight() const { return myHeight; }
      void SetSize(uint aWidth, uint aHeight);

      Slot<void(uint, uint)> myOnResize;
      Slot<void(UINT, WPARAM, LPARAM, bool*)> myWindowEventHandler;

      LRESULT HandleWindowEvent(UINT message, WPARAM wParam, LPARAM lParam);

    private:
      explicit RenderWindow(HWND aHandle);

      HWND myWindowHandle;
      uint myHeight;
      uint myWidth;
  };
//---------------------------------------------------------------------------//
}
