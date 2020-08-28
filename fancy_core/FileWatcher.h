#pragma once

#include "FancyCoreDefines.h"
#include "Slot.h"

#include "EASTL/vector.h"

namespace Fancy {
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
    FileWatcher();
    virtual ~FileWatcher();

    void AddFileWatch(const eastl::string& aPath) const;
    void RemoveFileWatch(const eastl::string& aPath);

    Slot<void(const eastl::string&)> myOnFileUpdated;
    Slot<void(const eastl::string&)> myOnFileDeletedMoved;

  private:
    void UpdateFileInfos();
    
    mutable eastl::vector<FileWatchEntry> myWatchEntries;
  };
//---------------------------------------------------------------------------//
}
