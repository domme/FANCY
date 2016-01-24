#ifndef INCLUDE_GPUPROGRAMFEATURES_H
#define INCLUDE_GPUPROGRAMFEATURES_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "FixedArray.h"
#include "Serializable.h"

namespace Fancy { namespace Rendering {
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
  typedef FixedArray<GpuProgramFeature, 128u> GpuProgramFeatureList;
//---------------------------------------------------------------------------//
  class GpuProgramPermutation
  {
    public:
      SERIALIZABLE(GpuProgramPermutation)
        
      static ObjectName getTypeName() { return _N(GpuProgramPermutation); }
      uint64 GetHash() const { return m_uHash; }
      void serialize(IO::Serializer* aSerializer);

      bool operator==(const GpuProgramPermutation& anOtherPermutation) const {
        return m_uHash == anOtherPermutation.m_uHash;
      }

      GpuProgramPermutation() : m_uHash(0u) {}
      ~GpuProgramPermutation() {}

      static String featureToDefineString(GpuProgramFeature _eFeature);
      

      void addFeature(GpuProgramFeature _eFeature);
      bool hasFeature(GpuProgramFeature _eFeature) const;
      void setFeatures(const GpuProgramFeatureList& _vFeatures);
      const GpuProgramFeatureList& getFeatureList() const {return m_vFeatures;}

    private:
      GpuProgramFeatureList m_vFeatures;
      uint64 m_uHash;
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_GPUPROGRAMCOMPILER_H



