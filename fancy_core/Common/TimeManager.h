#pragma once

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

      void Update(float fDt);
      float GetDelta() const {return myDerivedDeltaTime; }

      float GetTimeScale() const {return myTimeScale;}
      void SetTimeScale(float _timeScale) {myTimeScale = _timeScale;}

      float GetElapsed() const {return myElapsedTime;}

      Slot<void()>& GetTimedUpdateSlot(TimedUpdateInterval anInterval) { return myOnTimeIntervalElapsed[(uint)anInterval]; }

      static uint64 ourFrameIdx;

    private:
      float myElapsedTime;
      float myTimeScale;
      float myDerivedDeltaTime;
      float myLastUpdateTimes[(uint)TimedUpdateInterval::NUM];
      
      Slot<void()> myOnTimeIntervalElapsed[(uint) TimedUpdateInterval::NUM];
  };
//---------------------------------------------------------------------------//
}  // end of namespace Fancy