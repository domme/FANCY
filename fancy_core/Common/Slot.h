#pragma once

#include "Callback.h"

#include "EASTL/algorithm.h"
#include "EASTL/fixed_vector.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  template<typename SignatureT>
  struct Slot;
//---------------------------------------------------------------------------//
  template<typename ReturnT, class... Args>
  struct Slot<ReturnT(Args...)>
  {
    //---------------------------------------------------------------------------//
      void operator()(const Args&... someArgs)
      {
        for (CallbackT& callback : myCallbacks)
          callback.myFunction(someArgs...);
      }
  //---------------------------------------------------------------------------//
      template<typename ClassT>
      void Connect(ClassT* anObserver, ReturnT(ClassT::*aMemFnPtr)(Args...))
      {
        Callback<ReturnT(Args...)> tempCallback = Bind(anObserver, aMemFnPtr);
        Connect(tempCallback.myFunction, anObserver);
      }
  //---------------------------------------------------------------------------//
      void Connect(std::function<ReturnT(Args...)>& aFunction, void* anInstance = nullptr)
      {
        if (anInstance != nullptr && 
          std::find_if(myCallbacks.begin(), myCallbacks.end(), [anInstance](const CallbackT& entry) {
          return entry.myInstance == anInstance;
        }) != myCallbacks.end())
        {
          return;
        }

        CallbackT callback;
        callback.myFunction = aFunction;
        callback.myInstance = anInstance;

        myCallbacks.push_back(callback);
      }
    //---------------------------------------------------------------------------//
      void DetachObserver(void* anObserver)
      {
        if (anObserver == nullptr)
          return;

        eastl::remove_if(myCallbacks.begin(), myCallbacks.end(), [anObserver](const CallbackT& entry) {
          return entry.myInstance == anObserver;
        });
      }
    //---------------------------------------------------------------------------//
  private:
    using CallbackT = Callback<ReturnT(Args...)>;
    eastl::fixed_vector<CallbackT, 8> myCallbacks;
  };
//---------------------------------------------------------------------------//
}
