#pragma once

#include "Log.h"

namespace Fancy {

  /*
   * Represents a simple double-linked list that will never deallocate memory but will only allocate if needed.
   * Intended for lists that need high-performance removal of elements and/or only ever grow to a fixed size.
   * Elements are allocated in groups of pages. Each page is allocated in a continuous memory region. 
   * Adding and removing elements in random positions in the list (as opposed to at the end) will result in memory fragmentation
   * that will degrade performance. In such use-cases, a regular std::list might be a better choice.
   */
  template<class T, uint64 PageSize>
  class GrowingList
  {
  public:
    struct Element;

    struct Iterator
    {
      friend class GrowingList;

      Iterator() : myElement(nullptr) {}
      Iterator(Element* anElement) : myElement(anElement) {}
      T& operator*() { ASSERT(myElement != nullptr); return myElement->myData; }
      T* operator->() { ASSERT(myElement != nullptr); return &myElement->myData; }
      explicit operator bool() const { return myElement != nullptr; }
      bool operator==(const Iterator& anOther) const { return myElement == anOther.myElement; }
      bool operator!=(const Iterator& anOther) const { return myElement != anOther.myElement; }
      Iterator Next() const { return myElement != nullptr ? myElement->myNext : nullptr; }
      Iterator Prev() const { return myElement != nullptr ? myElement->myPrev: nullptr; }
      bool HasNext() const { return myElement && myElement->myNext; }
      bool HasPrev() const { return myElement && myElement->myPrev; }
      Iterator operator++();
      Iterator operator--();

    private:
      Element* myElement = nullptr;
    };

    GrowingList();
    ~GrowingList();

    Iterator Begin();
    T& Back();
    Iterator Last();
    Iterator Invalid();
    Iterator Add(T aData);
    Iterator AddBefore(Iterator aPos, T aData);
    Iterator AddAfter(Iterator aPos, T aData);
    Iterator Remove(Iterator aPos);
    Iterator Find(const T& aData);
    Iterator Find(std::function<bool(const T&)> aPredicate);
    Iterator ReverseFind(const T& aData);
    Iterator ReverseFind(std::function<bool(const T&)> aPredicate);
    bool IsEmpty() const;
    void DestroyAll();

  private:
    struct Element
    {
      T myData;
      Element* myPrev = nullptr;
      Element* myNext = nullptr;
    };

    struct Page
    {
      Page();
      ~Page();

      Element* myElements = nullptr;
      uint myNextFreeElementIdx = 0u;
      Page* myNext = nullptr;
    };

    Element* AllocateElement();
    Page* FindPage(Iterator aPos);

