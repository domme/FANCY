#pragma once

#include "FancyCoreDefines.h"

namespace Fancy
{
  template<uint POOL_SIZE>
  struct WrappedInteger
  {
    static int GetWrapped(int aVal)
    {
      static_assert((POOL_SIZE & (POOL_SIZE - 1)) == 0,
        "POOL_SIZE must be power of two for cheaper modulo");

      if (aVal >= 0)
        return aVal & (POOL_SIZE - 1);

      return (aVal % POOL_SIZE) + POOL_SIZE;
    }

    explicit WrappedInteger(int anId) : myId(GetWrapped(anId)) {}
    WrappedInteger() : myId(0) {}

    WrappedInteger& operator=(int anOther) { myId = anOther; return *this; }

    operator int() const { return myId; }
    WrappedInteger operator+(const WrappedInteger& anOther) const { return WrappedInteger(myId + anOther.myId); }
    WrappedInteger operator+(int anOtherId) const { return WrappedInteger(myId + anOtherId); }
    WrappedInteger operator-(const WrappedInteger& anOther) const { return WrappedInteger(myId - anOther.myId); }
    WrappedInteger operator-(uint anOtherId) const { return WrappedInteger(myId - anOtherId); }

    bool operator==(int aVal) const { return myId == aVal; }
    bool operator<(int aVal) const { return myId < aVal; }
    bool operator>(int aVal) const { return myId > aVal; }
    bool operator!=(int aVal) const { return myId != aVal; }

    

    void operator+=(int anOtherId) { myId = GetWrapped(myId + anOtherId); }
    void operator+=(const WrappedInteger& anOther) { myId = GetWrapped(myId + anOther.myId); }
    WrappedInteger operator++(int) { WrappedInteger old(myId); myId = GetWrapped(myId + 1); return old; }
    WrappedInteger& operator++() { myId = GetWrapped(myId + 1); return *this; }

    void operator-=(int anOtherId) { myId = GetWrapped(myId - anOtherId); }
    void operator-=(const WrappedInteger& anOther) { myId = GetWrapped(myId - anOther.myId); }
    WrappedInteger operator--(int) { WrappedInteger old(myId);  myId = GetWrapped(myId - 1); return old; }
    WrappedInteger& operator--() { WrappedInteger old(myId);  myId = GetWrapped(myId - 1); return old; }

    int myId;
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

    using SampleId = WrappedInteger<SAMPLE_POOL_SIZE>;
    using FrameId = WrappedInteger<FRAME_POOL_SIZE>;

    struct SampleNodeInfo
    {
      char myName[MAX_NAME_LENGTH];
      uint8 myTag;
    };

    struct SampleNode
    {
      SampleId myChild = SampleId(INT_MAX);
      SampleId myNext = SampleId(INT_MAX);
      float64 myStart = 0.0;
      float64 myDuration = 0.0;
      uint64 myNodeInfo = 0u;
    };

    struct FrameData
    {
      SampleId myFirstSample = SampleId(INT_MAX);
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