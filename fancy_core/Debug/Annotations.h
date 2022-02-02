#pragma once

#include "Common/FancyCoreDefines.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct AnnotationTagData
  {
    char myName[260] = "Default";
    uint myColor = 0xFF7C7C7C;
  };
//---------------------------------------------------------------------------//
  namespace Annotations
  {
    uint16 CreateTag(const char* aName, uint aColor);
    const AnnotationTagData& GetTagData(uint16 aTag);
  }
//---------------------------------------------------------------------------//
}

#define ANNOTATION_CREATE_TAG(anId, aName, aColor) \
  const uint16 anId = Fancy::Annotations::CreateTag(aName, aColor);

#define ANNOTATION_USE_TAG(anId) \
  extern const uint16 anId;