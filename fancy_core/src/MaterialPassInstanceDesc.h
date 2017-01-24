#pragma once

#include "FancyCorePrerequisites.h"
#include "GpuBufferDesc.h"
#include "TextureDesc.h"
#include "TextureSamplerDesc.h"
#include "MaterialPassDesc.h"

namespace Fancy { namespace IO {
    class Serializer;
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct MaterialPassInstanceDesc
  {
    MaterialPassDesc myMaterialPass;
    TextureDesc myReadTextures[Constants::kMaxNumReadTextures];
    TextureDesc myWriteTextures[Constants::kMaxNumWriteTextures];
    GpuBufferDesc myReadBuffers[Constants::kMaxNumReadBuffers];
    GpuBufferDesc myWriteBuffers[Constants::kMaxNumWriteBuffers];
    TextureSamplerDesc myTextureSamplers[Constants::kMaxNumTextureSamplers];

    uint64 GetHash() const;
    bool operator==(const MaterialPassInstanceDesc& anOther) const;
    void Serialize(IO::Serializer* aSerializer);
  };
//---------------------------------------------------------------------------//
} }
