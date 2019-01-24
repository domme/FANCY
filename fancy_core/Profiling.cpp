#include "fancy_core_precompile.h"
#include "Profiling.h"
#include "MathUtil.h"

#include <chrono>
#include <ratio>

namespace Fancy
{
  static std::unordered_map<uint64, Profiling::SampleNodeInfo> ourNodeInfoPool;
  static DynamicArray<Profiling::SampleNode> ourNodePool;
  static DynamicArray<Profiling::FrameData> ourFramePool;
  
  static Profiling::SampleId ourNextFreeNode(0);
  static Profiling::SampleId ourNextUsedNode(0);

  static Profiling::SampleId ourNodeStack[Profiling::MAX_SAMPLE_DEPTH];
  static Profiling::SampleId ourTailStack[Profiling::MAX_SAMPLE_DEPTH];
  static uint ourSampleDepth = 0u;

  static Profiling::FrameId ourNextFreeFrame(0);
  static Profiling::FrameId ourNextUsedFrame(0);
  static Profiling::FrameId ourFrameHead(0);
  static Profiling::FrameId ourFrameTail(0);

  static Profiling::FrameData ourCurrFrame;
  static bool ourPauseRequested = false;
  static bool ourPaused = false;
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
  void FreeFirstFrame()
  {
    const Profiling::FrameData& firstFrame = ourFramePool[ourFrameHead];

    // Advance the used-markers
    if (ourFrameHead != ourFrameTail)
    {
      const Profiling::FrameId nextFrame = ourFrameHead + 1u;
      ourNextUsedNode = ourFramePool[nextFrame].myFirstSample;
      ourNextUsedFrame = nextFrame;
    }
    else
    {
      ourNextUsedNode = 0;
      ourNextUsedFrame = 0;
    }
    
    ++ourFrameHead;
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
    ++ourNextFreeNode;

    if (ourNextFreeNode == ourNextUsedNode)
      FreeFirstFrame(); 
      
    return freeNode;
  }
//---------------------------------------------------------------------------//
  uint AllocateFrame()
  {
    const uint freeFrame = ourNextFreeFrame;
    ++ourNextFreeFrame;

    if (ourNextFreeFrame == ourNextUsedFrame)
      FreeFirstFrame();

    return freeFrame;
  }
//---------------------------------------------------------------------------//
  uint Profiling::PushMarker(const char* aName, uint8 aTag)
  {
    if (ourPaused)
      return UINT_MAX;

    ASSERT(ourSampleDepth < MAX_SAMPLE_DEPTH);

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

    const uint nodeId = ourNodeStack[--ourSampleDepth];
    Profiling::SampleNode& node = ourNodePool[nodeId];
    node.myDuration = SampleTimeMs() - node.myStart;

    if (ourSampleDepth == 0 && ourCurrFrame.myFirstSample == UINT_MAX)  // First child of frame?
    {
        ourCurrFrame.myFirstSample = nodeId;
    }
    else if (ourSampleDepth > 0 && ourNodePool[ourNodeStack[ourSampleDepth - 1]].myChild == UINT_MAX)  // First child of parent?
    {
      ourNodePool[ourNodeStack[ourSampleDepth - 1]].myChild = nodeId;
    }
    else
    {
      const uint lastNodeOnLevel = ourTailStack[ourSampleDepth];
      ourNodePool[lastNodeOnLevel].myNext = nodeId;
    }
    ourTailStack[ourSampleDepth] = nodeId;

    return nodeId;
  }
//---------------------------------------------------------------------------//
  void Profiling::BeginFrame()
  {
    ASSERT(ourSampleDepth == 0, "There are still open profiling markers at the end of the frame");

    // Finalize the last frame
    if (ourCurrFrame.myFirstSample != UINT_MAX)  // We have samples in this frame
    {
      ourCurrFrame.myDuration = SampleTimeMs() - ourCurrFrame.myStart;
      const uint allocatedFrame = AllocateFrame();

      ourFramePool[allocatedFrame] = ourCurrFrame;
      ourFrameTail = allocatedFrame;
    }
    
    // Start the next frame
    ourPaused = ourPauseRequested;
    if (!ourPaused)
    {
      ourCurrFrame.myFirstSample = UINT_MAX;
      ourCurrFrame.myDuration = 0u;
      ourCurrFrame.myStart = SampleTimeMs();
    }
  }
//---------------------------------------------------------------------------//
  void Profiling::EndFrame()
  {
    
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void Profiling::Init()
  {
    if (ourNodePool.empty() || ourFramePool.empty())
    {
      ourNodePool.resize(SAMPLE_POOL_SIZE);
      ourFramePool.resize(FRAME_POOL_SIZE);
    }
  }
  //---------------------------------------------------------------------------//
  Profiling::FrameId Profiling::GetLastFrame()
  {
    return ourFrameTail;
  }
//---------------------------------------------------------------------------//
  Profiling::FrameId Profiling::GetFirstFrame()
  {
    return ourFrameHead;
  }
//---------------------------------------------------------------------------//
  const Profiling::FrameData& Profiling::GetFrameData(Profiling::FrameId aFrameId)
  {
    return ourFramePool[aFrameId];
  }
//---------------------------------------------------------------------------//
  const Profiling::SampleNode& Profiling::GetSampleData(Profiling::SampleId aSampleId)
  {
    return ourNodePool[aSampleId];
  }
//---------------------------------------------------------------------------//
  const Profiling::SampleNodeInfo& Profiling::GetSampleInfo(uint64 anInfoId)
  {
    const auto it = ourNodeInfoPool.find(anInfoId);
    ASSERT(it != ourNodeInfoPool.end());
    return it->second;
  }
//---------------------------------------------------------------------------//
  void Profiling::SetPaused(bool aPause)
  {
    ourPauseRequested = aPause;
  }
//---------------------------------------------------------------------------//
}