#ifndef INCLUDE_TEXTURE_H
#define INCLUDE_TEXTURE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "Serializable.h"
#include PLATFORM_DEPENDENT_INCLUDE_TEXTURE

namespace Fancy { namespace Rendering {

class Texture : public PLATFORM_DEPENDENT_NAME(Texture)
{
public:
  enum { IsSerializable = 1 }; using DescT =TextureDesc; template<class dummy> static IO::DataType getDataTypePtr() { return IO::DataType(IO::EBaseDataType::ResourcePtr, &Fancy::Internal::MetaTableResourceImpl<std::shared_ptr<Texture>>::ourVTable); }
};

} } // end of namespace Fancy::Rendering


#endif  // INCLUDE_TEXTURE_H