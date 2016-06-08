#include "RendererPrerequisites.h"

#include "ComputeContextDX12.h"
#include "CommandListType.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
ComputeContextDX12::ComputeContextDX12()
  : CommandContext(CommandListType::Compute)
{
}

ComputeContextDX12::~ComputeContextDX12()
{
}

void ComputeContextDX12::SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const
{
}

void ComputeContextDX12::SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const
{
}

void ComputeContextDX12::SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const
{
}

void ComputeContextDX12::SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const
{
}

void ComputeContextDX12::SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aDescriptorTypeMask, uint32 aRegisterIndex)
{
}
//---------------------------------------------------------------------------//
} } }