
#include "TimeManager.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Internal
  {

  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  uint32 Time::m_uCurrentFrameIndex = 0u;
  float Time::m_fTimeScale = 1.0f;
  float Time::m_fElapsedTime = 0.0f;
  float Time::m_fCurrDerivedDt = 0.0f;
//---------------------------------------------------------------------------//
  Time::Time()
  {

  }
//---------------------------------------------------------------------------//
  Time::~Time()
  {

  }
//---------------------------------------------------------------------------//
  void Time::update( float fDt )
  {
    ++m_uCurrentFrameIndex;
    m_fCurrDerivedDt = fDt * m_fTimeScale;
    m_fElapsedTime += m_fCurrDerivedDt;
  }
//---------------------------------------------------------------------------//
 
}  // end of namespace Fancy

