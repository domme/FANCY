#pragma once

#include "FancyCoreDefines.h"

namespace Fancy
{
  struct AnnotationTagData
  {
    char myName[260];
    uint myColor;
  };

  namespace Annotations
  {
    uint8 CreateTag(const char* aName, uint aColor);
  }

#define ANNOTATION_CREATE_TAG(anId, aName, aColor) \
  const uint8 anId = Annotations::CreateTag(aName, aColor);

#define ANNOTATION_USE_TAG(anId) \
  extern const uint8 anId;
}


