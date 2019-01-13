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
  static std::list<Profiling::FrameData> ourFrameDatas;
  
  static constexpr uint64 ourMaxNumNodes = static_cast<uint>((Profiling::MAX_NODE_POOL_MB * SIZE_MB) / sizeof(Profiling::SampleNode));
  static uint ourNextFreeNode = 0u;
  static uint ourNextUsedNode = ourMaxNumNodes - 1;
  static uint ourMaxNumSamplesPerFrame = 0u;

  static std::stack<Profiling::SampleNode*> ourCurrStack;
  static Profiling::FrameData ourCurrFrame;
  static uint ourNumSamplesThisFrame = 0u;
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
  void Profiling::PushMarker(const char* aName, uint8 aTag)
  {
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
    node.myNumChildren = 0u;
    node.myDuration = 0u;

    if (ourCurrStack.empty())
    {
      ASSERT(ourCurrFrame.myNumSamples <= MAX_NUM_FRAME_SAMPLES, "Insufficient number of allocated frame samples");
      ourCurrFrame.mySamples[ourCurrFrame.myNumSamples++] = nodeId;
    }
    else
    {
      Profiling::SampleNode* parent = ourCurrStack.top();
      ASSERT(parent->myNumChildren < MAX_NUM_CHILDREN);
      parent->myChildren[parent->myNumChildren++] = nodeId;
    }

    ourCurrStack.push(&node);
  }
//---------------------------------------------------------------------------//
  void Profiling::PopMarker()
  {
    ASSERT(!ourCurrStack.empty());

    Profiling::SampleNode* node = ourCurrStack.top();
    node->myDuration = SampleTimeMs() - node->myStart;
    ourCurrStack.pop();
  }
//---------------------------------------------------------------------------//
  void Profiling::BeginFrame()
  {
    // End frame functionality. This is done here to make it possible to also profile stuff during the endFrame() calls.
    // conceptually, a frame begins in BeginFrame() and ends in the next BeginFrame() for the profiler
    {
      ASSERT(ourCurrStack.empty(), "There are still open profiling markers at the end of the frame");

      ourMaxNumSamplesPerFrame = glm::max(ourMaxNumSamplesPerFrame, ourNumSamplesThisFrame);
      ourNumSamplesThisFrame = 0u;

      if (ourCurrFrame.myNumSamples > 0u)
      {
        ourCurrFrame.myDuration = SampleTimeMs() - ourCurrFrame.myStart;
        ourFrameDatas.push_back(ourCurrFrame);
      }
      memset(&ourCurrFrame, 0u, sizeof(ourCurrFrame));
    }

    // Check if any recorded frames should be cleaned up
    const uint numFreeNodesNeeded = ourMaxNumSamplesPerFrame + 100;
    if (ourNextUsedNode - ourNextFreeNode < numFreeNodesNeeded)
    {
      auto frameIt = ourFrameDatas.begin();
      auto nextFrameIt = frameIt != ourFrameDatas.end() ? std::next(frameIt, 1) : frameIt;
      while (ourNextUsedNode - ourNextFreeNode < numFreeNodesNeeded && frameIt != ourFrameDatas.end())
      {
        ourNextFreeNode = glm::min(ourNextFreeNode, frameIt->mySamples[0]);

        ourNextUsedNode = nextFrameIt != ourFrameDatas.end() ? nextFrameIt->mySamples[0] : ourMaxNumNodes - 1;

        frameIt = ourFrameDatas.erase(frameIt);

        if (frameIt != ourFrameDatas.end())
          nextFrameIt = std::next(frameIt, 1);
      }

      ASSERT(ourNextUsedNode - ourNextFreeNode >= numFreeNodesNeeded);
    }

    ourCurrFrame.myStart = SampleTimeMs();
  }
//---------------------------------------------------------------------------//
  void Profiling::EndFrame()
  {
    
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void Profiling::Init()
  {
    ourNodePool.resize(ourMaxNumNodes);
  }
//---------------------------------------------------------------------------//
  const Profiling::SampleNode* Profiling::GetSample(uint aSampleId)
  {
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
}


