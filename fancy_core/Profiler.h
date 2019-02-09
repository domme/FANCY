#pragma once

#include "FancyCoreDefines.h"
#include "CircularArray.h"

#include <unordered_map>

namespace Fancy
{
  struct Profiler
  {
    Profiler() = delete;
    ~Profiler() = delete;

    enum Consts
    {
      MAX_NAME_LENGTH = 128,
      FRAME_POOL_SIZE = 1 << 9,
      SAMPLE_POOL_SIZE = 1 << 14,
      MAX_SAMPLE_DEPTH = 2048,
    };

    struct SampleNodeInfo
    {
      char myName[MAX_NAME_LENGTH];
      uint8 myTag;
    };

    struct SampleNode
    {
      float64 myStart = 0.0;
      float64 myDuration = 0.0;
      uint64 myNodeInfo = 0u;
      CircularArray<SampleNode>::Handle myChild;
      CircularArray<SampleNode>::Handle myNext;
    };

    struct FrameData
    {
      uint64 myFrame = 0u;
      float64 myStart = 0.0;
      float64 myDuration = 0.0;
      CircularArray<SampleNode>::Handle myFirstSample;
    };

    using SampleHandle = CircularArray<SampleNode>::Handle;

    struct ScopedMarker
    {
      ScopedMarker(const char* aName, uint8 aTag);
      ~ScopedMarker();
    };
  
    static void PushMarker(const char* aName, uint8 aTag);
    static void PopMarker();

    static void Init();
    static void BeginFrame();
    static void EndFrame();

    static const CircularArray<FrameData>& GetRecordedFrames() { return ourRecordedFrames; }
    static const CircularArray<SampleNode>& GetRecordedSamples() { return ourRecordedSamples; }
    static const SampleNodeInfo& GetSampleInfo(uint64 anInfoId);

    static bool ourPauseRequested;

    static void FreeFirstFrame();

    static CircularArray<FrameData> ourRecordedFrames;
    static CircularArray<SampleNode> ourRecordedSamples;
    static std::unordered_map<uint64, SampleNodeInfo> ourNodeInfoPool;
  };

#define PROFILE_FUNCTION(...) Profiler::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}
