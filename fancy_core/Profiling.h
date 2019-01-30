#pragma once

#include "FancyCoreDefines.h"
#include "MathUtil.h"

namespace Fancy
{
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

  namespace Profiling
  {
    enum Consts
    {
      MAX_NAME_LENGTH = 128,
      FRAME_POOL_SIZE = 1 << 11,
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
    uint GetNumRecordedFrames();

    const FrameData& GetFrameData(FrameId aFrameId);
    const SampleNode& GetSampleData(SampleId aSampleId);
    const SampleNodeInfo& GetSampleInfo(uint64 anInfoId);
    void SetPaused(bool aPause);
  };

#define PROFILE_FUNCTION(...) Profiling::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}