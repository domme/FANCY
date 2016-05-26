#pragma once

#include <windows.h>
#include "FancyCorePrerequisites.h"
#include "ScopedPtr.h"

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
      void AddResizeCallback(const std::function<void(uint, uint)>& aCallback);

    private:
      
      static std::vector<SharedPtr<RenderWindow>> ourCreatedWindows;
      static LRESULT CALLBACK OnWindowEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

      LRESULT HandleWindowEvent(UINT message, WPARAM wParam, LPARAM lParam);

      HWND myWindowHandle;
      uint32 myHeight;
      uint32 myWidth;

      std::vector<std::function<void(uint, uint)>> myResizeCallbacks;
  };
//---------------------------------------------------------------------------//
}
