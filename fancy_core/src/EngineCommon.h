#ifndef INCLUDE_TIMEMANAGER_H
#define INCLUDE_TIMEMANAGER_H

#include "FancyCorePrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class EngineCommon 
  {
  //---------------------------------------------------------------------------//
  public:
    ~EngineCommon();

    static bool initEngine();
    
  private:
    EngineCommon();
  };
//---------------------------------------------------------------------------//
} // end of namespace Fancy


#endif  // INCLUDE_TIMEMANAGER_H
