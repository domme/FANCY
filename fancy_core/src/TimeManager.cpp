
#include "TimeManager.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  static float locUpdateFrequencies[(uint32) TimedUpdateInterval::NUM] =
  {
    0.0f,
    1.0f,
  };
//---------------------------------------------------------------------------//
  Time::Time()
    : myTimeScale(1.0f)
    , myElapsedTime(0.0f)
    , myDerivedDeltaTime(0.0f)
  {
    memset(myLastUpdateTimes, sizeof(myLastUpdateTimes), 0u);
  }
//---------------------------------------------------------------------------//
  Time::~Time()
  {

  }
//---------------------------------------------------------------------------//
  void Time::Update( float fDt )
  {
    myDerivedDeltaTime = fDt * myTimeScale;
    myElapsedTime += myDerivedDeltaTime;

    for (uint32 i = 0u; i < (uint32)TimedUpdateInterval::NUM; ++i)
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

