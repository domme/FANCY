#ifndef INCLUDE_LOADABLEOBJECT
#define INCLUDE_LOADABLEOBJECT

#include "FancyCorePrerequisites.h"

class DLLEXPORT LoadableObject
{
  public:
    LoadableObject() : m_bIsInitialized(false) {}
    virtual ~LoadableObject() {};
    bool init();
    bool destroy();
    bool isInitialized() { return m_bIsInitialized; }

  protected:
    virtual bool _init() = 0;
    virtual bool _destroy() = 0;

    bool m_bIsInitialized;
};


#endif  // INCLUDE_LOADABLEOBJECT