#include "fancy_core_precompile.h"
#include "Profiling.h"
#include "MathUtil.h"

#include <chrono>
#include <ratio>
#include <stack>

namespace Fancy
{
  static std::unordered_map<uint64, Profiling::SampleNodeInfo> ourNodeInfoPool;
  static DynamicArray<Profiling::SampleNode> ourNodePool;
  
  static constexpr uint64 ourMaxNumNodes = static_cast<uint>((Profiling::MAX_NODE_POOL_MB * SIZE_MB) / sizeof(Profiling::SampleNode));
  static uint ourNextFreeNode = 0u;
  static uint ourNextUsedNode = ourMaxNumNodes;
  static uint ourMaxNumSamplesPerFrame = 0u;

  static bool ourPauseRequested = false;
  static bool ourPaused = false;

  static std::stack<uint> ourCurrStack;
  static uint ourChildrenTail = 0u;
  static uint ourLevelHead = 0u;
  static uint ourLevelTail = 0u;
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
  uint AllocateNode()
  {
    ASSERT(ourNextFreeNode < ourNextUsedNode, "Insufficient amount of free profiler nodes");
    return ourNextFreeNode++;
  }
//---------------------------------------------------------------------------//
  uint Profiling::PushMarker(const char* aName, uint8 aTag)
  {
    if (ourPaused)
      return UINT_MAX;

    const uint nameLen = static_cast<uint>(strlen(aName));
    ASSERT(nameLen <= MAX_NAME_LENGTH);

    uint64 hash = MathUtil::ByteHash(reinterpret_cast<const uint8*>(aName), nameLen);
    MathUtil::hash_combine(hash, aTag);
    if (ourNodeInfoPool.find(hash) == ourNodeInfoPool.end())
    {
      SampleNodeInfo& info = ourNodeInfoPool[hash];
      strcpy(info.myName, aName);
      info.myTag = aTag;
    }

    const uint nodeId = AllocateNode();
    Profiling::SampleNode& node = ourNodePool[nodeId];
    
    node.myNodeInfo = hash;
    node.myStart = SampleTimeMs();
    node.myDuration = 0u;
    node.myNext = UINT_MAX;
    node.myChild = UINT_MAX;
    node.myNumNodes = 0u;

    ourCurrStack.push(nodeId);

    return nodeId;
  }
//---------------------------------------------------------------------------//
  uint Profiling::PopMarker()
  {
    if (ourPaused)
      return UINT_MAX;

    ASSERT(!ourCurrStack.empty());

    const uint nodeId = ourCurrStack.top();
    ourCurrStack.pop();

    Profiling::SampleNode* node = &ourNodePool[nodeId];
    node->myDuration = SampleTimeMs() - node->myStart;

    if (ourCurrStack.empty())
    {
      // This is the frame-root
      ourNodePool[ourLevelTail].myNext = nodeId;
      ourLevelTail = nodeId;
    }
    else
    {
      const uint parentNodeId = ourCurrStack.top();
      Profiling::SampleNode* parent = &ourNodePool[parentNodeId];

      ASSERT((parent->myChild == UINT_MAX) == (ourChildrenTail == UINT_MAX));
      if (parent->myChild == UINT_MAX)
        parent->myChild = nodeId;
      else
        ourNodePool[ourChildrenTail].myNext = nodeId;

      ourChildrenTail = nodeId;
    }

    return nodeId;
  }
//---------------------------------------------------------------------------//
  void Profiling::BeginFrame()
  {
    ASSERT(ourCurrStack.empty(), "There are still open profiling markers at the end of the frame");

    ourMaxNumSamplesPerFrame = glm::max(ourMaxNumSamplesPerFrame, ourLevelTail != UINT_MAX ? ourNodePool[ourLevelTail].myNumNodes : 0u);

    // Clean up old recorded frame data if necessary
    const uint numFreeNodesNeeded = ourMaxNumSamplesPerFrame + 100;
    if (ourNextUsedNode - ourNextFreeNode < numFreeNodesNeeded)
    {
      ASSERT(ourLevelHead != ourLevelTail); // Are there at least two recorded frames?

      while (ourNextUsedNode - ourNextFreeNode < numFreeNodesNeeded && ourLevelHead != UINT_MAX)
      {
        ourNextFreeNode = ourLevelHead;
        ourNextUsedNode = ourLevelHead + ourNodePool[ourLevelHead].myNumNodes;

        ourLevelHead = ourNodePool[ourLevelHead].myNext;
      }

      ASSERT(ourNextUsedNode - ourNextFreeNode >= numFreeNodesNeeded);
    }

    // Start the next frame
    ourPaused = ourPauseRequested;

    if (!ourPaused)
      PushMarker("Frame Root", 0u);
  }
//---------------------------------------------------------------------------//
  void Profiling::EndFrame()
  {
    if (!ourPaused)
    {
      const uint frameId = PopMarker();  // Close the frame root marker
      ourNodePool[ourLevelTail].myNext = frameId != ourLevelTail ? frameId : UINT_MAX;
      ourLevelTail = frameId;
    }

    ourChildrenTail = ourLevelTail;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void Profiling::Init()
  {
    if (ourNodePool.empty())
      ourNodePool.resize(ourMaxNumNodes);
  }
//---------------------------------------------------------------------------//
  const Profiling::SampleNode* Profiling::GetSample(uint aSampleId)
  {
    if (aSampleId == UINT_MAX)
      return nullptr;

    ASSERT(aSampleId < ourNextFreeNode);
    return &ourNodePool[aSampleId];
  }
//---------------------------------------------------------------------------//
  const Profiling::SampleNodeInfo* Profiling::GetSampleInfo(uint64 anInfoId)
  {
    return &ourNodeInfoPool[anInfoId];
  }
//---------------------------------------------------------------------------//
  void Profiling::SetPaused(bool aPause)
  {
    ourPauseRequested = aPause;
  }
//---------------------------------------------------------------------------//
}


