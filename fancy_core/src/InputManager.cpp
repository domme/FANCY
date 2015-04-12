//#include "InputManager.h"
//
//namespace Fancy { namespace IO {
////---------------------------------------------------------------------------//
//  namespace Internal 
//  {
//    InputManager::KeyList keyList;
//    InputManager::ModifierKeyList modifierKeyList;
//    glm::vec2 mousePosition;
//    glm::vec2 mouseMovement;
//  }
////---------------------------------------------------------------------------//
//
////---------------------------------------------------------------------------//
//  const InputManager::KeyList& InputManager::getPressedKeys()
//  {
//    return Internal::keyList;
//  }
////---------------------------------------------------------------------------//
//  const InputManager::ModifierKeyList& InputManager::getPressedModifierKeys()
//  {
//    return Internal::modifierKeyList;
//  }
////---------------------------------------------------------------------------//
//  void InputManager::setPressedKeys( const KeyList& _keyList )
//  {
//    Internal::keyList = _keyList;
//  }
////---------------------------------------------------------------------------//
//  void InputManager::setPressedModifierKeys( const ModifierKeyList& _modifierKeyList )
//  {
//    Internal::modifierKeyList = _modifierKeyList;
//  }
////---------------------------------------------------------------------------//
//  bool InputManager::isKeyDown( EKey _key )
//  {
//    return Internal::keyList.contains(_key);
//  }
////---------------------------------------------------------------------------//
//  bool InputManager::isModifierDown( EModifierKey _modifier )
//  {
//    return Internal::modifierKeyList.contains(_modifier);
//  }
////---------------------------------------------------------------------------//
//  const glm::vec2& InputManager::getMousePosition()
//  {
//    return Internal::mousePosition;
//  }
////---------------------------------------------------------------------------//
//  const glm::vec2& InputManager::getMouseMovement()
//  {
//    return Internal::mouseMovement;
//  }
////---------------------------------------------------------------------------//
//  void InputManager::setMousePosition(const glm::vec2& _pos)
//  {
//    Internal::mouseMovement = _pos - Internal::mousePosition;
//    Internal::mousePosition = _pos;
//  }
////---------------------------------------------------------------------------//
//} }  // end of namespace Fancy::IO
//
