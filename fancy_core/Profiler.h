#pragma once

#include "FancyCoreDefines.h"
#include "MathUtil.h"
#include "CircularArray.h"

namespace Fancy
{
  /*
  template<uint POOL_SIZE>
  struct WrappedId
  {
    static_assert((POOL_SIZE & (POOL_SIZE - 1)) == 0, "POOL_SIZE must be power of two");

    static uint GetWrapped(uint aVal) { return aVal % (POOL_SIZE - 1); }

    explicit WrappedId(uint anId) : myValue(anId) {}
    WrappedId() : myValue(UINT_MAX) {}

    WrappedId& operator=(uint aVal) { myValue = aVal; return *this; }

    operator uint() const { return myValue; }
    WrappedId operator+(uint anOtherId) const { return WrappedId(GetWrapped(myValue + anOtherId)); }
    WrappedId operator-(uint anOtherId) const { return WrappedId(GetWrapped(myValue - anOtherId)); }

    void operator+=(uint anOtherId) { myValue = GetWrapped(myValue + anOtherId); }
    WrappedId operator++(int) { WrappedId old(myValue); myValue = GetWrapped(myValue + 1u); return old; }
    WrappedId& operator++() { myValue = GetWrapped(myValue + 1u); return *this; }

    void operator-=(uint aVal) { myValue = GetWrapped(myValue - aVal); }
    WrappedId operator--(int) { WrappedId old(myValue); myValue = GetWrapped(myValue - 1u); return old; }
    WrappedId& operator--() { myValue = GetWrapped(myValue - 1u); return *this; }

    uint myValue;
  };
  */

  struct Profiler
  {
    enum Consts
    {
      MAX_NAME_LENGTH = 128,
      FRAME_POOL_SIZE = 1 << 11,
      SAMPLE_POOL_SIZE = 1 << 14,
      MAX_SAMPLE_DEPTH = 2048,
    };

    struct SampleNodeInfo
    {
      char myName[MAX_NAME_LENGTH];
      uint8 myTag;
    };

    struct SampleNode
    {
      uint myChild;
      uint myNext;
      float64 myStart = 0.0;
      float64 myDuration = 0.0;
      uint64 myNodeInfo = 0u;
    };

    struct FrameData
    {
      uint myFirstSample;
      uint64 myFrame = 0u;
      float64 myStart = 0.0;
      float64 myDuration = 0.0;
    };

    struct ScopedMarker
    {
      ScopedMarker(const char* aName, uint8 aTag);
      ~ScopedMarker();
    };
  
    static uint PushMarker(const char* aName, uint8 aTag);
    static uint PopMarker();

    static void Init();
    static void BeginFrame();
    static void EndFrame();

    static const CircularArray<FrameData>& GetRecordedFrames() { return ourRecordedFrames; }
    static const CircularArray<SampleNode>& GetRecordedSamples() { return ourRecordedSamples; }
    static const SampleNodeInfo& GetSampleInfo(uint64 anInfoId);

    static bool ourPauseRequested;

  private: 
    Profiler() = delete;
    ~Profiler() = delete;

    static void FreeFirstFrame();
    static uint AllocateSample();
    static uint AllocateFrame();

    static CircularArray<FrameData> ourRecordedFrames;
    static CircularArray<SampleNode> ourRecordedSamples;
    static std::unordered_map<uint64, SampleNodeInfo> ourNodeInfoPool;
  };

#define PROFILE_FUNCTION(...) Profiler::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}
