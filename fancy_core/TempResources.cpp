#include "fancy_core_precompile.h"
#include "TempResources.h"
#include "TempResourcePool.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  TempResourceKeepAlive::~TempResourceKeepAlive()
  {
    myPool->FreeResource(myResource, myBucketHash);
  }
//---------------------------------------------------------------------------//
}