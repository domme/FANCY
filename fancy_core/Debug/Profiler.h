#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/CircularArray.h"

#include <unordered_map>

namespace Fancy
{
  class CommandList;

  struct Profiler
  {
    Profiler() = delete;
    ~Profiler() = delete;

    enum Consts
    {
      MAX_NAME_LENGTH = 128,
      FRAME_POOL_SIZE = 1 << 11,
      SAMPLE_POOL_SIZE = 1 << 14,
      MAX_SAMPLE_DEPTH = 2048,
    };

    enum Timeline
    {
      TIMELINE_MAIN = 0,
      TIMELINE_GPU,
      TIMELINE_NUM
    };

    struct SampleNodeInfo
    {
      char myName[MAX_NAME_LENGTH];
      uint16 myTag;
    };

    struct GpuQueryInfo
    {
      uint myIndex;
      uint myCommandListType;
    };

    union TimeSample
    {
      float64 myTime;
      GpuQueryInfo myQueryInfo;
    };

    struct SampleNode
    {
      TimeSample myStart = { 0.0 };
      TimeSample myEnd = { 0.0 };
      float64 myDuration = { 0u };
      uint64 myNodeInfo = 0u;
      bool myHasValidTimes = true;
      CircularArray<SampleNode>::Handle myChild;
      CircularArray<SampleNode>::Handle myNext;
    };

    struct FrameData
    {
      uint64 myFrame = 0u;
      TimeSample myStart = { 0u };
      TimeSample myEnd = { 0u };
      float64 myDuration = 0.0;
      bool myHasValidTimes = true;
      uint myNumSamples = 0u;
      CircularArray<SampleNode>::Handle myFirstSample;
    };

    using SampleHandle = CircularArray<SampleNode>::Handle;
    using FrameHandle = CircularArray<FrameData>::Handle;

    struct ScopedMarker
    {
      ScopedMarker(const char* aName, uint16 aTag);
      ~ScopedMarker();
    };

    static SampleNode& OpenMarker(const char* aName, uint16 aTag, Timeline aTimeline);
    static SampleNode& CloseMarker(Timeline aTimeline);
  
    static void PushMarker(const char* aName, uint16 aTag);
    static void PopMarker();

    static void PushGpuMarker(CommandList* aCommandList, const char* aName, uint16 aTag);
    static void PopGpuMarker(CommandList* aCommandList);

    static void BeginFrame();
    static void EndFrame();

    static void BeginFrameGPU();
    static void EndFrameGPU();

    static const CircularArray<FrameData>& GetRecordedFrames(Timeline aTimeline) { return ourRecordedFrames[aTimeline]; }
    static const CircularArray<SampleNode>& GetRecordedSamples(Timeline aTimeline) { return ourRecordedSamples[aTimeline]; }
    static const SampleNodeInfo& GetSampleInfo(uint64 anInfoId);

    static bool ourPauseRequested;

    static void FreeFirstFrame(Timeline aTimeline);

    static CircularArray<FrameData> ourRecordedFrames[TIMELINE_NUM];
    static CircularArray<SampleNode> ourRecordedSamples[TIMELINE_NUM];
    static std::unordered_map<uint64, SampleNodeInfo> ourNodeInfoPool;

  private:
    static void UpdateGpuDurations();
  };

#define PROFILE_FUNCTION(...) Profiler::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
#define PROFILE_FUNCTION_TAG(aTag, ...) Profiler::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, aTag)
#define GPU_BEGIN_PROFILE_FUNCTION_TAG(aCommandList, aTag, ...) Profiler::PushGpuMarker(aCommandList, __FUNCTION__, aTag)
#define GPU_BEGIN_PROFILE_FUNCTION(aCommandList, ...) Profiler::PushGpuMarker(aCommandList, __FUNCTION__, 0u)
#define GPU_BEGIN_PROFILE(aCommandList, aName, aTag) Profiler::PushGpuMarker(aCommandList, aName, aTag)
#define GPU_END_PROFILE(aCommandList) Profiler::PopGpuMarker(aCommandList)
}
