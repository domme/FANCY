#include "fancy_core_precompile.h"

#include "RenderCore_Platform.h"
#include "Common/CommandLine.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  RenderPlatformDriverVendor ResolveGpuTypeArg( RenderPlatformDriverVendor & outVendor, bool & outHasFilter )
  {
    outVendor = RenderPlatformDriverVendor::OTHER;
    outHasFilter = false;

    const char * gpuTypeArg = CommandLine::GetInstance()->GetStringValue( "GpuType" );
    if ( gpuTypeArg != nullptr )
    {
      eastl::string gpuTypeStr = gpuTypeArg;
      gpuTypeStr.make_lower();
      if ( gpuTypeStr == "amd" )
      {
        outVendor = RenderPlatformDriverVendor::AMD;
        outHasFilter = true;
      }
      else if ( gpuTypeStr == "nvidia" )
      {
        outVendor = RenderPlatformDriverVendor::NVIDIA;
        outHasFilter = true;
      }
      else if ( gpuTypeStr == "other" )
      {
        outVendor = RenderPlatformDriverVendor::OTHER;
        outHasFilter = true;
      }
      else
      {
        LOG_WARNING( "Unknown GpuType '%s' - ignoring filter", gpuTypeArg );
      }
    }

    return outVendor;
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy
