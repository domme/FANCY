#pragma once

#include "FancyCorePrerequisites.h"
#include "Slot.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct FileWatchEntry
  {
    String myPath;
    uint64 myLastWriteTime;
  };
  //---------------------------------------------------------------------------//
  class FileWatcher
  {
  public:
    FileWatcher();
    ~FileWatcher();

    static uint64 GetFileWriteTime(const String& aFile);

    void AddFileWatch(const String& aPath) const;
    void RemoveFileWatch(const String& aPath);

    Slot<void(const String&)> myOnFileUpdated;
    Slot<void(const String&)> myOnFileDeletedMoved;

  private:
    void UpdateFileInfos();
    
    mutable std::vector<FileWatchEntry> myWatchEntries;
  };
//---------------------------------------------------------------------------//
}