    Page* myHeadPage = nullptr;
    Page* myTailPage = nullptr;
    Element* myHeadElement = nullptr;
    Element* myTailElement = nullptr;
    Element** myFreeElements = nullptr;
    uint myNextFreeFreeElement = 0u;
    uint myMaxNumFreeElements = PageSize;
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  GrowingList<T, PageSize>::Page::Page()
  {
    myElements = new Element[PageSize];
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  GrowingList<T, PageSize>::Page::~Page()
  {
    delete[] myElements;
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::Iterator::operator++()
  {
    myElement = myElement->myNext;
    return *this;
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::Iterator::operator--()
  {
    myElement = myElement->myPrev;
    return *this;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  GrowingList<T, PageSize>::GrowingList()
  {
    myMaxNumFreeElements = PageSize;
    myFreeElements = new Element*[myMaxNumFreeElements];
    memset(myFreeElements, 0u, sizeof(Element*) * myMaxNumFreeElements);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  GrowingList<T, PageSize>::~GrowingList()
  {
    DestroyAll();
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::Begin()
  {
    return Iterator(myHeadElement);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  T& GrowingList<T, PageSize>::Back()
  {
    ASSERT(!IsEmpty());
    return myTailElement->myData;
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::Last()
  {
    return Iterator(myTailElement);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::Invalid()
  {
    return Iterator(nullptr);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::Add(T aData)
  {
    Element* newElement = AllocateElement();
    newElement->myData = std::move(aData);

    newElement->myNext = nullptr;
    if (myHeadElement == nullptr)  // first element ever
    {
      newElement->myPrev = nullptr;
      myHeadElement = newElement;
      myTailElement = newElement;
    }
    else
    {
      newElement->myPrev = myTailElement;
      myTailElement->myNext = newElement;
      myTailElement = newElement;
    }
    
    return Iterator(newElement);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::AddBefore(Iterator aPos, T aData)
  {
    ASSERT(FindPage(aPos) != nullptr);

    Element* newElement = AllocateElement();
    newElement->myData = std::move(aData);

    Element* pos = aPos.myElement;
    Element* posPrev = pos->myPrev;

    newElement->myNext = pos;
    newElement->myPrev = posPrev;
    pos->myPrev = newElement;

    if (posPrev == nullptr)
    {
      ASSERT(pos == myHeadElement);
      myHeadElement = newElement;
    }
    else
    {
      posPrev->myNext = newElement;
    }

    return Iterator(newElement);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::AddAfter(Iterator aPos, T aData)
  {
    ASSERT(FindPage(aPos) != nullptr);

    Element* newElement = AllocateElement();
    newElement->myData = std::move(aData);

    Element* pos = aPos.myElement;
    Element* posNext = pos->myNext;

    newElement->myPrev = pos;
    newElement->myNext = posNext;
    pos->myNext = newElement;

    if (posNext == nullptr)
    {
      ASSERT(pos == myTailElement);
      myTailElement = newElement;
    }
    else
    {
      posNext->myPrev = newElement;
    }

    return Iterator(newElement);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::Remove(Iterator aPos)
  {
    Page* page = FindPage(aPos);
    ASSERT(page != nullptr);

    Element* element = aPos.myElement;
    Element* prev = element->myPrev;
    Element* next = element->myNext;
    if (prev != nullptr)
      prev->myNext = next;
    if (next != nullptr)
      next->myPrev = prev;
    if (myHeadElement == element)
      myHeadElement = next;
    if (myTailElement == element)
      myTailElement = prev;

    // In the special case of element being the last element allocated from a page, we can free it again.
    if (page->myNextFreeElementIdx > 0 && element == &page->myElements[page->myNextFreeElementIdx - 1])
      --page->myNextFreeElementIdx;
    else  // Otherwise put it in the growing free elements list
    {
      if (myNextFreeFreeElement == myMaxNumFreeElements)  // Realloc free elements list
      {
        myMaxNumFreeElements += PageSize;
        Element** newFreeElementsList = new Element*[myMaxNumFreeElements];
        const uint sizeOldList = sizeof(Element*) * (myNextFreeFreeElement - 1u);
        memcpy(newFreeElementsList, myFreeElements, sizeOldList);
        memset(newFreeElementsList + sizeOldList, 0u, (sizeof(Element*) * myMaxNumFreeElements) - sizeOldList);
        delete[] myFreeElements;
        myFreeElements = newFreeElementsList;
      }

      myFreeElements[myNextFreeFreeElement++] = element;
    }

    // Destruct the element
    element->myPrev = nullptr;
    element->myNext = nullptr;
    element->myData.~T();

    return Iterator(prev);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::Find(const T& aData)
  {
    Iterator it = Begin();
    Iterator end = Invalid();
    while (it != end)
    {
      if ((*it) == aData)
        return it;

      ++it;
    }

    return end;
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::Find(std::function<bool(const T&)> aPredicate)
  {
    Iterator it = Begin();
    Iterator end = Invalid();
    while (it != end)
    {
      if (aPredicate(*it))
        return it;

      ++it;
    }

    return end;
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::ReverseFind(const T& aData)
  {
    Iterator it(myTailElement);
    while(it.myElement != nullptr)
    {
      if ((*it) == aData)
        return it;

      --it;
    }

    return Invalid();
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::ReverseFind(
    std::function<bool(const T&)> aPredicate)
  {
    Iterator it(myTailElement);
    while (it.myElement != nullptr)
    {
      if (aPredicate(*it))
        return it;

      --it;
    }

    return Invalid();
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  bool GrowingList<T, PageSize>::IsEmpty() const
  {
    return myHeadElement == nullptr;
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  void GrowingList<T, PageSize>::DestroyAll()
  {
    Page* page = myHeadPage;
    while (page != nullptr)
    {
      Page* nextPage = page->myNext;
      delete page;
      page = nextPage;
    }

    myHeadPage = nullptr;
    myTailPage = nullptr;
    delete[] myFreeElements;
    myFreeElements = nullptr;
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Element* GrowingList<T, PageSize>::AllocateElement()
  {
    if (myNextFreeFreeElement > 0)  // We have an element that we can re-use
      return myFreeElements[--myNextFreeFreeElement];

    if (myTailPage == nullptr || myTailPage->myNextFreeElementIdx == PageSize) // A new page is needed
    {
      Page* newPage = new Page;
      if (myHeadPage == nullptr)// First page ever created
      {
        myHeadPage = newPage;
        myTailPage = newPage;
      }
      else // We have a tail-page that is full
      {
        myTailPage->myNext = newPage;
        myTailPage = newPage;
      }
    }

    return &myTailPage->myElements[myTailPage->myNextFreeElementIdx++];
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Page* GrowingList<T, PageSize>::FindPage(Iterator aPos)
  {
    Page* page = myHeadPage;
    while (page != nullptr)
    {
      if ((void*)aPos.myElement >= (void*)page->myElements && (void*)aPos.myElement <= (void*)(page->myElements + page->myNextFreeElementIdx))
        return page;

      page = page->myNext;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
}

