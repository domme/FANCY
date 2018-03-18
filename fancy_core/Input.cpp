#include "Input.h"
#include <Windowsx.h>

using namespace Fancy;

InputState::InputState()
  : myModifierKeyMask(0u)
  , myMouseBtnMask(0u)
  , myMousePos(0,0)
  , myMouseDelta(0,0)
  , myMouseWeelDelta(0.0f)
{
  memset(myKeyState, false, sizeof(myKeyState));
}

void InputState::OnWindowEvent(UINT message, WPARAM wParam, LPARAM lParam, bool* aWasHandled)
{
  switch (message)
  {
    case WM_KEYDOWN: HandleKeyUpDownEvent(wParam, true); break;
    case WM_KEYUP: HandleKeyUpDownEvent(wParam, false); break;

    case WM_MOUSEWHEEL:
      myMouseWeelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    break;
    case WM_MOUSEMOVE:
      glm::ivec2 newMousePos;
      newMousePos.x = GET_X_LPARAM(lParam);
      newMousePos.y = GET_Y_LPARAM(lParam);
      myMouseDelta = newMousePos - myMousePos;
      myMousePos = newMousePos;
    break;
  }
}

void InputState::HandleKeyUpDownEvent(WPARAM wParam, bool aDown)
{
  // "0..1" and "A..Z" from https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
  if (wParam >= 0x30 && wParam <= 0x5A)
  {
    myKeyState[wParam] = aDown;
    if (wParam >= 'A' && wParam <= 'Z')
      myKeyState[wParam + ('a' - 'A')] = aDown;
  }
  else
  {
    switch (wParam)
    {
    case VK_LBUTTON: myMouseBtnMask = (myMouseBtnMask & ~MOUSE_BTN_LEFT) | (MOUSE_BTN_LEFT & (uint)aDown); break;
    case VK_RBUTTON: myMouseBtnMask = (myMouseBtnMask & ~MOUSE_BTN_RIGHT) | (MOUSE_BTN_RIGHT & (uint)aDown); break;
    case VK_MBUTTON: myMouseBtnMask = (myMouseBtnMask & ~MOUSE_BTN_MIDDLE) | (MOUSE_BTN_MIDDLE & (uint)aDown); break;
    
    case VK_SHIFT: myModifierKeyMask = (myModifierKeyMask & ~MOD_KEY_SHIFT) | (MOD_KEY_SHIFT & (uint) aDown); break;
    case VK_MENU: myModifierKeyMask = (myModifierKeyMask & ~MOD_KEY_ALT) | (MOD_KEY_ALT & (uint)aDown); break;
    case VK_CONTROL: myModifierKeyMask = (myModifierKeyMask & ~MOD_KEY_CTRL) | (MOD_KEY_CTRL & (uint)aDown); break;
    }
  }
}
