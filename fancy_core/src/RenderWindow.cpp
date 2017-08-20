#include "RenderWindow.h"
#include "Fancy.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  std::vector<RenderWindow*> ourCreatedWindows;
//---------------------------------------------------------------------------//
  LRESULT CALLBACK locOnWindowEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    auto it = std::find_if(ourCreatedWindows.begin(), ourCreatedWindows.end(),
      [hWnd](auto wndIt) { return wndIt->GetWindowHandle() == hWnd; });

    LRESULT handled = S_FALSE;
    
    if (it != ourCreatedWindows.end())
      handled = (*it)->HandleWindowEvent(message, wParam, lParam);

    if (handled == S_OK)
      return handled;

    return DefWindowProc(hWnd, message, wParam, lParam);
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  RenderWindow::RenderWindow(HWND aHandle)
    : myWindowHandle(aHandle)
    , myWidth(1)
    , myHeight(1)
  {
    
  }
//---------------------------------------------------------------------------//
  RenderWindow::~RenderWindow()
  {
    auto it = std::find_if(ourCreatedWindows.begin(), ourCreatedWindows.end(),
      [=](auto wndIt) { return wndIt->GetWindowHandle() == myWindowHandle; });

    if (it != ourCreatedWindows.end())
    {
      ourCreatedWindows.erase(it);
    }
  }
//---------------------------------------------------------------------------//
  LRESULT RenderWindow::HandleWindowEvent(UINT message, WPARAM wParam, LPARAM lParam)
  {
    bool handledExternally = false;
    myWindowEventHandler(message, wParam, lParam, &handledExternally);

    if (handledExternally)
      return S_OK;

    // Handle destroy/shutdown messages.
    switch (message)
    {
      case WM_DESTROY:
        PostQuitMessage(0);
        return S_OK;

      case WM_SIZE:
      {
        uint width = LOWORD(lParam);
        uint height = HIWORD(lParam);
        SetSize(width, height);
        return S_OK;
      }
    }

    return S_FALSE; // This window doesn't handle this message
  }
//---------------------------------------------------------------------------//
  SharedPtr<RenderWindow> RenderWindow::Create(HINSTANCE anInstanceHandle, const WindowParameters& someParams)
  {
    ASSERT(anInstanceHandle != nullptr);

    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &locOnWindowEvent;
    windowClass.hInstance = anInstanceHandle;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "WindowClass1";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(someParams.myWidth), static_cast<LONG>(someParams.myHeight) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    HWND windowHandle = CreateWindowEx(NULL,
      "WindowClass1",
      "Fancy",
      WS_OVERLAPPEDWINDOW,
      300,
      300,
      windowRect.right - windowRect.left,
      windowRect.bottom - windowRect.top,
      NULL,		// We have no parent window, NULL.
      NULL,		// We aren't using menus, NULL.
      anInstanceHandle,
      NULL);		// We aren't using multiple windows, NULL.

    ASSERT(windowHandle != nullptr);

    SharedPtr<RenderWindow> window(new RenderWindow(windowHandle));
    window->myWidth = someParams.myWidth;
    window->myHeight = someParams.myHeight;
    ourCreatedWindows.push_back(window.get());

    ShowWindow(window->myWindowHandle, 10);

    return window;
  }
//---------------------------------------------------------------------------//
  void RenderWindow::SetSize(uint aWidth, uint aHeight)
  {
    if (myWidth == aWidth && myHeight == aHeight)
      return;

    myWidth = aWidth;
    myHeight = aHeight;

    myOnResize(aWidth, aHeight);
  }
//---------------------------------------------------------------------------//
}
