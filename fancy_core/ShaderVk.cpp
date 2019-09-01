#include "fancy_core_precompile.h"
#include "ShaderVk.h"
#include "VkPrerequisites.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  ShaderVk::ShaderVk()
  {
  }


  ShaderVk::~ShaderVk()
  {
  }

  void ShaderVk::SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }

  uint64 ShaderVk::GetNativeBytecodeHash() const
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return 0u;
  }

//---------------------------------------------------------------------------//
}


