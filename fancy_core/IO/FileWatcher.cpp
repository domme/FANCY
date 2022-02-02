#include "fancy_core_precompile.h"
#include "FileWatcher.h"
#include "Common/FancyCoreDefines.h"
#include "Common/TimeManager.h"
#include "Common/Fancy.h"
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

    eastl::vector<eastl::string> watchedPaths;
    watchedPaths.reserve(myWatchEntries.size());
    for (const FileWatchEntry& entry : myWatchEntries)
      watchedPaths.push_back(entry.myPath);

    for (const eastl::string& path : watchedPaths)
      RemoveFileWatch(path);
  }
//---------------------------------------------------------------------------//
  void FileWatcher::AddFileWatch(const eastl::string& aPath) const
  {
    if (std::find_if(myWatchEntries.begin(), myWatchEntries.end(), [=](const auto& entry){ return entry.myPath == aPath; }) != myWatchEntries.end())
      return;

    const uint64 currWriteTime = Path::GetFileWriteTime(aPath);

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
  void FileWatcher::RemoveFileWatch(const eastl::string& aPath)
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
      const uint64 currWriteTime = Path::GetFileWriteTime(entry.myPath);

      if (entry.myLastWriteTime < currWriteTime)
        myOnFileUpdated(entry.myPath);
      else if (currWriteTime == 0u && entry.myLastWriteTime != 0u)
        myOnFileDeletedMoved(entry.myPath);

      entry.myLastWriteTime = currWriteTime;
    }
  }
//---------------------------------------------------------------------------//
}
