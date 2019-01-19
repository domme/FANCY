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
  static DynamicArray<uint> ourNodeFrameOwners;
  static DynamicArray<Profiling::FrameData> ourFramePool;
  
  static uint ourNextFreeNode = 0u;
  static uint ourNextUsedNode = Profiling::SAMPLE_POOL_SIZE;

  static bool ourPauseRequested = false;
  static bool ourPaused = false;

  static uint ourNodeStack[Profiling::MAX_SAMPLE_DEPTH];
  static uint ourTailStack[Profiling::MAX_SAMPLE_DEPTH];
  static uint ourSampleDepth = 0u;

  static uint ourNextFreeFrame = 0u;
  static uint ourNextUsedFrame = Profiling::FRAME_POOL_SIZE;
  static uint ourFrameHead = 0u;
  static uint ourFrameTail = 0u;
  static Profiling::FrameData ourCurrFrame;
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
  void FreeFirstFrame()
  {
    const Profiling::FrameData& firstFrame = ourFramePool[ourFrameHead];
    uint frameNode = firstFrame.myFirstSample;
    ASSERT(frameNode != UINT_MAX && frameNode < Profiling::SAMPLE_POOL_SIZE);
    ASSERT(ourNodeFrameOwners[frameNode] == ourFrameHead);

    while (ourNodeFrameOwners[frameNode] == frameNode)
    {
      ourNodeFrameOwners[frameNode] = UINT_MAX;
      frameNode = (frameNode + 1) % Profiling::SAMPLE_POOL_SIZE;
    }

    ourFrameHead = firstFrame.myNext;
  }
//---------------------------------------------------------------------------//
  static float64 SampleTimeMs()
  {
    const std::chrono::duration<float64, std::milli> now(std::chrono::system_clock::now().time_since_epoch());
    return now.count();
  }
//---------------------------------------------------------------------------//
  uint AllocateNode()
  {
    const uint freeNode = ourNextFreeNode;
    const uint nextUsedWrapped = ourNextUsedNode % Profiling::SAMPLE_POOL_SIZE;
    ourNextFreeNode = (ourNextFreeNode + 1) % Profiling::SAMPLE_POOL_SIZE;
    if (ourNextFreeNode == nextUsedWrapped)
    {
      const Profiling::FrameData& firstFrame = ourFramePool[ourFrameHead];
      ASSERT(ourNextFreeNode == firstFrame.myFirstSample);
      FreeFirstFrame(); 
      ourNextUsedNode = ourFramePool[ourFrameHead].myFirstSample;
    }
    return freeNode;
  }
//---------------------------------------------------------------------------//
  uint AllocateFrame()
  {
    const uint freeFrame = ourNextFreeFrame;
    const uint nextUsedWrapped = ourNextUsedFrame % Profiling::FRAME_POOL_SIZE;
    ourNextFreeFrame = (ourNextFreeFrame + 1) % Profiling::FRAME_POOL_SIZE;
    if (ourNextFreeFrame == nextUsedWrapped)
    {
      const Profiling::FrameData& firstFrame = ourFramePool[ourFrameHead];
      FreeFirstFrame();
      ourNextUsedNode = ourFramePool[ourFrameHead].myFirstSample;
      ourNextUsedFrame = (ourNextUsedFrame + 1) % Profiling::FRAME_POOL_SIZE;
    }
    return freeFrame;
  }
//---------------------------------------------------------------------------//
  uint Profiling::PushMarker(const char* aName, uint8 aTag)
  {
    if (ourPaused || ourSampleDepth == MAX_SAMPLE_DEPTH)
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
    
    ourNodeStack[ourSampleDepth++] = nodeId;

    return nodeId;
  }
//---------------------------------------------------------------------------//
  uint Profiling::PopMarker()
  {
    if (ourPaused)
      return UINT_MAX;

    ASSERT(ourSampleDepth > 0u);

    const uint nodeId = ourNodeStack[ourSampleDepth];
    Profiling::SampleNode* node = &ourNodePool[nodeId];
    node->myDuration = SampleTimeMs() - node->myStart;

    ourNodeFrameOwners[nodeId] = ourCurrFrame;
    FrameData& frame = ourFramePool[ourCurrFrame];

    if (ourSampleDepth == 0 && frame.myFirstSample == UINT_MAX)
    {
      frame.myFirstSample = nodeId;
    }
    else if (ourSampleDepth > 0)
    {
      const uint parentNodeId = ourNodeStack[ourSampleDepth - 1];
      SampleNode& parentNode = ourNodePool[parentNodeId];
      if (parentNode.myChild == UINT_MAX)
      {
        parentNode.myChild = nodeId;
      }
      else
      {
        const uint lastNodeOnLevel = ourTailStack[ourSampleDepth];
        ourNodePool[lastNodeOnLevel].myNext = nodeId;
      }
    }

    ourTailStack[ourSampleDepth] = nodeId;
    --ourSampleDepth;

    return nodeId;
  }
//---------------------------------------------------------------------------//
  void Profiling::BeginFrame()
  {
    ASSERT(ourSampleDepth == 0, "There are still open profiling markers at the end of the frame");

    // Finalize the last frame
    if (lastFrame.myFirstSample != UINT_MAX)  // We have samples in this frame
    {
      lastFrame.myDuration = SampleTimeMs() - lastFrame.myStart;
      const uint allocatedFrame = AllocateFrame();
      ourFramePool[allocatedFrame] = lastFrame;
      
      ourFramePool[ourFrameTail].myNext = allocatedFrame;
      ourFrameTail = allocatedFrame;
    }
    
    // Start the next frame
    ourPaused = ourPauseRequested;
    if (!ourPaused)
    {
      FrameData& currFrame = ourFramePool[ourCurrFrame];
      currFrame.myFirstSample = UINT_MAX;
      currFrame.myNext = UINT_MAX;
      currFrame.myStart = SampleTimeMs();
      currFrame.myDuration = 0u;
    }
  }
//---------------------------------------------------------------------------//
  void Profiling::EndFrame()
  {
    if (!ourPaused)
    {
      const uint frameId = PopMarker();  // Close the frame root marker
      ourNodePool[ourFrameTail].myNext = frameId != ourFrameTail ? frameId : UINT_MAX;
      ourFrameTail = frameId;
    }

    ourChildrenTail = ourFrameTail;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void Profiling::Init()
  {
    if (ourNodePool.empty() || ourFramePool.empty())
    {
      ourNodePool.resize(ourMaxNumNodes);
      ourFramePool.resize(FRAME_POOL_SIZE);
      ourNodeFrameOwners.resize(ourMaxNumNodes, UINT_MAX);
    }
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


