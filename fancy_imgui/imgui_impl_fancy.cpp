#include "imgui_impl_fancy.h"
#include "imgui.h"

#include <functional>

#include <fancy_core/GpuProgramPipeline.h>
#include <fancy_core/RenderCore.h>
#include <fancy_core/Window.h>
#include <fancy_core/RenderOutput.h>
#include <fancy_core/Texture.h>
#include <fancy_core/BlendState.h>
#include <fancy_core/DepthStencilState.h>
#include <fancy_core/CommandContext.h>
#include <fancy_core/GpuProgramDesc.h>
#include <fancy_core/GpuProgramPipelineDesc.h>
#include <fancy_core/Slot.h>
#include <fancy_core/Log.h>
#include <fancy_core/WindowsIncludes.h>
#include <fancy_core/CommandQueue.h>

namespace Fancy { namespace ImGuiRendering {
//---------------------------------------------------------------------------//
  FancyRuntime* ourFancyRuntime = nullptr;
  RenderOutput* ourRenderOutput = nullptr;

  struct CBufferData
  {
    glm::float4x4 myProjectionMatrix;
  };

  SharedPtr<TextureView> ourFontTexture;
  SharedPtr<GpuProgramPipeline> ourProgramPipeline;
  SharedPtr<BlendState> ourBlendState;
  SharedPtr<DepthStencilState> ourDepthStencilState;
      
  HWND ourHwnd = nullptr;
  INT64 ourTicksPerSecond = 0;
  INT64 ourTime = 0;

  void HandleWindowEvent(UINT msg, WPARAM wParam, LPARAM lParam, bool* aWasHandled)
  {
    ImGuiIO& io = ::ImGui::GetIO();
    switch (msg)
    {
    case WM_LBUTTONDOWN:
      io.MouseDown[0] = true;
      (*aWasHandled) = true;
      return;
    case WM_LBUTTONUP:
      io.MouseDown[0] = false;
      (*aWasHandled) = true;
      return;
    case WM_RBUTTONDOWN:
      io.MouseDown[1] = true;
      (*aWasHandled) = true;
      return;
    case WM_RBUTTONUP:
      io.MouseDown[1] = false;
      (*aWasHandled) = true;
      return;
    case WM_MOUSEWHEEL:
      io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
      (*aWasHandled) = true;
      return;
    case WM_MOUSEMOVE:
      io.MousePos.x = (signed short)(lParam);
      io.MousePos.y = (signed short)(lParam >> 16);
      (*aWasHandled) = true;
      return;
    case WM_KEYDOWN:
      if (wParam < 256)
        io.KeysDown[wParam] = 1;
      (*aWasHandled) = true;
      return;
    case WM_KEYUP:
      if (wParam < 256)
        io.KeysDown[wParam] = 0;
      (*aWasHandled) = true;
      return;
    case WM_CHAR:
      // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
      if (wParam > 0 && wParam < 0x10000)
        io.AddInputCharacter((unsigned short)wParam);
      (*aWasHandled) = true;
      return;
    }

    (*aWasHandled) = false;
  }

  bool Init(Fancy::RenderOutput* aRenderOutput, Fancy::FancyRuntime* aRuntime)
  {
    ourRenderOutput = aRenderOutput;
    std::function<void(UINT, WPARAM, LPARAM, bool*)> fnWindowHandler = &HandleWindowEvent;
    ourRenderOutput->GetWindow()->myWindowEventHandler.Connect(fnWindowHandler);

    ourFancyRuntime = aRuntime;
    ourHwnd = ourRenderOutput->GetWindow()->GetWindowHandle();

    ImGuiIO& io = ::ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;                              // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';
  
    io.RenderDrawListsFn = RenderDrawLists;
    io.ImeWindowHandle = ourHwnd;
  
    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&ourTicksPerSecond))
      return false;
    if (!QueryPerformanceCounter((LARGE_INTEGER *)&ourTime))
      return false;

