#ifndef INCLUDE_TIMEMANAGER_H
#define INCLUDE_TIMEMANAGER_H

#include "FancyCorePrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class Time 
  {
    public:
      static void update(float fDt);
      static uint32 getCurrentFrameIndex() { return m_uCurrentFrameIndex; }
      static float getDeltaTime() {return m_fCurrDerivedDt; }

      static float getTimeScale() {return m_fTimeScale;}
      static void setTimeScale(float _timeScale) {m_fTimeScale = _timeScale;}

      static float getElapsedTime() {return m_fElapsedTime;}

    private:
      Time();
      ~Time();

      static uint32 m_uCurrentFrameIndex;
      static float m_fElapsedTime;
      static float m_fTimeScale;
      static float m_fCurrDerivedDt;
  };
//---------------------------------------------------------------------------//
}  // end of namespace Fancy


#endif  // INCLUDE_TIMEMANAGER_H