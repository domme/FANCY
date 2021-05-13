#ifndef INC_ROOTSIGNATURE_H
#define INC_ROOTSIGNATURE_H

#define _TexSpace space0
#define _RWTexSpace space1
#define _BufSpace space2
#define _RWBufSpace space3
#define _SamplerSpace space4
#define _LocalSRVSpace space5
#define _LocalUAVSpace space6
#define _LocalSamplerSpace space7
#define _LocalCBufferSpace space8

#define RootSig_Default "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT),"\
						"DescriptorTable(SRV(t0, space = 0, numDescriptors = unbounded)),"\
						"DescriptorTable(SRV(t0, space = 5, numDescriptors = 16)),"\
						"DescriptorTable(Sampler(s0, space = 7, numDescriptors = 17)),"\
						"CBV(b0, space = 8),"\
						"CBV(b1, space = 8),"\
						"CBV(b2, space = 8),"\
						"CBV(b3, space = 8),"\
						"CBV(b4, space = 8),"\
						"CBV(b5, space = 8),"\
						"CBV(b6, space = 8),"\
						"CBV(b7, space = 8)"


						//"DescriptorTable(UAV(u0, space = 1, numDescriptors = unbounded)),"\
						//"DescriptorTable(SRV(t0, space = 2, numDescriptors = unbounded)),"\
						//"DescriptorTable(UAV(u0, space = 3, numDescriptors = unbounded)),"\
						//"DescriptorTable(Sampler(s0, space = 4, numDescriptors = unbounded)),"\

						//"DescriptorTable(UAV(u0, space = 6, numDescriptors = 16)),"\

#endif  // INC_ROOTSIGNATURE_H