#ifndef INCLUDE_ENGINECOMMON_H
#define INCLUDE_ENGINECOMMON_H

#include "FancyCorePrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class DLLEXPORT EngineCommon
  {
  //---------------------------------------------------------------------------//
  public:
    ~EngineCommon();

    static bool initEngine();
        
  private:
    EngineCommon();

    static void initComponentSubsystem();
    static void initRenderingSubsystem();
  };
//---------------------------------------------------------------------------//
} // end of namespace Fancy

#endif  // INCLUDE_ENGINECOMMON_H
