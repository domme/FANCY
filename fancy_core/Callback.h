#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  template<typename SignatureT>
  struct Callback;
//---------------------------------------------------------------------------//
  template<typename ReturnT, typename... Args>
  struct Callback<ReturnT(Args...)>
  {
    Callback() : myInstance(nullptr) {}
    Callback(const std::function<ReturnT(Args...)>& aFunction, void* anInstance)
      : myInstance(anInstance)
    {
      myFunction = aFunction;
    }

    std::function<ReturnT(Args...)> myFunction;
    void* myInstance;
  };
//---------------------------------------------------------------------------//


//---------------------------------------------------------------------------//
// Bind 0 Args
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT>
  Callback<ReturnT(void)>
    Bind(ClassT* anInstance, ReturnT(ClassT::*aMemFnPtr) (void))
  {
    std::function<ReturnT(void)> func = std::bind(aMemFnPtr, anInstance);
    return Callback<ReturnT(void)>(func, anInstance);
  }
//---------------------------------------------------------------------------//
  template<class ReturnT>
  Callback<ReturnT(void)>
    Bind(ReturnT(*aFnPtr) (void))
  {
    std::function<ReturnT(void)> func = aFnPtr;
    return Callback<ReturnT(void)>(func, nullptr);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Bind 1 Arg
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T>
 Callback<ReturnT(Arg1T)>
    Bind(ClassT* anInstance, ReturnT(ClassT::*aMemFnPtr) (Arg1T))
  {
    std::function<ReturnT(Arg1T)> func = std::bind(aMemFnPtr, anInstance, std::placeholders::_1);
    return Callback<ReturnT(Arg1T)>(func, anInstance);
  }
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T>
  Callback<ReturnT(Arg1T)>
    Bind(ClassT* anInstance, ReturnT(ClassT::*aMemFnPtr) (Arg1T), const Arg1T& anArg)
  {
    std::function<ReturnT(Arg1T)> func = std::bind(aMemFnPtr, anInstance, anArg);
    return Callback<ReturnT(Arg1T)>(func, anInstance);
  }
//---------------------------------------------------------------------------//
    template<class ReturnT, class Arg1T>
 Callback<ReturnT(Arg1T)>
    Bind(ReturnT(*aFnPtr) (Arg1T))
  {
    std::function<ReturnT(Arg1T)> func = aFnPtr;
    return Callback<ReturnT(Arg1T)>(func, nullptr);
  }
//---------------------------------------------------------------------------//
  template<class ReturnT, class Arg1T>
  Callback<ReturnT(Arg1T)>
    Bind(ReturnT(*aFnPtr) (Arg1T), const Arg1T& anArg)
  {
    std::function<ReturnT(Arg1T)> func = std::bind(aFnPtr, anArg);
    return Callback<ReturnT(Arg1T)>(func, nullptr);
  }
//---------------------------------------------------------------------------//


//---------------------------------------------------------------------------//
// Bind 2 Args
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T>
  Callback<ReturnT(Arg1T, Arg2T)>
    Bind(ClassT* anInstance, ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T))
  {
    std::function<ReturnT(Arg1T, Arg2T)> func = std::bind(aMemFnPtr, anInstance, std::placeholders::_1, std::placeholders::_2);
    return Callback<ReturnT(Arg1T, Arg2T)>(func, anInstance);
  }
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T>
  Callback<ReturnT(Arg1T, Arg2T)>
    Bind(ClassT* anInstance, ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T), const Arg1T& anArg1, const Arg2T& anArg2)
  {
    std::function<ReturnT(Arg1T, Arg2T)> func = std::bind(aMemFnPtr, anInstance, anArg1, anArg2);
    return Callback<ReturnT(Arg1T, Arg2T)>(func, anInstance);
  }
//---------------------------------------------------------------------------//
  template<class ReturnT, class Arg1T, class Arg2T>
  Callback<ReturnT(Arg1T, Arg2T)>
    Bind(ReturnT(*aFnPtr) (Arg1T, Arg2T))
  {
    std::function<ReturnT(Arg1T, Arg2T)> func = std::bind(aFnPtr, std::placeholders::_1, std::placeholders::_2);
    return Callback<ReturnT(Arg1T, Arg2T)>(func, nullptr);
  }
