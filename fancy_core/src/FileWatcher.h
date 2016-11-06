#pragma once

#include "FancyCorePrerequisites.h"

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
    virtual ~FileWatcher();

    static uint64 GetFileWriteTime(const String& aFile);

    void AddFileWatch(const String& aPath) const;
    void RemoveFileWatch(const String& aPath);

  protected:
    virtual void OnFileUpdated(const String& aFile) = 0;
    virtual void OnFileDeletedMoved(const String& aFile) = 0;

  private:
    void UpdateFileInfos();
    
    mutable std::vector<FileWatchEntry> myWatchEntries;
  };
//---------------------------------------------------------------------------//
}
