#include "fancy_core_precompile.h"
#include "GpuProgramFeatures.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Internal 
  {
    uint64 sortAndGetHash(DynamicArray<GpuProgramFeature>& _vFeatures)
    {
      std::sort(_vFeatures.begin(), _vFeatures.end());
      
      uint64 hash = 0u;
      for (uint i = 0u; i < _vFeatures.size(); ++i)
      {
        MathUtil::hash_combine(hash, static_cast<uint>(_vFeatures[i]));
      }

      return hash;
    }
  }
//---------------------------------------------------------------------------//
  String GpuProgramPermutation::featureToDefineString(GpuProgramFeature _eFeature)
  {
    switch (_eFeature)
    {
      case GpuProgramFeature::FEAT_ALBEDO_TEXTURE: return "FEAT_ALBEDO_TEXTURE";
      case GpuProgramFeature::FEAT_ALBEDO_COLOR: return "FEAT_ALBEDO_COLOR";
      case GpuProgramFeature::FEAT_NORMAL_MAPPED: return "FEAT_NORMAL_MAPPED";
      case GpuProgramFeature::FEAT_SPECULAR: return "FEAT_SPECULAR";
      case GpuProgramFeature::FEAT_SPECULAR_TEXTURE: return "FEAT_SPECULAR_TEXTURE";
      case GpuProgramFeature::FEAT_SPECULAR_POWER_TEXTURE: return "FEAT_SPECULAR_POWER_TEXTURE";
      case GpuProgramFeature::FEAT_TRANSPARENT: return "FEAT_TRANSPARENT";
      case GpuProgramFeature::FEAT_CUTOUT: return "FEAT_CUTOUT";
    default:
      ASSERT(false);
      return "";
    }
  }
//---------------------------------------------------------------------------//
  bool GpuProgramPermutation::hasFeature( GpuProgramFeature _eFeature ) const
  {
    for (uint i = 0u; i < m_vFeatures.size(); ++i)
    {
      if (m_vFeatures[i] == _eFeature)
      {
        return true;
      }
    }

    return false;
  }
//---------------------------------------------------------------------------//
  void GpuProgramPermutation::addFeature( GpuProgramFeature _eFeature )
  {
    if (!hasFeature((_eFeature)))
    {
      m_vFeatures.push_back(_eFeature);
      m_uHash = Internal::sortAndGetHash(m_vFeatures);
    }
  }
//---------------------------------------------------------------------------//
  void GpuProgramPermutation::setFeatures(const DynamicArray<GpuProgramFeature>& _vFeatures)
  {
    m_vFeatures = _vFeatures;
    m_uHash = Internal::sortAndGetHash(m_vFeatures);
  }
//---------------------------------------------------------------------------//
}
