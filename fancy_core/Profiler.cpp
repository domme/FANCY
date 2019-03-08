#include "fancy_core_precompile.h"
#include "Profiler.h"
#include "MathUtil.h"
#include "CommandContext.h"
#include "CommandQueue.h"

#include <chrono>
#include <ratio>
#include "TimeManager.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace {
    bool locPaused = false;

    Profiler::FrameHandle locNextGpuFrameToUpdate = { 0u };
    uint locSampleDepth[Profiler::TIMELINE_NUM] = { 0u };
    Profiler::FrameData locCurrFrame[Profiler::TIMELINE_NUM];
    Profiler::SampleHandle locSampleStack[Profiler::TIMELINE_NUM][Profiler::MAX_SAMPLE_DEPTH];
    Profiler::SampleHandle locTailSampleStack[Profiler::TIMELINE_NUM][Profiler::MAX_SAMPLE_DEPTH];
  }

  bool Profiler::ourPauseRequested = false;
  CircularArray<Profiler::FrameData> Profiler::ourRecordedFrames[TIMELINE_NUM]{ FRAME_POOL_SIZE, FRAME_POOL_SIZE };
  CircularArray<Profiler::SampleNode> Profiler::ourRecordedSamples[TIMELINE_NUM]{ SAMPLE_POOL_SIZE, SAMPLE_POOL_SIZE };
  std::unordered_map<uint64, Profiler::SampleNodeInfo> Profiler::ourNodeInfoPool;
  CommandContext* Profiler::ourRenderContext = nullptr;
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
    PopMarker();
  }
//---------------------------------------------------------------------------//
  Profiler::SampleNode& Profiler::OpenMarker(const char* aName, uint8 aTag, Timeline aTimeline)
  {
    uint& sampleDepth = locSampleDepth[aTimeline];
    auto& recordedSamples = ourRecordedSamples[aTimeline];
    auto& sampleStack = locSampleStack[aTimeline];

    ASSERT(sampleDepth < MAX_SAMPLE_DEPTH);

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

    if (recordedSamples.IsFull())
      FreeFirstFrame(aTimeline);

    recordedSamples.Add({ 0u, 0.0, hash });
    sampleStack[sampleDepth++] = recordedSamples.GetHandle(recordedSamples.Size() - 1u);
    return recordedSamples[recordedSamples.Size() - 1u];
  }
//---------------------------------------------------------------------------//
  Profiler::SampleNode& Profiler::CloseMarker(Timeline aTimeline)
  {
    uint& sampleDepth = locSampleDepth[aTimeline];
    auto& recordedSamples = ourRecordedSamples[aTimeline];
    auto& sampleStack = locSampleStack[aTimeline];
    FrameData& currFrame = locCurrFrame[aTimeline];
    auto& tailSampleStack = locTailSampleStack[aTimeline];

    ASSERT(sampleDepth > 0u);

    const SampleHandle sampleHandle = sampleStack[--sampleDepth];
    SampleNode& sample = recordedSamples[sampleHandle];
    
    if (sampleDepth == 0 && currFrame.myFirstSample == UINT_MAX)  // First child of frame?
    {
      currFrame.myFirstSample = sampleHandle;
    }
    else if (sampleDepth > 0 &&
      recordedSamples[sampleStack[sampleDepth - 1]].myChild == UINT_MAX)  // First child of parent?
    {
      recordedSamples[sampleStack[sampleDepth - 1]].myChild = sampleHandle;
    }
    else
    {
      const SampleHandle lastSampleOnLevel = tailSampleStack[sampleDepth];
      recordedSamples[lastSampleOnLevel].myNext = sampleHandle;
    }
    tailSampleStack[sampleDepth] = sampleHandle;
    return sample;
  }
//---------------------------------------------------------------------------//
  void Profiler::FreeFirstFrame(Timeline aTimeline)
  {
    if (ourRecordedFrames[aTimeline].IsEmpty())
      return;

    ourRecordedFrames[aTimeline].RemoveFirstElement();

    const FrameData& nextFrame = ourRecordedFrames[aTimeline][0];
    while (ourRecordedSamples[aTimeline].GetHandle(0) != nextFrame.myFirstSample)
      ourRecordedSamples[aTimeline].RemoveFirstElement();
  }
//---------------------------------------------------------------------------//
  void Profiler::UpdateGpuDurations()
  {
    auto& recordedFrames = ourRecordedFrames[TIMELINE_GPU];
    auto& recordedSamples = ourRecordedSamples[TIMELINE_GPU];

    if (recordedFrames.IsEmpty() || locNextGpuFrameToUpdate.myIndex == UINT_MAX)
      return;

    FrameData& frame = recordedFrames[locNextGpuFrameToUpdate];


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

    SampleNode& newSample = OpenMarker(aName, aTag, TIMELINE_MAIN);
    newSample.myStart.myTime = SampleTimeMs();
  }