    // Load the imgui-shader state
    {
      GpuProgramPipelineDesc pipelineDesc;
      GpuProgramDesc* shaderDesc = &pipelineDesc.myGpuPrograms[(uint)ShaderStage::VERTEX];
      shaderDesc->myShaderFileName = "Imgui";
      shaderDesc->myMainFunction = "main";
      shaderDesc->myShaderStage = (uint)ShaderStage::VERTEX;
      shaderDesc = &pipelineDesc.myGpuPrograms[(uint)ShaderStage::FRAGMENT];
      shaderDesc->myShaderFileName = "Imgui";
      shaderDesc->myShaderStage = (uint)ShaderStage::FRAGMENT;
      shaderDesc->myMainFunction = "main";
      ourProgramPipeline = RenderCore::CreateGpuProgramPipeline(pipelineDesc);
      ASSERT(ourProgramPipeline != nullptr);
    }
   
    // Create the font texture
    {
      uint8* fontPixelData = nullptr;
      int width, height, pixelSizeBytes;
      ImGuiIO& io = ::ImGui::GetIO();
      io.Fonts->GetTexDataAsRGBA32(&fontPixelData, &width, &height, &pixelSizeBytes);
      ASSERT(fontPixelData != nullptr);

      TextureSubData uploadData;
      uploadData.myData = fontPixelData;
      uploadData.myPixelSizeBytes = pixelSizeBytes;
      uploadData.myRowSizeBytes = width * uploadData.myPixelSizeBytes;
      uploadData.mySliceSizeBytes = height * uploadData.myRowSizeBytes;
      uploadData.myTotalSizeBytes = uploadData.mySliceSizeBytes;

      TextureProperties props;
      props.myDimension = GpuResourceDimension::TEXTURE_2D;
      props.myWidth = width;
      props.myHeight = height;
      props.eFormat = DataFormat::RGBA_8;
      const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(props.eFormat);
      ASSERT(formatInfo.mySizeBytes == pixelSizeBytes);

      TextureViewProperties viewProps;
      viewProps.myDimension = GpuResourceDimension::TEXTURE_2D;
      viewProps.myFormat = DataFormat::RGBA_8;
      
      ourFontTexture = RenderCore::CreateTextureView(props, viewProps, "Imgui Font Texture", &uploadData, 1u);
      ASSERT(ourFontTexture != nullptr);
    }

    // Blend state (alpha blending)
    {
      BlendStateDesc desc;
      desc.myBlendEnabled[0] = true;
      desc.mySrcBlend[0] = static_cast<uint>(BlendInput::SRC_ALPHA);
      desc.myDestBlend[0] = static_cast<uint>(BlendInput::INV_SRC_ALPHA);
      desc.myBlendOp[0] = static_cast<uint>(BlendOp::ADD);
      ourBlendState = RenderCore::CreateBlendState(desc);
      ASSERT(ourBlendState != nullptr);
    }

    // Depth-stencil state (no depth testing)
    {
      DepthStencilStateDesc desc;
      desc.myStencilEnabled = false;
      desc.myDepthTestEnabled = false;
      desc.myDepthWriteEnabled = false;
      ourDepthStencilState = RenderCore::CreateDepthStencilState(desc);
      ASSERT(ourDepthStencilState != nullptr);
    }
    
    return true;
  }
//---------------------------------------------------------------------------//
  void NewFrame()
  {
    ImGuiIO& io = ::ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    RECT rect;
    GetClientRect(ourHwnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    // Setup time step
    INT64 current_time;
    QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
    io.DeltaTime = (float)(current_time - ourTime) / ourTicksPerSecond;
    ourTime = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    // io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
    // io.MousePos : filled by WM_MOUSEMOVE events
    // io.MouseDown : filled by WM_*BUTTON* events
    // io.MouseWheel : filled by WM_MOUSEWHEEL events

    // Hide OS mouse cursor if ImGui is drawing it
    SetCursor(io.MouseDrawCursor ? NULL : LoadCursor(NULL, IDC_ARROW));

    // Start the frame
    ::ImGui::NewFrame();
  }
//---------------------------------------------------------------------------//
  void RenderDrawLists(ImDrawData* _draw_data)
  {
    CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);

    RenderOutput* renderOutput = ourRenderOutput;

    ctx->SetViewport(glm::uvec4(0, 0, ::ImGui::GetIO().DisplaySize.x, ::ImGui::GetIO().DisplaySize.y));
    ctx->SetRenderTarget(renderOutput->GetBackbufferRtv(), renderOutput->GetDepthStencilDsv());
    ctx->SetDepthStencilState(ourDepthStencilState);
    ctx->SetBlendState(ourBlendState);
    ctx->SetCullMode(CullMode::NONE);
    ctx->SetFillMode(FillMode::SOLID);
    ctx->SetWindingOrder(WindingOrder::CCW);
    ctx->SetTopologyType(TopologyType::TRIANGLE_LIST);
    ctx->SetGpuProgramPipeline(ourProgramPipeline);

    // Update the cbuffer data
    {
      float translate = -0.5f * 2.f;
      const float L = 0.f;
      const float R = ::ImGui::GetIO().DisplaySize.x;
      const float B = ::ImGui::GetIO().DisplaySize.y;
      const float T = 0.f;

      CBufferData cbuffer = {};
      cbuffer.myProjectionMatrix = glm::float4x4(
        2.0f / (R - L),     0.0f,               0.0f,       0.0f,
        0.0f,               2.0f / (T - B),     0.0f,       0.0f,
        0.0f,               0.0f,               0.5f,       0.0f,
        (R + L) / (L - R),  (T + B) / (B - T),  0.5f,       1.0f);
      
      ctx->BindConstantBuffer(&cbuffer, sizeof(cbuffer), 0u);
    }

    uint cmdListVertexOffset = 0u;
    uint cmdListIndexOffset = 0u;
    for (uint iCmdList = 0u; iCmdList < static_cast<uint>(_draw_data->CmdListsCount); ++iCmdList)
    {
      const ImDrawList* cmd_list = _draw_data->CmdLists[iCmdList];

      ctx->BindVertexBuffer(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), sizeof(ImDrawVert));
      ctx->BindIndexBuffer(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), sizeof(ImDrawIdx));

      uint cmdIndexOffset = 0;
      for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
      {
        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
        if (pcmd->UserCallback)
        {
          pcmd->UserCallback(cmd_list, pcmd);
        }
        else
        {
          const GpuResourceView* textureViews[] = { ourFontTexture.get() };

          ImTextureID textureId = pcmd->TextureId;
          if (textureId != nullptr)
            textureViews[0] = static_cast<const GpuResourceView*>(textureId);

          ctx->BindResourceSet(textureViews, 1u, 1u);

          const glm::uvec4 clipRect( (uint) pcmd->ClipRect.x, (uint) pcmd->ClipRect.y, (uint) pcmd->ClipRect.z, (uint) pcmd->ClipRect.w);
          ctx->SetClipRect(clipRect);

          ctx->Render(pcmd->ElemCount, 1u, cmdIndexOffset, 0u, 0u);
        }
        cmdIndexOffset += pcmd->ElemCount;
      }
    }

    CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
    queue->ExecuteContext(ctx);
    
    RenderCore::FreeContext(ctx);
  }

  void Shutdown()
  {
    ourFancyRuntime = nullptr;
    ourRenderOutput = nullptr;
    
    ourFontTexture.reset();
    ourProgramPipeline.reset();
    ourBlendState.reset();
    ourDepthStencilState.reset();
    
    ourHwnd = nullptr;
    ourTicksPerSecond = 0;
    ourTime = 0;
  }
//---------------------------------------------------------------------------//
} }
