#pragma once
#include "GpuProgramDesc.h"

#if defined (RENDERER_DX12)
#include "GpuProgramResource.h"
#include "VertexInputLayout.h"

namespace Fancy{ namespace IO{
class ObjectFactory;
}}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class RootSignatureDX12;
//---------------------------------------------------------------------------//
  struct ShaderVertexInputLayoutDX12
  {
    std::vector<D3D12_INPUT_ELEMENT_DESC> myElements;
  };
//---------------------------------------------------------------------------//
  struct GpuProgramCompilerOutputDX12
  {
    ShaderVertexInputLayout clVertexInputLayout;
    GpuResourceInfoList vReadTextureInfos;
    GpuResourceInfoList vReadBufferInfos;
    GpuResourceInfoList vWriteTextureInfos;
    GpuResourceInfoList vWriteBufferInfos;
    ConstantBufferElementList myConstantBufferElements;
    ShaderStage eShaderStage;
    String myShaderCode;
    String myShaderFilename;  /// Platform-independent shader filename (e.g. "MaterialForward")

    GpuProgramPermutation myPermutation;
    RootSignatureDX12* myRootSignature;
    ComPtr<ID3DBlob>* myNativeData;
  };
//---------------------------------------------------------------------------//
  class GpuProgramDX12
  {
    friend class GpuProgramCompilerDX12;
    friend class RendererDX12;
    friend class IO::ObjectFactory;

  public:
    GpuProgramDX12();
    ~GpuProgramDX12();
    void Destroy();
    bool operator==(const GpuProgramDX12& anOther) const;
    bool operator==(const GpuProgramDesc& aDesc) const;
  //---------------------------------------------------------------------------//
    static ObjectName getTypeName() { return _N(GpuProgram); }
    void serialize(IO::Serializer* aSerializer);
    uint64 GetHash() const { return GetDescription().GetHash(); }

    GpuProgramDesc GetDescription() const;
    bool SetFromDescription(const GpuProgramDesc& aDesc);
    void SetFromCompilerOutput(const GpuProgramCompilerOutputDX12& aCompilerOutput);
        
    ShaderStage getShaderStage() const { return myStage; }
    /*const GpuResourceInfoList& getReadTextureInfoList() const { return myReadTextureInfos; }
    const GpuResourceInfoList& getReadBufferInfoList() const { return myReadBufferInfos; }
    const GpuResourceInfoList& getWriteTextureInfoList() const { return myWriteTextureInfos; }
    const GpuResourceInfoList& getWriteBufferInfoList() const { return myWriteBufferInfos; }
    const ShaderVertexInputLayout* getVertexInputLayout() const { return &myInputLayout; } */

    ComPtr<ID3DBlob>& getNativeData() { return myNativeData; }
    const D3D12_SHADER_BYTECODE& getNativeByteCode() const { return myNativeByteCode; }

    /// Returns the pipeline state embedded into the HLSL code
    const RootSignatureDX12* GetRootSignature() { return myRootSignature; }

  private:
    String mySourcePath;
    ShaderStage myStage;
    GpuProgramPermutation myPermutation;

    /*
    GpuResourceInfoList myReadTextureInfos;
    GpuResourceInfoList myReadBufferInfos;
    GpuResourceInfoList myWriteTextureInfos;
    GpuResourceInfoList myWriteBufferInfos;
    */
    ShaderVertexInputLayout myInputLayout;

    ComPtr<ID3DBlob> myNativeData;
    RootSignatureDX12* myRootSignature;
    
    D3D12_SHADER_BYTECODE myNativeByteCode;
  };
//---------------------------------------------------------------------------//
}}}

#endif


