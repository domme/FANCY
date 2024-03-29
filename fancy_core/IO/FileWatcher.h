#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/Slot.h"

#include "EASTL/vector.h"

namespace Fancy {
  class Time;

  //---------------------------------------------------------------------------//
  struct FileWatchEntry
  {
    eastl::string myPath;
    uint64 myLastWriteTime;
  };
  //---------------------------------------------------------------------------//
  class FileWatcher
  {
  public:
    FileWatcher(const SharedPtr<Time>& aTimeClock);
    virtual ~FileWatcher();

    void AddFileWatch(const eastl::string& aPath) const;
    void RemoveFileWatch(const eastl::string& aPath);

    Slot<void(const eastl::string&)> myOnFileUpdated;
    Slot<void(const eastl::string&)> myOnFileDeletedMoved;

  private:
    void UpdateFileInfos();
    
    mutable eastl::vector<FileWatchEntry> myWatchEntries;
    SharedPtr<Time> myClock;
  };
//---------------------------------------------------------------------------//
}
