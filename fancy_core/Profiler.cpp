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

    recordedSamples.Add({ 0u, 0u, 0.0, hash, aTimeline != TIMELINE_GPU, UINT_MAX, UINT_MAX });
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
    if (recordedFrames.IsEmpty() || locNextGpuFrameToUpdate.myIndex == UINT_MAX)
      return;

    const uint timeStampDataSize = RenderCore::GetQueryTypeDataSize(GpuQueryType::TIMESTAMP);
    ASSERT(timeStampDataSize == sizeof(uint64)); // Code below assumes this...

    const float64 timeTicksToMs[] = {
       (float64)RenderCore::GetTimestampToSecondsFactor(CommandListType::Graphics) * 1000.0,
       (float64)RenderCore::GetTimestampToSecondsFactor(CommandListType::Compute) * 1000.0
    };

    const auto ReadQueryTime = [timeStampDataSize, &timeTicksToMs](const uint8* aQueryDataBuf, TimeSample& aSample)
    {
      uint64 timeStampData = 0u;
      memcpy(&timeStampData, aQueryDataBuf + aSample.myQueryInfo.myIndex * timeStampDataSize, timeStampDataSize);
      aSample.myTime = static_cast<float64>(timeStampData) * timeTicksToMs[aSample.myQueryInfo.myCommandListType];
    };

    const uint frameIdx = recordedFrames.GetElementIndex(locNextGpuFrameToUpdate);
    FrameData& frame = recordedFrames[frameIdx];
    auto& recordedSamples = ourRecordedSamples[TIMELINE_GPU];

    if (RenderCore::IsFrameDone(frame.myFrame))
    {
      const uint8* queryData = nullptr;
      if (RenderCore::BeginQueryDataReadback(GpuQueryType::TIMESTAMP, frame.myFrame, &queryData))
      {
        ASSERT(!frame.myHasValidTimes);  // There must be something wrong with locNextGpuFrameToUpdate 

        ReadQueryTime(queryData, frame.myStart);
        ReadQueryTime(queryData, frame.myEnd);
        frame.myDuration = frame.myEnd.myTime - frame.myStart.myTime;
        frame.myHasValidTimes = true;

        const uint nextFrameFirstSample = frameIdx + 1 < recordedFrames.Size() ? recordedSamples.GetElementIndex(recordedFrames[frameIdx + 1].myFirstSample) : recordedSamples.Size();
        const uint firstSample = recordedSamples.GetElementIndex(frame.myFirstSample);

        for (uint sampleIdx = firstSample; sampleIdx < nextFrameFirstSample; ++sampleIdx)
        {
          SampleNode& sample = recordedSamples[sampleIdx];
          ASSERT(!sample.myHasValidTimes);

          ReadQueryTime(queryData, sample.myStart);
          ReadQueryTime(queryData, sample.myEnd);
          sample.myDuration = sample.myEnd.myTime - sample.myStart.myTime;
          sample.myHasValidTimes = true;
        }

        locNextGpuFrameToUpdate = frameIdx + 1 < recordedFrames.Size() ? recordedFrames.GetHandle(frameIdx + 1) : FrameHandle();
        RenderCore::EndQueryDataReadback(GpuQueryType::TIMESTAMP);
      }
    }
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
    newSample.myStart.myQueryInfo.myIndex = timestamp.myIndexInHeap;
    newSample.myStart.myQueryInfo.myCommandListType = (uint) timestamp.myCommandListType;
  }
//---------------------------------------------------------------------------//
  void Profiler::PopGpuMarker(CommandContext* aContext)
  {
    if (locPaused)
      return;

    SampleNode& closedSample = CloseMarker(TIMELINE_GPU);

    ASSERT(aContext->IsOpen());
    const GpuQuery timestamp = aContext->InsertTimestamp();
    closedSample.myEnd.myQueryInfo.myIndex = timestamp.myIndexInHeap;
    closedSample.myEnd.myQueryInfo.myCommandListType = (uint)timestamp.myCommandListType;
  }
//---------------------------------------------------------------------------//
  void Profiler::BeginFrame()
  {
    locPaused = ourPauseRequested;
    if (locPaused)
      return;

    FrameData& currFrame = locCurrFrame[TIMELINE_MAIN];
    currFrame.myFirstSample.myIndex = UINT_MAX;
    currFrame.myDuration = 0u;
    currFrame.myFrame = Time::ourFrameIdx;
    currFrame.myEnd = { 0u };
    currFrame.myHasValidTimes = true;
    currFrame.myStart.myTime = SampleTimeMs();
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
    UpdateGpuDurations();

    if (locPaused)
      return;

    FrameData& currFrame = locCurrFrame[TIMELINE_GPU];
    currFrame.myFirstSample.myIndex = UINT_MAX;
    currFrame.myDuration = 0u;
    currFrame.myFrame = Time::ourFrameIdx;
    currFrame.myEnd = { 0u };
    currFrame.myHasValidTimes = false;

    CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
    const GpuQuery timestamp = ctx->InsertTimestamp();
    RenderCore::GetCommandQueue(CommandListType::Graphics)->ExecuteContext(ctx);
    RenderCore::FreeContext(ctx);

    currFrame.myStart.myQueryInfo.myIndex = timestamp.myIndexInHeap;
    currFrame.myStart.myQueryInfo.myCommandListType = (uint) timestamp.myCommandListType;
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

      CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
      const GpuQuery timestamp = ctx->InsertTimestamp();
      RenderCore::GetCommandQueue(CommandListType::Graphics)->ExecuteContext(ctx);
      RenderCore::FreeContext(ctx);

      currFrame.myEnd.myQueryInfo.myIndex = timestamp.myIndexInHeap;
      currFrame.myEnd.myQueryInfo.myCommandListType = (uint) timestamp.myCommandListType;
      
      recordedFrames.Add(currFrame);

      if (locNextGpuFrameToUpdate.myIndex == UINT_MAX)
        locNextGpuFrameToUpdate = recordedFrames.GetHandle(recordedFrames.Size() - 1);
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
