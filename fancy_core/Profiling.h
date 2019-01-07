#pragma once

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
      std::vector<SampleNode> myChildren;
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

    static const DynamicArray<SampleNode>& GetFrameSamples() { return ourSampleTrees; }

    // DEBUG:
    static void DebugPrint();
    static void DebugPrintRecursive(SampleNode* aNode, int anOffset);
    
  private:
    Profiling() = default;
    ~Profiling() = default;

    static SampleNode* ourCurrNode;
    static DynamicArray<SampleNode> ourSampleTrees;
    static float64 ourFrameStart;
    static float64 ourFrameDuration;
  };

#define PROFILE_FUNCTION(...) Profiling::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}