//---------------------------------------------------------------------------//
  template<class ReturnT, class Arg1T, class Arg2T>
  Callback<ReturnT(Arg1T, Arg2T)>
    Bind(ReturnT(*aFnPtr) (Arg1T, Arg2T), const Arg1T& anArg1, const Arg2T& anArg2)
  {
    std::function<ReturnT(Arg1T, Arg2T)> func = std::bind(aFnPtr, anArg1, anArg2);
    return Callback<ReturnT(Arg1T, Arg2T)>(func, nullptr);
  }
//---------------------------------------------------------------------------//


//---------------------------------------------------------------------------//
// Bind 3 Args
//---------------------------------------------------------------------------//
template<class ClassT, class ReturnT, class Arg1T, class Arg2T, class Arg3T>
Callback<ReturnT(Arg1T, Arg2T, Arg3T)>
  Bind(ClassT* anInstance,ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T, Arg3T))
{
  std::function<ReturnT(Arg1T, Arg2T, Arg3T)> func
    = std::bind(aMemFnPtr, anInstance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  return Callback<ReturnT(Arg1T, Arg2T, Arg3T)>(func, anInstance);
}
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T, class Arg3T>
  Callback<ReturnT(Arg1T, Arg2T, Arg3T)>
    Bind(ClassT* anInstance,
      ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T, Arg3T),
      const Arg1T& anArg1,
      const Arg2T& anArg2,
      const Arg3T& anArg3)
  {
    std::function<ReturnT(Arg1T, Arg2T, Arg3T)> func
      = std::bind(aMemFnPtr, anInstance, anArg1, anArg2, anArg3);
    return Callback<ReturnT(Arg1T, Arg2T, Arg3T)>(func, anInstance);
  }
//---------------------------------------------------------------------------//
  template<class ReturnT, class Arg1T, class Arg2T, class Arg3T>
Callback<ReturnT(Arg1T, Arg2T, Arg3T)>
  Bind(ReturnT(*aFnPtr) (Arg1T, Arg2T, Arg3T))
{
  std::function<ReturnT(Arg1T, Arg2T, Arg3T)> func
    = std::bind(aFnPtr, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  return Callback<ReturnT(Arg1T, Arg2T, Arg3T)>(func, nullptr);
}
//---------------------------------------------------------------------------//
  template<class ReturnT, class Arg1T, class Arg2T, class Arg3T>
  Callback<ReturnT(Arg1T, Arg2T, Arg3T)>
    Bind(ReturnT(*aFnPtr) (Arg1T, Arg2T, Arg3T),
      const Arg1T& anArg1,
      const Arg2T& anArg2,
      const Arg3T& anArg3)
  {
    std::function<ReturnT(Arg1T, Arg2T, Arg3T)> func
      = std::bind(aFnPtr, anArg1, anArg2, anArg3);
    return Callback<ReturnT(Arg1T, Arg2T, Arg3T)>(func, nullptr);
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
// Bind 4 Args
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T, class Arg3T, class Arg4T>
  Callback<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)>
    Bind(ClassT* anInstance, ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T, Arg3T, Arg4T))
  {
    std::function<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)> func
      = std::bind(aMemFnPtr, anInstance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    return Callback<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)>(func, anInstance);
  }
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T, class Arg3T, class Arg4T>
  Callback<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)>
    Bind(ClassT* anInstance, ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T, Arg3T, Arg4T), const Arg1T& anArg1, const Arg2T& anArg2, const Arg3T& anArg3, const Arg4T& anArg4)
  {
    std::function<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)> func
      = std::bind(aMemFnPtr, anInstance, anArg1, anArg2, anArg3, anArg4);

    return Callback<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)>(func, anInstance);
  }
//---------------------------------------------------------------------------//
    template<class ReturnT, class Arg1T, class Arg2T, class Arg3T, class Arg4T>
  Callback<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)>
    Bind(ReturnT(*aFnPtr) (Arg1T, Arg2T, Arg3T, Arg4T))
  {
    std::function<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)> func
      = std::bind(aFnPtr, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    return Callback<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)>(func, nullptr);
  }
//---------------------------------------------------------------------------//
  template<class ReturnT, class Arg1T, class Arg2T, class Arg3T, class Arg4T>
  Callback<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)>
    Bind(ReturnT(*aFnPtr) (Arg1T, Arg2T, Arg3T, Arg4T), const Arg1T& anArg1, const Arg2T& anArg2, const Arg3T& anArg3, const Arg4T& anArg4)
  {
    std::function<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)> func
      = std::bind(aFnPtr, anArg1, anArg2, anArg3, anArg4);

    return Callback<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)>(func, nullptr);
  }
//---------------------------------------------------------------------------//


}

