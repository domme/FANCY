#include "fancy_core_precompile.h"
#include "Profiling.h"

#include <sstream>

namespace Fancy
{
//---------------------------------------------------------------------------//
  Profiling::SampleNode* Profiling::ourCurrNode = nullptr;
  DynamicArray<Profiling::SampleNode> Profiling::ourSampleTrees;
//---------------------------------------------------------------------------//

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
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    Profiling::SampleNode node{ now, now, aTag, aName, nullptr };

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
    ourCurrNode->myEnd = std::chrono::system_clock::now();
    ourCurrNode = ourCurrNode->myParent;
  }
//---------------------------------------------------------------------------//
  void Profiling::Clear()
  {
    ourSampleTrees.clear();
    ourCurrNode = nullptr;
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

    const std::chrono::duration<float64, std::milli> dur(aNode->myEnd - aNode->myStart);
    const float64 duration = dur.count();
    const int durationOffset = locGetOffset(duration);

    outStr << aNode->myName << ": " << duration << "ms";
    for (int i = 0u; i < durationOffset; ++i)
      outStr << "*";

    Log_Debug(outStr.str());

    int childOffset = anOffset;
    for (SampleNode& child : aNode->myChildren)
    {
      const std::chrono::duration<float64, std::milli> childDur(child.myEnd - child.myStart);
      const int childDurationOffset = locGetOffset(childDur.count());
      DebugPrintRecursive(&child, childOffset);
      childOffset += childDurationOffset;
    }
  }
//---------------------------------------------------------------------------//
}


