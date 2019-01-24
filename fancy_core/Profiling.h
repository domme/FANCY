#pragma once

#include "FancyCoreDefines.h"
#include "MathUtil.h"

namespace Fancy
{
  template<uint POOL_SIZE>
  struct WrappedId
  {
    static_assert((POOL_SIZE & (POOL_SIZE - 1)) == 0, "POOL_SIZE must be power of two for cheaper modulo");
    static constexpr uint BIT_DEPTH = MathUtil::Log2(POOL_SIZE);

    explicit WrappedId(uint anId) : myValue(anId), myIsValid(anId < POOL_SIZE) {}
    WrappedId() : myValue(0), myIsValid(false) {}

    WrappedId& operator=(uint aVal) { myValue = aVal; myIsValid = aVal < POOL_SIZE;  return *this; }

    operator uint() const { return myIsValid ? myValue : UINT_MAX; }
    WrappedId operator+(const WrappedId& anOther) const { return WrappedId(myValue + anOther.myValue); }
    WrappedId operator+(int anOtherId) const { return WrappedId(myValue + anOtherId); }
    WrappedId operator-(const WrappedId& anOther) const { return WrappedId(myValue - anOther.myValue); }
    WrappedId operator-(uint anOtherId) const { return WrappedId(myValue - anOtherId); }

    void operator+=(uint anOtherId) { myValue += anOtherId; }
    void operator+=(const WrappedId& anOther) { myValue += anOther.myValue; }
    WrappedId operator++(int) { return WrappedId(myValue++); }
    WrappedId& operator++() { ++myValue; return *this; }

    void operator-=(uint aVal) { myValue -= aVal; }
    void operator-=(const WrappedId& anOther) { myValue -= anOther.myValue; }
    WrappedId operator--(int) { return WrappedId(myValue--); }
    WrappedId& operator--() { --myValue; return *this; }

    uint myValue : BIT_DEPTH;
    uint myIsValid : 1;
  };

  /*
  template<uint POOL_SIZE>
  struct WrappedInteger2
  {
    static_assert((POOL_SIZE & (POOL_SIZE - 1)) == 0, "POOL_SIZE must be power of two for cheaper modulo");

    static int GetWrapped(int aVal)
    {
      static_assert((POOL_SIZE & (POOL_SIZE - 1)) == 0,
        "POOL_SIZE must be power of two for cheaper modulo");

      if (aVal >= 0)
        return aVal & (POOL_SIZE - 1);

      return (aVal % POOL_SIZE) + POOL_SIZE;
    }

    explicit WrappedId(int anId) : myId(GetWrapped(anId)) {}
    WrappedId() : myId(0) {}

    WrappedId& operator=(int anOther) { myId = anOther; return *this; }

    operator int() const { return myId; }
    WrappedId operator+(const WrappedId& anOther) const { return WrappedId(myId + anOther.myId); }
    WrappedId operator+(int anOtherId) const { return WrappedId(myId + anOtherId); }
    WrappedId operator-(const WrappedId& anOther) const { return WrappedId(myId - anOther.myId); }
    WrappedId operator-(uint anOtherId) const { return WrappedId(myId - anOtherId); }

    bool operator==(int aVal) const { return myId == aVal; }
    bool operator<(int aVal) const { return myId < aVal; }
    bool operator>(int aVal) const { return myId > aVal; }
    bool operator!=(int aVal) const { return myId != aVal; }

    void operator+=(int anOtherId) { myId = GetWrapped(myId + anOtherId); }
    void operator+=(const WrappedId& anOther) { myId = GetWrapped(myId + anOther.myId); }
    WrappedId operator++(int) { WrappedId old(myId); myId = GetWrapped(myId + 1); return old; }
    WrappedId& operator++() { myId = GetWrapped(myId + 1); return *this; }

    void operator-=(int anOtherId) { myId = GetWrapped(myId - anOtherId); }
    void operator-=(const WrappedId& anOther) { myId = GetWrapped(myId - anOther.myId); }
    WrappedId operator--(int) { WrappedId old(myId);  myId = GetWrapped(myId - 1); return old; }
    WrappedId& operator--() { WrappedId old(myId);  myId = GetWrapped(myId - 1); return old; }

    int myId;
  };
  */

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