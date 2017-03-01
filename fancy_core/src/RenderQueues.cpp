#include "RenderQueues.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  RenderQueue::RenderQueue()
  //  : myAllocator(128u)
  {
  }
//---------------------------------------------------------------------------//
  RenderQueue::~RenderQueue()
  {
  }
//---------------------------------------------------------------------------//
  RenderQueueItem* RenderQueue::AddItem()
  {
    ASSERT(!myItems.IsFull());

    // RenderQueueItem* item = myAllocator.Allocate();
    // myItems.push_back(item);
    // return item;

    myItems.push_back(RenderQueueItem());
    return &myItems[myItems.size() - 1u];
  }
//---------------------------------------------------------------------------//
  void RenderQueue::Clear()
  {
    myItems.clear();
    // myAllocator.FreeAll();
  }
//---------------------------------------------------------------------------// 
} }



