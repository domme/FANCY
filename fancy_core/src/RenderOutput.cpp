#include "RenderOutput.h"
#include "RenderWindow.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  RenderOutput::RenderOutput(void* aNativeInstanceHandle)
    : myCurrBackbufferIndex(0u)
  {
    Fancy::WindowParameters params;
    params.myTitle = "Fancy Engine";
    params.myWidth = 1280u;
    params.myHeight = 720u;

    HINSTANCE instanceHandle = static_cast<HINSTANCE>(aNativeInstanceHandle);

    myWindow = RenderWindow::Create(instanceHandle, params);
    myWindow->myOnResize.Connect(this, &RenderOutput::OnWindowResized);
  }
//---------------------------------------------------------------------------//
  RenderOutput::~RenderOutput()
  {
    myWindow->myOnResize.DetachObserver(this);
  }
//---------------------------------------------------------------------------//
  void RenderOutput::OnWindowResized(uint aWidth, uint aHeight)
  {
  }
//---------------------------------------------------------------------------//  
} }
