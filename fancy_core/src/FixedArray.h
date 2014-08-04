#ifndef INCLUDE_FIXEDARRAY_H
#define INCLUDE_FIXEDARRAY_H

#include "FancyCorePrerequisites.h"

namespace FANCY { namespace Core {
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  class FixedArrayDatastore_Stack
  {
    public:
      FixedArrayDatastore_Stack();
      virtual ~FixedArrayDatastore_Stack();

    protected:
      T m_Array[capacity];
  };
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  FixedArrayDatastore_Stack<T, capacity>::~FixedArrayDatastore_Stack()
  {

  }
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  FixedArrayDatastore_Stack<T, capacity>::FixedArrayDatastore_Stack()
  {

  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  class FixedArrayDatastore_Heap
  {
  public:
    FixedArrayDatastore_Heap();
    virtual ~FixedArrayDatastore_Heap();

  protected:
    T* m_Array;
  };
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  FixedArrayDatastore_Heap<T, capacity>::FixedArrayDatastore_Heap()
  {
    m_Array = reinterpret_cast<T*>(FANCY_ALLOCATE(sizeof(T) * capacity));
    ASSERT(m_Array != nullptr);
  }
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  FixedArrayDatastore_Heap<T, capacity>::~FixedArrayDatastore_Heap()
  {
    ASSERT(m_Array != nullptr);
    FANCY_FREE(m_Array);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  template<class T, uint32 capacity, class StorageType = FixedArrayDatastore_Stack>
  class FixedArray : StorageType<T, capacity>
  {
    public:
      FixedArray();
      virtual ~FixedArray();
    //---------------------------------------------------------------------------//  
      uint32 capacity() const {return capacity;}
      uint32 size() const {return m_u32Size;}
      void clear() {m_u32Size = 0u;}
      void push_back(const T& clElement) {ASSERT_M(m_u32Size < m_u32Capacity, "Array is full"); m_Array[m_u32Size++] = clElement;}
    //---------------------------------------------------------------------------//
      const T& operator[](const uint32 u32Index) const {ASSERT(u32Index < m_u32Size); return m_Array[u32Index];}
      T& operator[](const uint32 u32Index) {ASSERT(u32Index < m_u32Size); return m_Array[u32Index];}
    //---------------------------------------------------------------------------//
    private:
      uint32 m_u32Size;
  };
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity, class StorageType /* = FixedArrayDatastore_Stack */>
  FixedArray<T, capacity, StorageType>::FixedArray() :
    m_u32Size(0), 
    StorageType<T, capacity>()
  {

  }
//---------------------------------------------------------------------------//
} }  // end of namespace FANCY::Core


#endif  // INCLUDE_FIXEDARRAY_H
