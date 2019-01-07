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
  DynamicArray<Profiling::SampleNode> Profiling::ourSampleTrees;
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
      ourSampleTrees.push_back(node);
      ourCurrNode = &ourSampleTrees.back();
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
    ourSampleTrees.clear();
    ourCurrNode = nullptr;
    ourFrameDuration = 0u;
    ourFrameStart = Profiling_Priv::locSampleTimeMs();
  }
//---------------------------------------------------------------------------//
  void Profiling::EndFrame()
  {
    ourFrameDuration = Profiling_Priv::locSampleTimeMs() - ourFrameStart;
  }
//---------------------------------------------------------------------------//

  int locGetOffset(float64 aDuration)
  {
    return glm::clamp((int)(aDuration * 0.01), 1, 100);
  }

  void Profiling::DebugPrint()
  {
    for (int i = 0u, e = (int)ourSampleTrees.size(); i < e; ++i)
    {
      DebugPrintRecursive(&ourSampleTrees[i], 0);
    }
  }

  void Profiling::DebugPrintRecursive(SampleNode* aNode, int anOffset)
  {
    std::stringstream outStr;

    for (int i = 0; i < anOffset; ++i)
      outStr << " ";

    const int durationOffset = locGetOffset(aNode->myDuration);

    outStr << aNode->myName << ": " << aNode->myDuration << "ms";
    for (int i = 0u; i < durationOffset; ++i)
      outStr << "*";

    Log_Debug(outStr.str());

    int childOffset = anOffset;
    for (SampleNode& child : aNode->myChildren)
    {
      const int childDurationOffset = locGetOffset(child.myDuration);
      DebugPrintRecursive(&child, childOffset);
      childOffset += childDurationOffset;
    }
  }
//---------------------------------------------------------------------------//
}


