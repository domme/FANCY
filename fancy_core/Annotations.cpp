#include "fancy_core_precompile.h"
#include "Annotations.h"

namespace Fancy
{
  static AnnotationTagData ourAnnotationTagData[UINT8_MAX] = { 0u };
  static uint ourNextFreeIdx = 0u;

  uint8 Annotations::CreateTag(const char* aName, uint aColor)
  {
    for (uint i = 0u; i < ourNextFreeIdx; ++i)
      ASSERT(strcmp(aName, ourAnnotationTagData[i].myName) == 0);

    ASSERT(ourNextFreeIdx < UINT8_MAX);

    AnnotationTagData& tagData = ourAnnotationTagData[ourNextFreeIdx];
    ASSERT(strlen(aName) <= ARRAY_LENGTH(tagData.myName));

    strcpy(tagData.myName, aName);
    tagData.myColor = aColor;
    
    return static_cast<uint8>(ourNextFreeIdx++);
  }
}
