#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T>
  std::function<ReturnT(Arg1T)> 
    Bind( ClassT* anInstance, 
          ReturnT(ClassT::*aMemFnPtr) (Arg1T))
  {
    std::function<ReturnT(Arg1T)> func = std::bind(aMemFnPtr, anInstance, std::placeholders::_1);
    return func;
  }
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T>
  std::function<ReturnT(Arg1T)>
    Bind(ClassT* anInstance,
      ReturnT(ClassT::*aMemFnPtr) (Arg1T),
      const Arg1T& anArg)
  {
    std::function<ReturnT(Arg1T)> func = std::bind(aMemFnPtr, anInstance, anArg);
    return func;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T>
  std::function<ReturnT(Arg1T, Arg2T)> 
    Bind( ClassT* anInstance, 
          ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T))
  {
    std::function<ReturnT(Arg1T, Arg2T)> func = std::bind(aMemFnPtr, anInstance, std::placeholders::_1, std::placeholders::_2);
    return func;
  }
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T>
  std::function<ReturnT(Arg1T, Arg2T)>
    Bind(ClassT* anInstance,
      ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T),
      const Arg1T& anArg1,
      const Arg2T& anArg2)
  {
    std::function<ReturnT(Arg1T, Arg2T)> func = std::bind(aMemFnPtr, anInstance, anArg1, anArg2);
    return func;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T, class Arg3T>
  std::function<ReturnT(Arg1T, Arg2T, Arg3T)>
    Bind( ClassT* anInstance, 
          ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T, Arg3T))
  {
    std::function<ReturnT(Arg1T, Arg2T, Arg3T)> func
      = std::bind(aMemFnPtr, anInstance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    return func;
  }
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T, class Arg3T>
  std::function<ReturnT(Arg1T, Arg2T, Arg3T)>
    Bind(ClassT* anInstance,
      ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T, Arg3T),
      const Arg1T& anArg1,
      const Arg2T& anArg2,
      const Arg3T& anArg3)
  {
    std::function<ReturnT(Arg1T, Arg2T, Arg3T)> func
      = std::bind(aMemFnPtr, anInstance, anArg1, anArg2, anArg3);
    return func;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T, class Arg3T, class Arg4T>
  std::function<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)> 
    Bind( ClassT* anInstance, 
          ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T, Arg3T, Arg4T))
  {
    std::function<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)> func
      = std::bind(aMemFnPtr, anInstance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    return func;
  }
//---------------------------------------------------------------------------//
  template<class ClassT, class ReturnT, class Arg1T, class Arg2T, class Arg3T, class Arg4T>
  std::function<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)>
    Bind(ClassT* anInstance,
      ReturnT(ClassT::*aMemFnPtr) (Arg1T, Arg2T, Arg3T, Arg4T),
      const Arg1T& anArg1,
      const Arg2T& anArg2,
      const Arg3T& anArg3,
      const Arg4T& anArg4)
  {
    std::function<ReturnT(Arg1T, Arg2T, Arg3T, Arg4T)> func
      = std::bind(aMemFnPtr, anInstance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    return func;
  }
//---------------------------------------------------------------------------//
  
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
        for (ObserverEntry& observer : myObservers)
          observer.myFunction(someArgs...);
      }
    //---------------------------------------------------------------------------//
      template<typename ClassT>
      void Connect(ClassT* anObserver, ReturnT(ClassT::*aMemFnPtr)(Args...))
      {
        Connect(Bind(anObserver, aMemFnPtr), anObserver);
      }
    //---------------------------------------------------------------------------//
      void Connect(std::function<ReturnT(Args...)>& aFunction, void* anObserver = nullptr)
      {
        if (anObserver != nullptr && 
          std::find_if(myObservers.begin(), myObservers.end(), [anObserver](const ObserverEntry& entry) {
          return entry.myInstance == anObserver;
        }) != myObservers.end())
        {
          return;
        }

        ObserverEntry entry;
        entry.myFunction = aFunction;
        entry.myInstance = anObserver;

        myObservers.push_back(entry);
      }
    //---------------------------------------------------------------------------//
      void DetachObserver(void* anObserver)
      {
        if (anObserver == nullptr)
          return;

        std::remove_if(myObservers.begin(), myObservers.end(), [anObserver](const ObserverEntry& entry) {
          return entry.myInstance == anObserver;
        });
      }
    //---------------------------------------------------------------------------//
  private:
    struct ObserverEntry
    {
      std::function<ReturnT(Args...)> myFunction;
      void* myInstance;
    };

    std::vector<ObserverEntry> myObservers;
  };
//---------------------------------------------------------------------------//
}