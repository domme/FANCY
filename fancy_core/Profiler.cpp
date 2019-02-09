#include "fancy_core_precompile.h"
#include "Profiler.h"
#include "MathUtil.h"

#include <chrono>
#include <ratio>
#include "TimeManager.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace {
    uint locSampleDepth = 0u;
    Profiler::FrameData locCurrFrame;
    bool locPaused = false;
    Profiler::SampleHandle locSampleStack[Profiler::MAX_SAMPLE_DEPTH];
    Profiler::SampleHandle locTailSampleStack[Profiler::MAX_SAMPLE_DEPTH];
  }

  bool Profiler::ourPauseRequested = false;
  CircularArray<Profiler::FrameData> Profiler::ourRecordedFrames(Profiler::FRAME_POOL_SIZE);
  CircularArray<Profiler::SampleNode> Profiler::ourRecordedSamples(Profiler::SAMPLE_POOL_SIZE);
  std::unordered_map<uint64, Profiler::SampleNodeInfo> Profiler::ourNodeInfoPool;
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  Profiler::ScopedMarker::ScopedMarker(const char* aName, uint8 aTag)
  {
    // TODO: Check if the current name is already in use on the stack (e.g. prevent multiple PROFILE_FUNCTION calls in the same function?
    Profiler::PushMarker(aName, aTag);
  }
  //---------------------------------------------------------------------------//
  Profiler::ScopedMarker::~ScopedMarker()
  {
    Profiler::PopMarker();
  }
//---------------------------------------------------------------------------//
  void Profiler::FreeFirstFrame()
  {
    if (ourRecordedFrames.IsEmpty())
      return;

    ourRecordedFrames.RemoveFirstElement();

    const FrameData& nextFrame = ourRecordedFrames[0];
    while (ourRecordedSamples.GetHandle(0) != nextFrame.myFirstSample)
      ourRecordedSamples.RemoveFirstElement();
  }
//---------------------------------------------------------------------------//
  static float64 SampleTimeMs()
  {
    const std::chrono::duration<float64, std::milli> now(std::chrono::system_clock::now().time_since_epoch());
    return now.count();
  }
//---------------------------------------------------------------------------//
  void Profiler::PushMarker(const char* aName, uint8 aTag)
  {
    if (locPaused)
      return;

    ASSERT(locSampleDepth < MAX_SAMPLE_DEPTH);

    const uint nameLen = static_cast<uint>(strlen(aName));
    ASSERT(nameLen <= MAX_NAME_LENGTH);

    uint64 hash = MathUtil::ByteHash(reinterpret_cast<const uint8*>(aName), nameLen);
    MathUtil::hash_combine(hash, aTag);
    if (ourNodeInfoPool.find(hash) == ourNodeInfoPool.end())
    {
      SampleNodeInfo& info = ourNodeInfoPool[hash];
      strcpy(info.myName, aName);
      info.myTag = aTag;
    }

    if (ourRecordedSamples.IsFull())
      FreeFirstFrame();

    ourRecordedSamples.Add({ SampleTimeMs(), 0.0, hash });
    locSampleStack[locSampleDepth++] = ourRecordedSamples.GetHandle(ourRecordedSamples.Size() - 1u);
  }
//---------------------------------------------------------------------------//
  void Profiler::PopMarker()
  {
    if (locPaused)
      return;

    ASSERT(locSampleDepth > 0u);

    const SampleHandle sampleHandle = locSampleStack[--locSampleDepth];
    SampleNode& sample = ourRecordedSamples[sampleHandle];
    sample.myDuration = SampleTimeMs() - sample.myStart;

    if (locSampleDepth == 0 && locCurrFrame.myFirstSample == UINT_MAX)  // First child of frame?
    {
      locCurrFrame.myFirstSample = sampleHandle;
    }
    else if (locSampleDepth > 0 && 
      ourRecordedSamples[locSampleStack[locSampleDepth - 1]].myChild == UINT_MAX)  // First child of parent?
    {
      ourRecordedSamples[locSampleStack[locSampleDepth - 1]].myChild = sampleHandle;
    }
    else
    {
      const SampleHandle lastSampleOnLevel = locTailSampleStack[locSampleDepth];
      ourRecordedSamples[lastSampleOnLevel].myNext = sampleHandle;
    }
    locTailSampleStack[locSampleDepth] = sampleHandle;
  }
//---------------------------------------------------------------------------//
  void Profiler::BeginFrame()
  {
    ASSERT(locSampleDepth == 0, "There are still open profiling markers at the end of the frame");

    // Finalize the last frame
    if (!locPaused && locCurrFrame.myFirstSample != UINT_MAX)  // We have samples in this frame
    {
      if (ourRecordedFrames.IsFull())
        FreeFirstFrame();

      locCurrFrame.myDuration = SampleTimeMs() - locCurrFrame.myStart;
      ourRecordedFrames.Add(locCurrFrame);
    }

    // Start the next frame
    locPaused = ourPauseRequested;
    if (!locPaused)
    {
      locCurrFrame.myFirstSample.myIndex = UINT_MAX;
      locCurrFrame.myDuration = 0u;
      locCurrFrame.myStart = SampleTimeMs();
      locCurrFrame.myFrame = Time::ourFrameIdx;
    }
  }
//---------------------------------------------------------------------------//
  void Profiler::EndFrame()
  {
    
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void Profiler::Init()
  {
    
  }
//---------------------------------------------------------------------------//
  const Profiler::SampleNodeInfo& Profiler::GetSampleInfo(uint64 anInfoId)
  {
    const auto it = ourNodeInfoPool.find(anInfoId);
    ASSERT(it != ourNodeInfoPool.end());
    return it->second;
  }
//---------------------------------------------------------------------------//
}
