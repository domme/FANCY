#pragma once

#include "FancyCoreDefines.h"
#include "FC_String.h"
#include "DynamicArray.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class GpuProgramFeature
  {
    FEAT_ALBEDO_TEXTURE = 0,
    FEAT_ALBEDO_COLOR,
    FEAT_NORMAL_MAPPED,
    FEAT_SPECULAR,
    FEAT_SPECULAR_TEXTURE,
    FEAT_SPECULAR_POWER_TEXTURE,
    FEAT_TRANSPARENT,
    FEAT_CUTOUT,

    NUM
  };
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  class GpuProgramPermutation
  {
    public:
      uint64 GetHash() const { return m_uHash; }

      bool operator==(const GpuProgramPermutation& anOtherPermutation) const {
        return m_uHash == anOtherPermutation.m_uHash;
      }

      GpuProgramPermutation() : m_uHash(0u) {}
      ~GpuProgramPermutation() = default;

      static String featureToDefineString(GpuProgramFeature _eFeature);
      
      void addFeature(GpuProgramFeature _eFeature);
      bool hasFeature(GpuProgramFeature _eFeature) const;
      void setFeatures(const DynamicArray<GpuProgramFeature>& _vFeatures);
      const DynamicArray<GpuProgramFeature>& getFeatureList() const {return m_vFeatures;}

    private:
      DynamicArray<GpuProgramFeature> m_vFeatures;
      uint64 m_uHash;
  };
//---------------------------------------------------------------------------//
}


