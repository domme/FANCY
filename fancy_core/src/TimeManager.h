#ifndef INCLUDE_TIMEMANAGER_H
#define INCLUDE_TIMEMANAGER_H

#include "FancyCorePrerequisites.h"
#include "Callback.h"
#include "Slot.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class TimedUpdateInterval
  {
    PER_FRAME = 0,
    PER_SECOND_REALTIME,
    PER_SECOND_GAMETIME,

    NUM
  };
//---------------------------------------------------------------------------//
  class Time 
  {
    public:
      Time();
      ~Time();

      void Update(float fDt);
      float GetDelta() const {return myDerivedDeltaTime; }

      float GetTimeScale() const {return myTimeScale;}
      void SetTimeScale(float _timeScale) {myTimeScale = _timeScale;}

      float GetElapsed() const {return myElapsedTime;}

      Slot<void()>& GetTimedUpdateSlot(TimedUpdateInterval anInterval) { return myOnTimeIntervalElapsed[(uint32)anInterval]; }

    private:
      float myElapsedTime;
      float myTimeScale;
      float myDerivedDeltaTime;

      Slot<void()> myOnTimeIntervalElapsed[(uint32) TimedUpdateInterval::NUM];
      float myLastUpdateTimes[(uint32)TimedUpdateInterval::NUM];
  };
//---------------------------------------------------------------------------//
}  // end of namespace Fancy


#endif  // INCLUDE_TIMEMANAGER_H