#include "RenderWindow.h"
#include "Fancy.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  std::vector<SharedPtr<RenderWindow>> RenderWindow::ourCreatedWindows;  
//---------------------------------------------------------------------------//
  LRESULT RenderWindow::OnWindowEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    auto it = std::find_if(ourCreatedWindows.begin(), ourCreatedWindows.end(),
      [hWnd](auto wndIt) { return (*wndIt)->myWindowHandle == hWnd; });

    if (it == ourCreatedWindows.end())
      return 0;

    return (*it)->HandleWindowEvent(message, wParam, lParam);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  RenderWindow::RenderWindow() :
    myWindowHandle(nullptr)
  {

  }
//---------------------------------------------------------------------------//
  RenderWindow::~RenderWindow()
  {
    auto it = std::find_if(ourCreatedWindows.begin(), ourCreatedWindows.end(),
      [=](auto wndIt) { return (*wndIt)->myWindowHandle == myWindowHandle; });

    if (it != ourCreatedWindows.end())
    {
      ourCreatedWindows.erase(it);
    }
  }
//---------------------------------------------------------------------------//
  LRESULT RenderWindow::HandleWindowEvent(UINT message, WPARAM wParam, LPARAM lParam)
  {
    // Handle destroy/shutdown messages.
    switch (message)
    {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(myWindowHandle, message, wParam, lParam);
  }
//---------------------------------------------------------------------------//
  SharedPtr<RenderWindow> RenderWindow::Create(const WindowParameters& someParams)
  {
    HINSTANCE instanceHandle = Fancy::GetAppInstanceHandle();
    ASSERT(instanceHandle != nullptr);

    SharedPtr<RenderWindow> window(new RenderWindow());
    ourCreatedWindows.push_back(window);
    
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &RenderWindow::OnWindowEvent;
    windowClass.hInstance = instanceHandle;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "WindowClass1";
    RegisterClassEx(&windowClass);

    window->myWidth = someParams.myWidth;
    window->myHeight = someParams.myHeight;

    RECT windowRect = { 0, 0, static_cast<LONG>(someParams.myWidth), static_cast<LONG>(someParams.myHeight) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    window->myWindowHandle = CreateWindowEx(NULL,
      "WindowClass1",
      "Fancy (DX12)",
      WS_OVERLAPPEDWINDOW,
      300,
      300,
      windowRect.right - windowRect.left,
      windowRect.bottom - windowRect.top,
      NULL,		// We have no parent window, NULL.
      NULL,		// We aren't using menus, NULL.
      instanceHandle,
      NULL);		// We aren't using multiple windows, NULL.

    ShowWindow(window->myWindowHandle, 1);

    return window;
  }
//---------------------------------------------------------------------------//
  void RenderWindow::AddResizeCallback(const std::function<void(uint, uint)>& aCallback)
  {
    
  }
//---------------------------------------------------------------------------//


  struct test
  {
    void OnResize(uint width, uint height)
    {
      
    }

    test()
    {
      RenderWindow* wnd = new RenderWindow();

      wnd->AddResizeCallback(std::)
    }
  };
}
