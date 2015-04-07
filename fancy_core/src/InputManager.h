#ifndef INCLUDE_INPUTMANAGER_H_
#define INCLUDE_INPUTMANAGER_H_

#include "FancyCorePrerequisites.h"
#include "FixedArray.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  enum class DLLEXPORT EKey
  {
    FORWARD = 0,
    BACKWARD,
    RIGHT,
    LEFT,
    UP,
    DOWN,
    MOUSE_LEFT_BUTTON,
    MOUSE_RIGHT_BUTTON,

    NUM
  };
//---------------------------------------------------------------------------//
  enum class DLLEXPORT EModifierKey
  {
    ALT = 0,
    LEFT_CTRL,
    LEFT_SHIFT,
    ALTGR,
    RIGHT_CTRL,
    RIGHT_SHIFT,

    NUM
  };
//---------------------------------------------------------------------------//
  class DLLEXPORT InputManager 
  {
    public:
      typedef FixedArray<EKey, (uint32) EKey::NUM> KeyList;
      typedef FixedArray<EModifierKey, (uint32) EModifierKey::NUM> ModifierKeyList;

      static const KeyList& getPressedKeys();
      static const ModifierKeyList& getPressedModifierKeys();
      static void setPressedKeys(const KeyList& _keyList);
      static void setPressedModifierKeys(const ModifierKeyList& _modifierKeyList);

      static bool isKeyDown(EKey _key);
      static bool isModifierDown(EModifierKey _modifier);

      static const glm::vec2& getMousePosition();
      static const glm::vec2& getMouseMovement();
      static void setMousePosition(const glm::vec2& _pos);

    private:
      InputManager() {};
      ~InputManager() {};
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO

#endif  // INCLUDE_INPUTMANAGER_H_