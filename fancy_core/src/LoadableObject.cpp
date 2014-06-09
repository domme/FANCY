#include "LoadableObject.h"

bool LoadableObject::init()
{
  if (!isInitialized())
  {
    // Call the virtual implementation
    m_bIsInitialized = _init();
  }

  return isInitialized();
}

bool LoadableObject::destroy()
{
  if (isInitialized())
  {
    // Call the virtual implementation
    m_bIsInitialized = _destroy();
  }

  return !isInitialized();
}