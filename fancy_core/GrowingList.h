#pragma once

#include <functional>
#include "Log.h"

namespace Fancy {

  /*
   * Represents a simple double-linked list as a potentially faster alternative to std::list. 
   * It will grow in pages of PageSize and won't re-use memory from removed elements and instead always appends new elements at the end of the pages - possibly allocating a new page.
   * Pages are only removed if all of its elements have been removed. This is to make it more likely that linked elements are close to each other in memory, increasing cache-efficiency.
   * Because of that, the GrowingList is intended for semi-ordered adds and removes. Completely randomized add/remove might make only one element used per page, increasing the memory-requirements to
   * PageSize * NumberOfListElements as a worst case-scenario.
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
    Iterator RemoveGetNext(Iterator aPos);
    Iterator RemoveGetPrev(Iterator aPos);
    Iterator Find(const T& aData);
    Iterator Find(std::function<bool(const T&)> aPredicate);
    Iterator FindAtIndex(uint anIndex);
    Iterator ReverseFind(const T& aData);
    Iterator ReverseFind(std::function<bool(const T&)> aPredicate);
    bool IsEmpty() const;
    uint Size() const { return mySize; }
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
      uint myNumUsedElements = 0u;
      Page* myNext = nullptr;
    };

    Element* AllocateElement();
    Page* FindPage(Iterator aPos);
    Iterator RemoveInternal(Iterator aPos, bool aReverse);

    Page* myHeadPage = nullptr;
    Page* myTailPage = nullptr;
    Element* myHeadElement = nullptr;
    Element* myTailElement = nullptr;
    uint mySize = 0u;
    uint myNumPages = 0u;
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  GrowingList<T, PageSize>::Page::Page()
  {
    myElements = static_cast<Element*>(malloc(sizeof(Element) * PageSize));  // Malloc: don't call constructor for "Element", which might have a non-trivial constructor due to T
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  GrowingList<T, PageSize>::Page::~Page()
  {
    free(myElements);
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
    
    ++mySize;
    return Iterator(newElement);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::AddBefore(Iterator aPos, T aData)
  {
    ASSERT(aPos);
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

    ++mySize;
    return Iterator(newElement);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::AddAfter(Iterator aPos, T aData)
  {
    ASSERT(aPos);
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

    ++mySize;
    return Iterator(newElement);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::RemoveGetNext(Iterator aPos)
  {
    return RemoveInternal(aPos, false);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::RemoveGetPrev(Iterator aPos)
  {
    return RemoveInternal(aPos, true);
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::RemoveInternal(Iterator aPos, bool aReverse)
  {
    ASSERT(aPos);
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

    // In the special case of element being the last element allocated from the last page, we can free it again.
    //if (page == myTailPage && page->myNextFreeElementIdx > 0 && element == &page->myElements[page->myNextFreeElementIdx - 1])
    //  --page->myNextFreeElementIdx;

    ASSERT(page->myNumUsedElements > 0);
    --page->myNumUsedElements;

    if (myHeadPage != myTailPage)
    {
      // If there's more than one page, check if any page is fully allocated but not used anymore and delete it

      Page* lastPage = nullptr;
      Page* currPage = myHeadPage;
      while (currPage != nullptr)
      {
        Page* nextPage = currPage->myNext;

        if (currPage->myNextFreeElementIdx == PageSize && currPage->myNumUsedElements == 0)
        {
          if (lastPage == nullptr)
          {
            ASSERT(currPage == myHeadPage);
            ASSERT(nextPage != nullptr);
            myHeadPage = nextPage;
          }
          else if (nextPage == nullptr)
          {
            ASSERT(currPage == myTailPage);
            ASSERT(lastPage != nullptr);
            myTailPage = lastPage;
            myTailPage->myNext = nullptr;
          }
          else
          {
            ASSERT(lastPage != nullptr);
            lastPage->myNext = nextPage;
          }
          delete currPage;
          --myNumPages;
        }

        lastPage = currPage;
        currPage = nextPage;
      }
    }

    // Destruct the element
    element->myPrev = nullptr;
    element->myNext = nullptr;
    element->myData.~T();
    --mySize;
    return aReverse ? Iterator(prev) : Iterator(next);
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
  typename GrowingList<T, PageSize>::Iterator GrowingList<T, PageSize>::FindAtIndex(uint anIndex)
  {
    ASSERT(anIndex < mySize);
    Iterator it(myHeadElement);
    while (anIndex != 0)
    {
      ++it;
      --anIndex;
    }

    return it;
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
  }
//---------------------------------------------------------------------------//
  template <class T, uint64 PageSize>
  typename GrowingList<T, PageSize>::Element* GrowingList<T, PageSize>::AllocateElement()
  {
    if (myTailPage == nullptr || myTailPage->myNextFreeElementIdx == PageSize) // A new page is needed
    {
      Page* newPage = new Page;
      ++myNumPages;
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

    ++myTailPage->myNumUsedElements;
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
}

