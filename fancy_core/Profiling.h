#pragma once

#include "FancyCoreDefines.h"
#include "FC_String.h"
#include "DynamicArray.h"

namespace Fancy
{
  class Profiling
  {
  public:
    struct SampleNode
    {
      float64 myStart;
      float64 myDuration;
      uint8 myTag = 0;
      String myName;
      SampleNode* myParent = nullptr;
      DynamicArray<SampleNode> myChildren;
    };

    struct ScopedMarker
    {
      ScopedMarker(const char* aName, uint8 aTag);
      ~ScopedMarker();
    };
  
    static void PushMarker(const char* aName, uint8 aTag);
    static void PopMarker();

    static void BeginFrame();
    static void EndFrame();

    static const DynamicArray<SampleNode>& GetLastFrameSamples() { return ourSampleTrees[(ourCurrIdx + kFrameHistorySize - 1) % kFrameHistorySize]; }
    static float64 GetLastFrameStart() { return ourFrameStart[(ourCurrIdx + kFrameHistorySize - 1) % kFrameHistorySize]; }
    static float64 GetLastFrameDuration() { return ourFrameDuration[(ourCurrIdx + kFrameHistorySize - 1) % kFrameHistorySize]; }
    
  private:
    Profiling() = default;
    ~Profiling() = default;

    enum
    {
      kFrameHistorySize = 5
    };

    static DynamicArray<SampleNode> ourSampleTrees[kFrameHistorySize];
    static float64 ourFrameStart[kFrameHistorySize];
    static float64 ourFrameDuration[kFrameHistorySize];
    static uint ourCurrIdx;

    static SampleNode* ourCurrNode;
  };

#define PROFILE_FUNCTION(...) Profiling::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}


