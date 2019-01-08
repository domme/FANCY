#pragma once

#include "FancyCoreDefines.h"
#include "FC_String.h"
#include "DynamicArray.h"

namespace Fancy
{
  namespace Profiling
  {
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
  
    void PushMarker(const char* aName, uint8 aTag);
    void PopMarker();

    void BeginFrame();
    void EndFrame();
    void SetPause(bool aPause);

    const DynamicArray<SampleNode>& GetLastFrameSamples();
    float64 GetLastFrameStart();
    float64 GetLastFrameDuration();
  };

#define PROFILE_FUNCTION(...) Profiling::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}


