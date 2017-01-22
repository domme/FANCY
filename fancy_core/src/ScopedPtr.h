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
    ScopedPtr(T* aRawPtr) { Reset(aRawPtr); }
    ~ScopedPtr() { Reset(nullptr); }
  //---------------------------------------------------------------------------//
    operator T*() const { return Get(); }

    const T* operator->() const { ASSERT(myRawPtr != nullptr); return myRawPtr; }
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
    T* Get() const { return myRawPtr; }
  //---------------------------------------------------------------------------//
  private:
    T* myRawPtr;
  };
//---------------------------------------------------------------------------//
}
