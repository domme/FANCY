#include "fancy_core_precompile.h"
#include "ShaderDX12.h"

#include "ShaderCompiler.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  void ShaderDX12::SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput)
  {
    Shader::SetFromCompilerOutput(aCompilerOutput);

    myCompiledData = eastl::any_cast<const ShaderCompiledDataDX12&>(aCompilerOutput.myNativeData);

    myNativeByteCode.pShaderBytecode = myCompiledData.myBytecode.data();
    myNativeByteCode.BytecodeLength = myCompiledData.myBytecode.size();
    myNativeByteCodeHash = MathUtil::ByteHash(myCompiledData.myBytecode.data(), myCompiledData.myBytecode.size());
  }
//---------------------------------------------------------------------------//
}

#endif