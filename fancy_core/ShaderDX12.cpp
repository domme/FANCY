#include "fancy_core_precompile.h"
#include "ShaderDX12.h"

#include "ShaderCompiler.h"
#include "RenderCore_PlatformDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  void ShaderDX12::SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput)
  {
    Shader::SetFromCompilerOutput(aCompilerOutput);

    const ShaderCompiledDataDX12& data = aCompilerOutput.myNativeData.To<ShaderCompiledDataDX12>();

    myBytecode = data.myBytecode;
    myRootSignature = data.myRootSignature;
    myRootSignatureLayout = data.myRootSignatureLayout;

    myNativeByteCode.pShaderBytecode = myBytecode.data();
    myNativeByteCode.BytecodeLength = myBytecode.size();
    myNativeByteCodeHash = MathUtil::ByteHash(myBytecode.data(), myBytecode.size());

    myResourceInfos = data.myResourceInfos;
  }
//---------------------------------------------------------------------------//
}

#endif