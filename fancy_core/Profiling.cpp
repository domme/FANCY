#include "fancy_core_precompile.h"
#include "Profiling.h"

#include <chrono>
#include <ratio>

// DEBUG:
#include <sstream>

namespace Fancy
{
  static const uint kFrameHistorySize = 5;
  static DynamicArray<Profiling::SampleNode> ourSampleTrees[kFrameHistorySize];
  static float64 ourFrameStart[kFrameHistorySize]{ 0u };
  static float64 ourFrameDuration[kFrameHistorySize]{ 0u };
  static uint ourCurrIdx = 0u;
  static bool ourIsPaused = false;
  static Profiling::SampleNode* ourCurrNode = nullptr;
//---------------------------------------------------------------------------//
  Profiling::ScopedMarker::ScopedMarker(const char* aName, uint8 aTag)
  {
    // TODO: Check if the current name is already in use on the stack (e.g. prevent multiple PROFILE_FUNCTION calls in the same function?
    Profiling::PushMarker(aName, aTag);
  }
  //---------------------------------------------------------------------------//
  Profiling::ScopedMarker::~ScopedMarker()
  {
    Profiling::PopMarker();
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  static float64 SampleTimeMs()
  {
    const std::chrono::duration<float64, std::milli> now(std::chrono::system_clock::now().time_since_epoch());
    return now.count();
  }
//---------------------------------------------------------------------------//
  void Profiling::PushMarker(const char* aName, uint8 aTag)
  {
    Profiling::SampleNode node{ SampleTimeMs(), 0u, aTag, aName, nullptr };

    if (ourCurrNode != nullptr)
    {
      node.myParent = ourCurrNode;
      ourCurrNode->myChildren.push_back(node);
      ourCurrNode = &ourCurrNode->myChildren.back();
    }
    else
    {
      ourSampleTrees[ourCurrIdx].push_back(node);
      ourCurrNode = &ourSampleTrees[ourCurrIdx].back();
    }
  }
//---------------------------------------------------------------------------//
  void Profiling::PopMarker()
  {
    ASSERT(ourCurrNode != nullptr);

    ourCurrNode->myDuration = SampleTimeMs() - ourCurrNode->myStart;
    ourCurrNode = ourCurrNode->myParent;
  }
//---------------------------------------------------------------------------//
  void Profiling::BeginFrame()
  {
    if (!ourIsPaused)
    {
      ourSampleTrees[ourCurrIdx].clear();
      ourCurrNode = nullptr;
      ourFrameDuration[ourCurrIdx] = 0u;
      ourFrameStart[ourCurrIdx] = SampleTimeMs();
    }
  }
//---------------------------------------------------------------------------//
  void Profiling::EndFrame()
  {
    if (!ourIsPaused)
    {
      ourFrameDuration[ourCurrIdx] = SampleTimeMs() - ourFrameStart[ourCurrIdx];
      ourCurrIdx = (ourCurrIdx + 1) % kFrameHistorySize;
    }
  }
//---------------------------------------------------------------------------//
  void Profiling::SetPause(bool aPause)
  {
    ourIsPaused = aPause;
  }
//---------------------------------------------------------------------------//
  const DynamicArray<Profiling::SampleNode>& Profiling::GetLastFrameSamples()
  {
    return ourSampleTrees[(ourCurrIdx + kFrameHistorySize - 1) % kFrameHistorySize];
  }
//---------------------------------------------------------------------------//
  float64 Profiling::GetLastFrameStart()
  {
    return ourFrameStart[(ourCurrIdx + kFrameHistorySize - 1) % kFrameHistorySize];
  }
//---------------------------------------------------------------------------//
  float64 Profiling::GetLastFrameDuration()
  {
    return ourFrameDuration[(ourCurrIdx + kFrameHistorySize - 1) % kFrameHistorySize];
  }
//---------------------------------------------------------------------------//
  

}


