#pragma once
#include "FancyCorePrerequisites.h"
#include "GpuProgramFeatures.h"
#include "MathUtil.h"
#include "DescriptionBase.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc : public DescriptionBase
  {
    GpuProgramDesc() : myShaderFileName(""), myShaderStage(0u), myMainFunction("main") {}
    bool operator==(const GpuProgramDesc& anOther) const;
    
    uint64 GetHash() const override;
    ~GpuProgramDesc() override {}
    ObjectName GetTypeName() const override { return _N(GpuProgram); }
    bool IsEmpty() const override { return myShaderFileName.empty(); }

    void Serialize(IO::Serializer* aSerializer) override;
    String myShaderFileName;
    String myMainFunction;
    uint myShaderStage;
    GpuProgramPermutation myPermutation;
  };
//---------------------------------------------------------------------------//
  inline bool GpuProgramDesc::operator==(const GpuProgramDesc& anOther) const
  {
    return myShaderFileName == anOther.myShaderFileName
      && myShaderStage == anOther.myShaderStage
      && myPermutation == anOther.myPermutation
      && myMainFunction == anOther.myMainFunction;
  }
//---------------------------------------------------------------------------//
  inline uint64 GpuProgramDesc::GetHash() const
  {
    uint64 hash;
    MathUtil::hash_combine(hash, MathUtil::Hash(myShaderFileName));
    MathUtil::hash_combine(hash, myShaderStage);
    MathUtil::hash_combine(hash, myPermutation.GetHash());
    MathUtil::hash_combine(hash, MathUtil::Hash(myMainFunction));
    return hash;
  }
//---------------------------------------------------------------------------//
  inline void GpuProgramDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&myShaderFileName, "myShaderFileName");
    aSerializer->Serialize(&myMainFunction, "myMainFunction");
    aSerializer->Serialize(&myShaderStage, "myShaderStage");
    aSerializer->Serialize(&myPermutation, "myPermutation");
  }
//---------------------------------------------------------------------------//
} }

