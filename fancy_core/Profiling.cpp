#include "fancy_core_precompile.h"
#include "Profiling.h"

#include <chrono>
#include <ratio>

// DEBUG:
#include <sstream>

namespace Fancy
{
//---------------------------------------------------------------------------//
  Profiling::SampleNode* Profiling::ourCurrNode = nullptr;
  DynamicArray<Profiling::SampleNode> Profiling::ourSampleTrees[Profiling::kFrameHistorySize];
  float64 Profiling::ourFrameStart[Profiling::kFrameHistorySize]{ 0u };
  float64 Profiling::ourFrameDuration[Profiling::kFrameHistorySize]{ 0u };
  uint Profiling::ourCurrIdx = 0u;
//---------------------------------------------------------------------------//
  namespace Profiling_Priv
  {
    float64 locSampleTimeMs()
    {
      const std::chrono::duration<float64, std::milli> now(std::chrono::system_clock::now().time_since_epoch());
      return now.count();
    }
  }
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
  void Profiling::PushMarker(const char* aName, uint8 aTag)
  {
    Profiling::SampleNode node{ Profiling_Priv::locSampleTimeMs(), 0u, aTag, aName, nullptr };

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

    ourCurrNode->myDuration = Profiling_Priv::locSampleTimeMs() - ourCurrNode->myStart;
    ourCurrNode = ourCurrNode->myParent;
  }
//---------------------------------------------------------------------------//
  void Profiling::BeginFrame()
  {
    ourSampleTrees[ourCurrIdx].clear();
    ourCurrNode = nullptr;
    ourFrameDuration[ourCurrIdx] = 0u;
    ourFrameStart[ourCurrIdx] = Profiling_Priv::locSampleTimeMs();
  }
//---------------------------------------------------------------------------//
  void Profiling::EndFrame()
  {
    ourFrameDuration[ourCurrIdx] = Profiling_Priv::locSampleTimeMs() - ourFrameStart[ourCurrIdx];
    ourCurrIdx = (ourCurrIdx + 1) % kFrameHistorySize;
  }
//---------------------------------------------------------------------------//
}


