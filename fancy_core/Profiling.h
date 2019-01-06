#pragma once

#include <chrono>
#include <ratio>

namespace Fancy
{
  class Profiling
  {
  public:
    struct SampleNode
    {
      std::chrono::system_clock::time_point myStart;
      std::chrono::system_clock::time_point myEnd;
      uint8 myTag = 0;
      String myName;
      SampleNode* myParent = nullptr;
      std::vector<SampleNode> myChildren;
    };

    struct ScopedMarker
    {
      ScopedMarker(const char* aName, uint8 aTag);
      ~ScopedMarker();
    };
  
    static void PushMarker(const char* aName, uint8 aTag);
    static void PopMarker();

    static const DynamicArray<SampleNode>& GetFrameSamples() { return ourSampleTrees; }

    // DEBUG:
    static void DebugPrint();
    static void DebugPrintRecursive(SampleNode* aNode, int anOffset);
    static void Clear();

  private:
    Profiling() = default;
    ~Profiling() = default;

    static SampleNode* ourCurrNode;
    static DynamicArray<SampleNode> ourSampleTrees;
  };

#define PROFILE_FUNCTION(...) Profiling::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}


