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
  case WM_MBUTTONDOWN: myMouseBtnMask |= MOUSE_BTN_MIDDLE; break;
  case WM_LBUTTONDOWN: myMouseBtnMask |= MOUSE_BTN_LEFT; break;
  case WM_RBUTTONDOWN: myMouseBtnMask |= MOUSE_BTN_RIGHT; break;
  case WM_SYSKEYDOWN:
  case WM_KEYDOWN: HandleKeyUpDownEvent(wParam, true); break;

  case WM_MBUTTONUP: myMouseBtnMask &= ~MOUSE_BTN_MIDDLE; break;
  case WM_LBUTTONUP: myMouseBtnMask &= ~MOUSE_BTN_LEFT; break;
  case WM_RBUTTONUP: myMouseBtnMask &= ~MOUSE_BTN_RIGHT; break;
  case WM_SYSKEYUP:
  case WM_KEYUP: HandleKeyUpDownEvent(wParam, false); break;

  case WM_MOUSEWHEEL:
    myMouseWeelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    break;
  case WM_MOUSEMOVE:
  {
    glm::ivec2 newMousePos;
    newMousePos.x = GET_X_LPARAM(lParam);
    newMousePos.y = GET_Y_LPARAM(lParam);
    myMouseDelta = newMousePos - myMousePos;
    myMousePos = newMousePos;
  }
  break;
  case WM_MOUSELEAVE:
    myMouseDelta = glm::ivec2(0, 0);
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
      case VK_SHIFT: myModifierKeyMask = aDown ? myModifierKeyMask | MOD_KEY_SHIFT : (myModifierKeyMask & ~MOD_KEY_SHIFT); break;
      case VK_MENU: myModifierKeyMask = aDown ? myModifierKeyMask | MOD_KEY_ALT : (myModifierKeyMask & ~MOD_KEY_ALT); break;
      case VK_CONTROL: myModifierKeyMask = aDown ? (myModifierKeyMask | MOD_KEY_CTRL) : (myModifierKeyMask & ~MOD_KEY_CTRL); break;
    }
  }
}
