#include "Test_GpuMemoryAllocator.h"

#include "Common/StaticString.h"
#include "Rendering/GpuBuffer.h"
#include "imgui.h"
#include "Rendering/RenderCore.h"

using namespace Fancy;

Test_GpuMemoryAllocator::Test_GpuMemoryAllocator( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow, Fancy::RenderOutput * aRenderOutput,
                                                  Fancy::InputState * anInputState )
    : Test( anAssetManager, aWindow, aRenderOutput, anInputState, "GPU Memory Allocations" ), myBufferToAllocSizeMb( 64 ) {}

Test_GpuMemoryAllocator::~Test_GpuMemoryAllocator() {
  RenderCore::WaitForIdle( CommandListType::Graphics );
}

void Test_GpuMemoryAllocator::OnUpdate( bool aDrawProperties ) {
  if ( !aDrawProperties )
    return;

  ImGui::InputInt( "Buffer Size (MiB)", &myBufferToAllocSizeMb );
  if ( ImGui::Button( "Allocate Gpu Buffer" ) ) {
    GpuBufferProperties props;
    props.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
    props.myNumElements = 1u;
    props.myIsShaderWritable = false;
    props.myElementSizeBytes = myBufferToAllocSizeMb * SIZE_MB;

    eastl::string name( StaticString< 64 >( "Gpu-buffer %d MiB", myBufferToAllocSizeMb ) );
    myBuffers.push_back( RenderCore::CreateBuffer( props, name.c_str() ) );
  }

  if ( !myBuffers.empty() && ImGui::Button( "Remove last GpuBuffer" ) ) {
    myBuffers.erase( myBuffers.begin() + myBuffers.size() - 1 );
  }

  if ( !myBuffers.empty() && ImGui::Button( "Remove all GpuBuffers" ) )
    myBuffers.clear();

  ImGui::Text( "%d Buffers allocated:", myBuffers.size() );

  for ( const Fancy::GpuBufferHandle & buffer : myBuffers ) {
    GpuBuffer * bufPtr = RenderCore::GetBuffer( buffer );
    ImGui::Text( "\t%s", bufPtr->myName.c_str() );
  }

  ImGui::NewLine();

  myGpuMemoryViewer.Render();
}

void Test_GpuMemoryAllocator::OnRender() {}
