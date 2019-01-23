#pragma once

#include "FancyCoreDefines.h"

namespace Fancy
{
  template<uint POOL_SIZE>
  struct WrappedId
  {
    static int GetWrapped(int aVal)
    {
      static_assert((POOL_SIZE & (POOL_SIZE - 1)) == 0,
        "POOL_SIZE must be power of two for cheaper modulo");

      if (aVal >= 0)
        return aVal & (POOL_SIZE - 1);

      return (aVal % POOL_SIZE) + POOL_SIZE;
    }

    explicit WrappedId(int anId) : myId(GetWrapped(anId)) {}

    int operator()() const { return myId; }
    WrappedId operator+(const WrappedId& anOther) const { return WrappedId(myId + anOther.myId); }
    WrappedId operator+(int anOtherId) const { return WrappedId(myId + anOtherId); }
    WrappedId operator-(const WrappedId& anOther) const { return WrappedId(myId - anOther.myId); }
    WrappedId operator-(uint anOtherId) const { return WrappedId(myId - anOtherId); }

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
      uint myChild = UINT_MAX;
      uint myNext = UINT_MAX;
      float64 myStart = 0.0;
      float64 myDuration = 0.0;
      uint64 myNodeInfo = 0u;
    };

    struct FrameData
    {
      uint myFirstSample = UINT_MAX;
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

    void GetLastFrames(uint* someFrameIdsOut, uint* aNumFramesOut, uint aMaxNumFrames);
    uint GetLastFrame();
    uint GetFirstFrame();

    inline uint GetWrappedSampleId(uint aSampleId)
    {
      static_assert((Profiling::SAMPLE_POOL_SIZE & (Profiling::SAMPLE_POOL_SIZE - 1)) == 0,
        "SAMPLE_POOL_SIZE must be power of two for cheaper modulo");

      return aSampleId & (Profiling::SAMPLE_POOL_SIZE - 1);
    }

    inline uint GetWrappedFrameId(uint aFrameId)
    {
      static_assert((Profiling::FRAME_POOL_SIZE & (Profiling::FRAME_POOL_SIZE - 1)) == 0,
        "FRAME_POOL_SIZE must be power of two for cheaper modulo");

      return aFrameId & (Profiling::FRAME_POOL_SIZE - 1);
    }


    const FrameData& GetFrame(uint aFrameId);
    const SampleNode& GetSample(uint aSampleId);
    const SampleNodeInfo& GetSampleInfo(uint64 anInfoId);
    void SetPaused(bool aPause);
  };

#define PROFILE_FUNCTION(...) Profiling::ScopedMarker __marker##__FUNCTION__ (__FUNCTION__, 0u)
}