#include "fancy_core_precompile.h"
#include "MathUtil.h"

#include <xxhash.h>

namespace Fancy { namespace MathUtil {
//---------------------------------------------------------------------------//
  namespace Priv_MathUtil
  {
    thread_local XXH64_state_t* ourXXHashState = nullptr;
    thread_local bool ourMultiHashStarted = false;
  }
//---------------------------------------------------------------------------//
  Hasher::Hasher(uint64 aSeed /* = 0u */)
    : myState(nullptr)
  {
    myState = XXH64_createState();
    XXH64_reset((XXH64_state_t*) myState, aSeed);
  }
//---------------------------------------------------------------------------//
  Hasher::~Hasher()
  {
    XXH64_freeState((XXH64_state_t*) myState);
  }
//---------------------------------------------------------------------------//
  void Hasher::Add(const void* aValue, uint64 aSize)
  {
    XXH64_update((XXH64_state_t*)myState, aValue, aSize);
  }
//---------------------------------------------------------------------------//
  uint64 Hasher::GetHashValue() const
  {
    return XXH64_digest((XXH64_state_t*)myState);
  }
//---------------------------------------------------------------------------//
  void BeginMultiHash()
  {
    ASSERT(!Priv_MathUtil::ourMultiHashStarted, "Multi hash already started. Did you forget to call EndMultiHash()?");
    if (Priv_MathUtil::ourXXHashState == nullptr)
      Priv_MathUtil::ourXXHashState = XXH64_createState();

    Priv_MathUtil::ourMultiHashStarted = true;
    XXH64_reset(Priv_MathUtil::ourXXHashState, 0u);
  }
//---------------------------------------------------------------------------//
  void AddToMultiHash(const void* aValue, uint64 aSize)
  {
    ASSERT(Priv_MathUtil::ourMultiHashStarted, "Multi hash not started. Did you forget to call BeginMultiHash()?");
    ASSERT(XXH64_update(Priv_MathUtil::ourXXHashState, aValue, aSize) == XXH_OK);
  }
//---------------------------------------------------------------------------//
  uint64 EndMultiHash()
  {
    ASSERT(Priv_MathUtil::ourMultiHashStarted, "Multi hash not started. Did you forget to call BeginMultiHash()?");
    Priv_MathUtil::ourMultiHashStarted = false;
    return XXH64_digest(Priv_MathUtil::ourXXHashState);
  }
//---------------------------------------------------------------------------//
  uint64 ByteHash(const void* aValue, uint64 aSize)
  {
    return XXH64(aValue, aSize, 0ull);
  }
//---------------------------------------------------------------------------//
  glm::float2 ToSpherical(const glm::float3& aNormalizedDir)
  {
    float phi = atan2f(aNormalizedDir.z, aNormalizedDir.x);
    float theta = glm::acos(aNormalizedDir.y);
    return glm::float2(theta, phi);
  }
//---------------------------------------------------------------------------//
  glm::float3 ToCartesian(const glm::float2 aSphericalThetaPhi)
  {
    float cosTheta = glm::cos(aSphericalThetaPhi.x);
    float sinTheta = glm::sin(aSphericalThetaPhi.x);
    float cosPhi = glm::cos(aSphericalThetaPhi.y);
    float sinPhi = glm::sin(aSphericalThetaPhi.y);

    return glm::float3(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
  }
//---------------------------------------------------------------------------//
} }