#include "fancy_core_precompile.h"
#include "MathUtil.h"

#include <xxHash/xxhash.h>

namespace Fancy { namespace MathUtil {
//---------------------------------------------------------------------------//
  namespace Priv_MathUtil
  {
    thread_local XXH64_state_t* ourXXHashState = nullptr;
    thread_local bool ourMultiHashStarted = false;
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
  void AddToMultiHash(const uint8* aValue, uint64 aSize)
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
  uint64 ByteHash(const uint8* aValue, uint64 aSize)
  {
    return XXH64(aValue, aSize, 0ull);
  }
//---------------------------------------------------------------------------//
} }