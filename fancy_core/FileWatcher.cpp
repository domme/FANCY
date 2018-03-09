#include "FileWatcher.h"
#include "FancyCorePrerequisites.h"
#include "TimeManager.h"
#include "Fancy.h"
#include "PathService.h"

namespace Fancy {
//---------------------------------------------------------------------------//  
  FileWatcher::FileWatcher()
  {
    Time& realTimeClock = FancyRuntime::GetInstance()->GetRealTimeClock();
    realTimeClock.GetTimedUpdateSlot(TimedUpdateInterval::PER_SECOND_REALTIME).Connect(this, &FileWatcher::UpdateFileInfos);
  }
//---------------------------------------------------------------------------//
  FileWatcher::~FileWatcher()
  {
    FancyRuntime* runtime = Fancy::FancyRuntime::GetInstance();
    if(runtime != nullptr)
        runtime->GetRealTimeClock().GetTimedUpdateSlot(TimedUpdateInterval::PER_SECOND_REALTIME).DetachObserver(this);

    std::vector<String> watchedPaths;
    watchedPaths.reserve(myWatchEntries.size());
    for (const FileWatchEntry& entry : myWatchEntries)
      watchedPaths.push_back(entry.myPath);

    for (const String& path : watchedPaths)
      RemoveFileWatch(path);
  }
//---------------------------------------------------------------------------//
  void FileWatcher::AddFileWatch(const String& aPath) const
  {
    if (std::find_if(myWatchEntries.begin(), myWatchEntries.end(), [=](const auto& entry){ return entry.myPath == aPath; }) != myWatchEntries.end())
      return;

    uint64 currWriteTime = Path::GetFileWriteTime(aPath);

    ASSERT(currWriteTime > 0u, "Could not read file write time");
    if (currWriteTime == 0u)
    {
      return;
    }

    FileWatchEntry entry;
    entry.myPath = aPath;
    entry.myLastWriteTime = currWriteTime;
    myWatchEntries.push_back(entry);
  }
//---------------------------------------------------------------------------//
  void FileWatcher::RemoveFileWatch(const String& aPath)
  {
    auto it = std::find_if(myWatchEntries.begin(), myWatchEntries.end(), [=](const auto& entry) { return entry.myPath == aPath; });

    if ( it != myWatchEntries.end())
      myWatchEntries.erase(it);
  }
//---------------------------------------------------------------------------//
  void FileWatcher::UpdateFileInfos()
  {
    for(FileWatchEntry& entry : myWatchEntries)
    {
      uint64 currWriteTime = Path::GetFileWriteTime(entry.myPath);

      if (entry.myLastWriteTime < currWriteTime)
        myOnFileUpdated(entry.myPath);
      else if (currWriteTime == 0u && entry.myLastWriteTime != 0u)
        myOnFileDeletedMoved(entry.myPath);

      entry.myLastWriteTime = currWriteTime;
    }
  }
//---------------------------------------------------------------------------//
}