//---------------------------------------------------------------------------//
  void Profiler::PopMarker()
  {
    if (locPaused)
      return;

    SampleNode& closedSample = CloseMarker(TIMELINE_MAIN);

    closedSample.myEnd.myTime = SampleTimeMs();
    closedSample.myDuration = closedSample.myEnd.myTime - closedSample.myStart.myTime;
  }
//---------------------------------------------------------------------------//
  void Profiler::PushGpuMarker(CommandContext* aContext, const char* aName, uint8 aTag)
  {
    if (locPaused)
      return;

    SampleNode& newSample = OpenMarker(aName, aTag, TIMELINE_GPU);

    ASSERT(aContext->IsOpen());
    const GpuQuery timestamp = aContext->InsertTimestamp();
    newSample.myStart.myQueryIdx = timestamp.myIndexInHeap;
  }
//---------------------------------------------------------------------------//
  void Profiler::PopGpuMarker(CommandContext* aContext)
  {
    if (locPaused)
      return;

    SampleNode& closedSample = CloseMarker(TIMELINE_MAIN);

    ASSERT(aContext->IsOpen());
    const GpuQuery timestamp = aContext->InsertTimestamp();
    closedSample.myEnd.myQueryIdx = timestamp.myIndexInHeap;
  }
//---------------------------------------------------------------------------//
  void Profiler::BeginFrame()
  {
    if (ourRenderContext == nullptr)
      ourRenderContext = RenderCore::AllocateContext(CommandListType::Graphics);

    locPaused = ourPauseRequested;
    if (locPaused)
      return;

    for (uint i = 0u; i < TIMELINE_NUM; ++i)
    {
      FrameData& currFrame = locCurrFrame[i];
      currFrame.myFirstSample.myIndex = UINT_MAX;
      currFrame.myDuration = 0u;
      currFrame.myFrame = Time::ourFrameIdx;
      currFrame.myEnd = { 0u };

      if (i == TIMELINE_GPU)
      {
        const GpuQuery timestamp = ourRenderContext->InsertTimestamp();
        currFrame.myStart.myQueryIdx = timestamp.myIndexInHeap;
      }
      else
      {
        currFrame.myStart.myTime = SampleTimeMs();
      }
    }
  }
//---------------------------------------------------------------------------//
  void Profiler::EndFrame()
  {
    uint& sampleDepth = locSampleDepth[TIMELINE_MAIN];
    auto& recordedFrames = ourRecordedFrames[TIMELINE_MAIN];
    FrameData& currFrame = locCurrFrame[TIMELINE_MAIN];

    ASSERT(sampleDepth == 0, "There are still open profiling markers at the end of the frame");

    if (!locPaused && currFrame.myFirstSample != UINT_MAX)  // We have samples in this frame
    {
      if (recordedFrames.IsFull())
        FreeFirstFrame(TIMELINE_MAIN);
      
      currFrame.myEnd.myTime = SampleTimeMs();
      currFrame.myDuration = currFrame.myEnd.myTime - currFrame.myStart.myTime;
      recordedFrames.Add(currFrame);
    }
  }
//---------------------------------------------------------------------------//
  void Profiler::BeginGpuFrame()
  {
    if (ourRenderContext == nullptr)
      ourRenderContext = RenderCore::AllocateContext(CommandListType::Graphics);

    // TODO: Update the durations for the finished GPU-frames

    if (locPaused)
      return;

    FrameData& currFrame = locCurrFrame[TIMELINE_GPU];
    currFrame.myFirstSample.myIndex = UINT_MAX;
    currFrame.myDuration = 0u;
    currFrame.myFrame = Time::ourFrameIdx;
    currFrame.myEnd = { 0u };

    const GpuQuery timestamp = ourRenderContext->InsertTimestamp();
    currFrame.myStart.myQueryIdx = timestamp.myIndexInHeap;
  }
//---------------------------------------------------------------------------//
  void Profiler::EndGpuFrame()
  {
    if (locPaused)
      return;

    uint& sampleDepth = locSampleDepth[TIMELINE_GPU];
    auto& recordedFrames = ourRecordedFrames[TIMELINE_GPU];
    FrameData& currFrame = locCurrFrame[TIMELINE_GPU];

    ASSERT(sampleDepth == 0, "There are still open profiling markers at the end of the frame");

    if (currFrame.myFirstSample != UINT_MAX)  // We have samples in this frame
    {
      if (recordedFrames.IsFull())
        FreeFirstFrame(TIMELINE_GPU);

      const GpuQuery timestamp = ourRenderContext->InsertTimestamp();
      currFrame.myEnd.myQueryIdx = timestamp.myIndexInHeap;
      RenderCore::GetCommandQueue(CommandListType::Graphics)->ExecuteContext(ourRenderContext);

      recordedFrames.Add(currFrame);
    }
    else
    {
      ourRenderContext->Reset(0u);
    }
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  const Profiler::SampleNodeInfo& Profiler::GetSampleInfo(uint64 anInfoId)
  {
    const auto it = ourNodeInfoPool.find(anInfoId);
    ASSERT(it != ourNodeInfoPool.end());
    return it->second;
  }
//---------------------------------------------------------------------------//
}
