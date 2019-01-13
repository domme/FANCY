#pragma once

#include "FancyCoreDefines.h"

#include <list>

namespace Fancy
{
  namespace Profiling
  {
    enum Consts
    {
      MAX_NAME_LENGTH = 128,
      MAX_NUM_CHILDREN = 16,
      MAX_NUM_FRAME_SAMPLES = 2048,
      MAX_NODE_POOL_MB = 100,
    };

    struct SampleNodeInfo
    {
      char myName[MAX_NAME_LENGTH];
      uint8 myTag;
    };

    struct SampleNode
    {
      float64 myStart = 0u;
      float64 myDuration = 0u;
      uint64 myNodeInfo = 0u;
      uint myChildren[MAX_NUM_CHILDREN] = { 0u };
      uint8 myNumChildren = 0u;
    };

    struct FrameData
    {
      float64 myStart = 0u;
      float64 myDuration = 0u;
      uint mySamples[MAX_NUM_FRAME_SAMPLES] = { 0u };
      uint myNumSamples = 0u;
    };

    struct ScopedMarker
    {
      ScopedMarker(const char* aName, uint8 aTag);
      ~ScopedMarker();
    };
  
    void PushMarker(const char* aName, uint8 aTag);
    void PopMarker();

    void Init();
    void BeginFrame();
    void EndFrame();

    const SampleNode* GetSample(uint aSampleId);
    const SampleNodeInfo* GetSampleInfo(uint64 anInfoId);
    const std::list<FrameData>& GetFrames();
  };

#define PROFILE_FUNCTION(...) Profiling::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}


