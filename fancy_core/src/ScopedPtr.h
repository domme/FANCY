#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  template<class T>
  class ScopedPtr
  {
  public:
  //---------------------------------------------------------------------------//
    ScopedPtr()
      : myRawPtr(nullptr)
    { }
  //---------------------------------------------------------------------------//
    explicit ScopedPtr(T* aRawPtr) { Reset(aRawPtr); }
    ~ScopedPtr() { Reset(nullptr); }
  //---------------------------------------------------------------------------//
    operator T*() { return Get(); }
    T* operator->() { ASSERT(myRawPtr != nullptr); return myRawPtr; }
    void operator=(T* anOther) { Reset(anOther); }
    
    template<class T2>
    void operator=(const ScopedPtr<T2>& anOther) { static_assert(sizeof(T2) == 0, "Can't assign another ScopedPtr"); }
  //---------------------------------------------------------------------------//
    void Reset(T* aRawPtr)
    {
      if (aRawPtr == myRawPtr)
        return;

      delete myRawPtr;
      myRawPtr = aRawPtr;
    }
  //---------------------------------------------------------------------------//
    T* Release()
    {
      T* ptr = myRawPtr;
      myRawPtr = nullptr;

      return ptr;
    }
  //---------------------------------------------------------------------------//
    T* Get() { return myRawPtr; }
    
  //---------------------------------------------------------------------------//
  private:
    T* myRawPtr;
  };
//---------------------------------------------------------------------------//
}
