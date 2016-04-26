#pragma once

#include <GL/glew.h>  // DEBUG!!!
#include "RendererPrerequisites.h"
#include "GpuProgramResource.h"

#if defined (RENDERER_DX12)

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
	class Adapter
	{
		public:
	//---------------------------------------------------------------------------//
		//static GLenum mapResourceTypeToGLbindingTarget(const GpuResourceType& generalType);

		static D3D12_BLEND_OP toNativeType(const BlendOp& generalType);
		static D3D12_BLEND toNativeType(const BlendInput& generalType);
		static D3D12_COMPARISON_FUNC toNativeType(const CompFunc& generalType);
		static D3D12_STENCIL_OP toNativeType(const StencilOp& generalType);
		static D3D12_FILL_MODE toNativeType(const FillMode& generalType);
		static D3D12_CULL_MODE toNativeType(const CullMode& generalType);
		//static Glenum toNativeType(const WindingOrder& generalType);
		//static GLenum toNativeType(const ShaderStage& generalType);
		//static GLuint toNativeFlag(const ShaderStageFlag& generalType);
		//static void mapGLpixelFormats(const DataFormat& generalPixelFormat, bool isDepthStencilFormat,
		//	GLenum& eFormat, GLenum& eInternalFormat, GLenum& ePixelType);
		//static void getGLtypeAndNumComponentsFromFormat(const DataFormat& eFormat,
		//	uint32& ruComponents, GLenum& reTypeGL);
		//---------------------------------------------------------------------------//
	};
}}} // end of namespace Fancy::Rendering::GL4
//---------------------------------------------------------------------------//
#endif // RENDERER_GL4
