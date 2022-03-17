#include "fancy_core_precompile.h"
#include "Annotations.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct AnnotationDataHolder
  {
    static AnnotationDataHolder& GetInstance()
    {
      static AnnotationDataHolder instance;
      return instance;
    }

    uint16 CreateTag(const char* aName, uint aColor)
    {
      for (uint i = 0u; i < myNextFreeIdx; ++i)
        ASSERT(strcmp(aName, myData[i].myName) != 0);

      ASSERT(myNextFreeIdx < UINT16_MAX);

      AnnotationTagData& tagData = myData[myNextFreeIdx];
      ASSERT(strlen(aName) <= ARRAY_LENGTH(tagData.myName));

      memcpy(tagData.myName, aName, strlen(aName));
      tagData.myColor = aColor;

      return static_cast<uint8>(myNextFreeIdx++);
    }

    const AnnotationTagData& GetTagData(uint16 aTag)
    {
      ASSERT(aTag < myNextFreeIdx);
      return myData[aTag];
    }

  private:
    AnnotationDataHolder() = default;
    ~AnnotationDataHolder() = default;

    AnnotationTagData myData[UINT16_MAX];
    uint myNextFreeIdx = 1u;  // 0 is the default tag set in the constructor of AnnotationTagData
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  uint16 Annotations::CreateTag(const char* aName, uint aColor)
  {
    return AnnotationDataHolder::GetInstance().CreateTag(aName, aColor);
  }
//---------------------------------------------------------------------------//
  const AnnotationTagData& Annotations::GetTagData(uint16 aTag)
  {
    return AnnotationDataHolder::GetInstance().GetTagData(aTag);
  }
//---------------------------------------------------------------------------//
}
