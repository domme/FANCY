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
  static uint ourNextUsedNode = ourMaxNumNodes - 1;
  static uint ourMaxNumSamplesPerFrame = 0u;

  static bool ourPauseRequested = false;
  static bool ourPaused = false;

  static std::stack<uint> ourCurrStack;
  static uint ourLastSampleOnLevel = UINT_MAX;
  static uint ourNumSamplesThisFrame = 0u;
  static uint ourDataHead = 0u;
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

    const uint nodeId = AllocateNode();
    Profiling::SampleNode& node = ourNodePool[nodeId];
    ++ourNumSamplesThisFrame;

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

    node.myNodeInfo = hash;
    node.myStart = SampleTimeMs();
    node.myDuration = 0u;
    node.myNext = UINT_MAX;
    node.myChild = UINT_MAX;

    ASSERT(!ourCurrStack.empty());  // There should always be a frame-root sample

    if (ourLastSampleOnLevel == UINT_MAX)  // First child of parent
    {
      SampleNode* currStackNode = &ourNodePool[ourCurrStack.top()];
      ASSERT(currStackNode->myChild == UINT_MAX);
      currStackNode->myChild = nodeId;
    }
    else
    {
      ourNodePool[ourLastSampleOnLevel].myNext = nodeId;
    }
    
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
    ourLastSampleOnLevel = nodeId;
  }
//---------------------------------------------------------------------------//
  void Profiling::BeginFrame()
  {
    ASSERT(ourCurrStack.empty(), "There are still open profiling markers at the end of the frame");

    ourMaxNumSamplesPerFrame = glm::max(ourMaxNumSamplesPerFrame, ourNumSamplesThisFrame);
    ourNumSamplesThisFrame = 0u;

    // Clean up old recorded frame data if necessary
    const uint numFreeNodesNeeded = ourMaxNumSamplesPerFrame + 100;
    if (ourNextUsedNode - ourNextFreeNode < numFreeNodesNeeded)
    {
      ASSERT(ourDataHead != UINT_MAX && ourNodePool[ourDataHead].myNext != UINT_MAX);  // Are there at least two recorded frames?

      uint frameStart = ourDataHead;
      uint nextFrameStart = ourNodePool[ourDataHead].myNext;
      while (ourNextUsedNode - ourNextFreeNode < numFreeNodesNeeded && frameStart != UINT_MAX)
      {
        ourNextFreeNode = glm::min(ourNextFreeNode, frameStart);
        ourNextUsedNode = nextFrameStart != UINT_MAX ? nextFrameStart : ourMaxNumNodes - 1;

        ourDataHead = nextFrameStart;
        frameStart = ourDataHead;
        nextFrameStart = ourNodePool[ourDataHead].myNext;
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
      PopMarker();  // Close the frame root marker

    if (ourDataHead == nullptr)
      ourDataHead = ourLastSampleOnLevel;

    ourLastSampleOnLevel = nullptr;
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
  const std::list<Profiling::FrameData>& Profiling::GetFrames()
  {
    return ourFrameDatas;
  }
//---------------------------------------------------------------------------//
  void Profiling::SetPaused(bool aPause)
  {
    ourPauseRequested = aPause;
  }
//---------------------------------------------------------------------------//
}


