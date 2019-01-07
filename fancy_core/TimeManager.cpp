#include "fancy_core_precompile.h"
#include "TimeManager.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  uint64 Time::ourFrameIdx(0u);
//---------------------------------------------------------------------------//
  static float locUpdateFrequencies[(uint) TimedUpdateInterval::NUM] =
  {
    0.0f,
    1.0f,
  };
//---------------------------------------------------------------------------//
  Time::Time()
    : myElapsedTime(0.0f)
    , myTimeScale(1.0f)
    , myDerivedDeltaTime(0.0f)
    , myLastUpdateTimes{0u}
  {
  }
//---------------------------------------------------------------------------//
  void Time::Update( float fDt )
  {
    myDerivedDeltaTime = fDt * myTimeScale;
    myElapsedTime += myDerivedDeltaTime;

    for (uint i = 0u; i < (uint)TimedUpdateInterval::NUM; ++i)
    {
      if (myElapsedTime - myLastUpdateTimes[i] >= locUpdateFrequencies[i] - FLT_MIN)
      {
        myLastUpdateTimes[i] = myElapsedTime;
        myOnTimeIntervalElapsed[i]();
      }
    }
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy

