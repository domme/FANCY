#include "RenderQueues.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  RenderQueue::RenderQueue()
  //  : myAllocator(128u)
    : myNumItems(0u)
  {
  }
//---------------------------------------------------------------------------//
  RenderQueue::~RenderQueue()
  {
  }
//---------------------------------------------------------------------------//
  RenderQueueItem* RenderQueue::AddItem()
  {
    ASSERT(myNumItems < kMaxNumRenderQueueItems);

    // RenderQueueItem* item = myAllocator.Allocate();
    // myItems.push_back(item);
    // return item;
    
    RenderQueueItem* item = &myItems[myNumItems++];
    memset(item, 0u, sizeof(RenderQueueItem));
    return item;
  }
//---------------------------------------------------------------------------//
  void RenderQueue::Clear()
  {
    myNumItems = 0u;
    // myAllocator.FreeAll();
  }
//---------------------------------------------------------------------------// 
} }



