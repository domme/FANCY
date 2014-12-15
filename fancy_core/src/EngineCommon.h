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
    static void shutdownEngine();
        
  private:
    EngineCommon();

    static void initComponentSubsystem();
    static void initRenderingSubsystem();
    static void initIOsubsystem();
  };
//---------------------------------------------------------------------------//
} // end of namespace Fancy

#endif  // INCLUDE_ENGINECOMMON_H
