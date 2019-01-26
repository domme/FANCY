#pragma once

#include "FancyCoreDefines.h"
#include "MathUtil.h"

namespace Fancy
{
  template<uint POOL_SIZE>
  struct WrappedId
  {
    static_assert((POOL_SIZE & (POOL_SIZE - 1)) == 0, "POOL_SIZE must be power of two");
    static constexpr uint BIT_DEPTH = MathUtil::Log2(POOL_SIZE);

    explicit WrappedId(uint anId) : myValue(anId), myIsValid(anId < POOL_SIZE) {}
    WrappedId() : myValue(0), myIsValid(false) {}

    WrappedId& operator=(uint aVal) { myValue = aVal; myIsValid = aVal < POOL_SIZE;  return *this; }

    operator uint() const { return myIsValid ? myValue : UINT_MAX; }
    WrappedId operator+(uint anOtherId) const { return WrappedId(myValue + anOtherId); }
    WrappedId operator-(uint anOtherId) const { return WrappedId(myValue - anOtherId); }

    void operator+=(uint anOtherId) { myValue += anOtherId; }
    WrappedId operator++(int) { return WrappedId(myValue++); }
    WrappedId& operator++() { ++myValue; return *this; }

    void operator-=(uint aVal) { myValue -= aVal; }
    WrappedId operator--(int) { return WrappedId(myValue--); }
    WrappedId& operator--() { --myValue; return *this; }

    uint myValue : BIT_DEPTH;
    uint myIsValid : 1;
  };

  namespace Profiling
  {
    enum Consts
    {
      MAX_NAME_LENGTH = 128,
      FRAME_POOL_SIZE = 1 << 13,
      SAMPLE_POOL_SIZE = 1 << 14,
      MAX_SAMPLE_DEPTH = 2048,
    };

    using SampleId = WrappedId<SAMPLE_POOL_SIZE>;
    using FrameId = WrappedId<FRAME_POOL_SIZE>;

    struct SampleNodeInfo
    {
      char myName[MAX_NAME_LENGTH];
      uint8 myTag;
    };

    struct SampleNode
    {
      SampleId myChild;
      SampleId myNext;
      float64 myStart = 0.0;
      float64 myDuration = 0.0;
      uint64 myNodeInfo = 0u;
    };

    struct FrameData
    {
      SampleId myFirstSample;
      uint64 myFrame = 0u;
      float64 myStart = 0.0;
      float64 myDuration = 0.0;
    };

    struct ScopedMarker
    {
      ScopedMarker(const char* aName, uint8 aTag);
      ~ScopedMarker();
    };
  
    uint PushMarker(const char* aName, uint8 aTag);
    uint PopMarker();

    void Init();
    void BeginFrame();
    void EndFrame();

    FrameId GetLastFrame();
    FrameId GetFirstFrame();

    const FrameData& GetFrameData(FrameId aFrameId);
    const SampleNode& GetSampleData(SampleId aSampleId);
    const SampleNodeInfo& GetSampleInfo(uint64 anInfoId);
    void SetPaused(bool aPause);
  };

#define PROFILE_FUNCTION(...) Profiling::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}