#include "fancy_core_precompile.h"
#include "Window.h"
#include "CommandLine.h"

#include "Debug/Log.h"

#include "EASTL/vector.h"
#include "EASTL/algorithm.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  eastl::vector<Window*> ourCreatedWindows;
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
  Window::Window(HWND aHandle)
    : myWindowHandle(aHandle)
    , myHeight(1)
    , myWidth(1)
  {
    
  }
//---------------------------------------------------------------------------//
  Window::~Window()
  {
    auto it = eastl::find_if(ourCreatedWindows.begin(), ourCreatedWindows.end(),
      [=](auto wndIt) { return wndIt->GetWindowHandle() == myWindowHandle; });

    if (it != ourCreatedWindows.end())
    {
      ourCreatedWindows.erase(it);
    }

    if (GetCapture() == myWindowHandle)
      ReleaseCapture();
  }
//---------------------------------------------------------------------------//
  LRESULT Window::HandleWindowEvent(UINT message, WPARAM wParam, LPARAM lParam)
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
  SharedPtr<Window> Window::Create(HINSTANCE anInstanceHandle, const WindowParameters& someParams)
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

    eastl::string title = someParams.myTitle;

    if (CommandLine::GetInstance()->HasArgument("vulkan") || CommandLine::GetInstance()->HasArgument("vk"))
      title += " - Vulkan";
    else
      title += " - DX12";

    // Create the window and store a handle to it.
    HWND windowHandle = CreateWindowEx(NULL,
      "WindowClass1",
      title.c_str(),
      WS_OVERLAPPEDWINDOW,
      100,
      100,
      windowRect.right - windowRect.left,
      windowRect.bottom - windowRect.top,
      NULL,		// We have no parent window, NULL.
      NULL,		// We aren't using menus, NULL.
      anInstanceHandle,
      NULL);		// We aren't using multiple windows, NULL.

    ASSERT(windowHandle != nullptr);

    if (someParams.myCaptureMouse)
      SetCapture(windowHandle);

    SharedPtr<Window> window(new Window(windowHandle));
    window->myWidth = someParams.myWidth;
    window->myHeight = someParams.myHeight;
    ourCreatedWindows.push_back(window.get());

    ShowWindow(window->myWindowHandle, 10);

    return window;
  }
//---------------------------------------------------------------------------//
  void Window::SetSize(uint aWidth, uint aHeight)
  {
    if (myWidth == aWidth && myHeight == aHeight)
      return;

    myWidth = aWidth;
    myHeight = aHeight;

    myOnResize(aWidth, aHeight);
  }
//---------------------------------------------------------------------------//
}
