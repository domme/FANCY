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
      FRAME_POOL_SIZE = 1000,
      SAMPLE_POOL_SIZE = 10000,
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
      uint myChild = UINT_MAX;
      uint myNext = UINT_MAX;
    };

    struct FrameData
    {
      float64 myStart = 0.0;
      float64 myDuration = 0.0;
      uint myFirstSample = UINT_MAX;
      uint myNext = UINT_MAX;
    };


    struct ScopedMarker
    {
      ScopedMarker(const char* aName, uint8 aTag);
      ~ScopedMarker();
    };
  
    uint PushMarker(const char* aName, uint8 aTag);
    uint PopMarker();

    void Init();
    void BeginFrame();
    void EndFrame();

    const SampleNode* GetSample(uint aSampleId);
    const SampleNodeInfo* GetSampleInfo(uint64 anInfoId);
    const SampleNode& GetData();
    void SetPaused(bool aPause);
  };

#define PROFILE_FUNCTION(...) Profiling::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}


