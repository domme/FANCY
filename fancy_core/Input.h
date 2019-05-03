#pragma once

#include "FancyCoreDefines.h"
#include <windef.h>

namespace Fancy {
//---------------------------------------------------------------------------//
  struct InputState
  {
    InputState();
    void OnWindowEvent(UINT message, WPARAM wParam, LPARAM lParam, bool* aWasHandled);

    enum MODIFIER_KEY
    {
     MOD_KEY_CTRL   = 1 << 0,
     MOD_KEY_ALT    = 1 << 1,
     MOD_KEY_SHIFT  = 1 << 2
    };

    enum MOUSE_BTN
    {
      MOUSE_BTN_LEFT = 1 << 0,
      MOUSE_BTN_RIGHT = 1 << 1,
      MOUSE_BTN_MIDDLE = 1 << 2
    };

    uint myModifierKeyMask;
    uint myMouseBtnMask;
    glm::ivec2 myMousePos;
    glm::ivec2 myMouseDelta;
    float myMouseWeelDelta;
    bool myKeyState[127];
    bool myLastKeyState[127];

  private:
    void HandleKeyUpDownEvent(WPARAM wParam, bool aDown);
  };
//---------------------------------------------------------------------------//
}

