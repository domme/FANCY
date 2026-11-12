#pragma once

#include <new>
#include <type_traits>

#include "FrameGraphBuilder.h"

/// Optional helpers for task classes with the usual layout: a nested RenderData struct (filled when the graph is compiled) and static
/// Compile() / Execute() functions. They generate the FgTaskCreateInfo and its type-erased functions.

namespace Fancy {
  //---------------------------------------------------------------------------//
  template < class T > void FgExecuteTask( CommandList * aCommandList, const void * aTask, const void * someRenderData ) {
    T::Execute( aCommandList, *static_cast< const T * >( aTask ), *static_cast< const typename T::RenderData * >( someRenderData ) );
  }
  //---------------------------------------------------------------------------//
  template < class T > void FgCompileTask( const void * aTask, const FrameGraphCompileContext & aContext, void * someRenderData ) {
    typename T::RenderData * renderData = new ( someRenderData ) typename T::RenderData();
    T::Compile( *static_cast< const T * >( aTask ), aContext, *renderData );
  }
  //---------------------------------------------------------------------------//
  template < class T > FgTaskCreateInfo FgMakeTaskCreateInfo( const char * aName, CommandListType aQueueType ) {
    static_assert( std::is_trivially_destructible< T >::value, "Tasks are never destructed" );
    static_assert( std::is_trivially_destructible< typename T::RenderData >::value, "RenderData is never destructed" );

    FgTaskCreateInfo createInfo;
    createInfo.myName = aName;
    createInfo.myExecuteFunction = &FgExecuteTask< T >;
    createInfo.myCompileFunction = &FgCompileTask< T >;
    createInfo.myQueueType = aQueueType;
    createInfo.myTaskSize = sizeof( T );
    createInfo.myTaskAlignment = alignof( T );
    createInfo.myRenderDataSize = sizeof( typename T::RenderData );
    createInfo.myRenderDataAlignment = alignof( typename T::RenderData );
    return createInfo;
  }
  //---------------------------------------------------------------------------//
  template < class T > uint FgAddTask( FrameGraphBuilder & aBuilder, const char * aName, CommandListType aQueueType, T ** aTaskOut ) {
    void * memory = nullptr;
    const uint taskIndex = aBuilder.AddTask( FgMakeTaskCreateInfo< T >( aName, aQueueType ), &memory );
    *aTaskOut = memory != nullptr ? new ( memory ) T() : nullptr;
    return taskIndex;
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy
