#ifndef INCLUDE_STRINGUTIL_H
#define INCLUDE_STRINGUTIL_H

#include "FancyCorePrerequisites.h"

namespace Fancy {

  class StringUtil
  {
  public:
    template<class T>
    static std::string toString(const T& _val)
    {
      std::stringstream ss;
      ss << _val;
      return ss.str();
    }
  };

}  // end of namespace Fancy


#endif  // INCLUDE_STRINGUTIL_H