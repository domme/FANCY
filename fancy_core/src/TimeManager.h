#ifndef INCLUDE_TIMEMANAGER_H
#define INCLUDE_TIMEMANAGER_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Core {
//---------------------------------------------------------------------------//
  class TimeManager 
  {
    public:
      static TimeManager& getInstance() {static TimeManager instance; return instance;}

      void update(float fDt);
      uint32 getCurrentFrameIndex() const { return m_uCurrentFrameIndex; }

    private:
      TimeManager();
      ~TimeManager();

      uint32 m_uCurrentFrameIndex;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Core


#endif  // INCLUDE_TIMEMANAGER_H