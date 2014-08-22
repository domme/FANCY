
#include "TimeManager.h"

namespace Fancy { namespace Core {
//---------------------------------------------------------------------------//
  TimeManager::TimeManager() :
    m_uCurrentFrameIndex(0u)
  {

  }
//---------------------------------------------------------------------------//
  TimeManager::~TimeManager()
  {

  }
//---------------------------------------------------------------------------//
  void TimeManager::update( float fDt )
  {
    ++m_uCurrentFrameIndex;
  }
//---------------------------------------------------------------------------//
 
} }  // end of namespace Fancy::Core

