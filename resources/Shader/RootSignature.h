#ifndef INC_ROOTSIGNATURE_H
#define INC_ROOTSIGNATURE_H

#define _TexSpace space0
#define _RWTexSpace space1
#define _BufSpace space2
#define _RWBufSpace space3
#define _SamplerSpace space4
#define _LocalTexSpace space5
#define _LocalRWTexSpace space6
#define _LocalBufSpace space7
#define _LocalRWBufSpace space8
#define _LocalSamplerSpace space9
#define _LocalCBufferSpace space10

#define RootSig_Default "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT),"\
						"DescriptorTable(SRV(t0, space = 0, numDescriptors = unbounded)),"\
						"DescriptorTable(UAV(u0, space = 1, numDescriptors = unbounded)),"\
						"DescriptorTable(SRV(t0, space = 2, numDescriptors = unbounded)),"\
						"DescriptorTable(UAV(u0, space = 3, numDescriptors = unbounded)),"\
						"DescriptorTable(Sampler(s0, space = 4, numDescriptors = unbounded)),"\
						"DescriptorTable(SRV(t0, space = 5, numDescriptors = 8)),"\
						"DescriptorTable(UAV(u0, space = 6, numDescriptors = 8)),"\
						"DescriptorTable(SRV(t0, space = 7, numDescriptors = 8)),"\
						"DescriptorTable(UAV(u0, space = 8, numDescriptors = 8)),"\
						"DescriptorTable(Sampler(s0, space = 9, numDescriptors = 8)),"\
						"CBV(b0, space = 10),"\
						"CBV(b1, space = 10),"\
						"CBV(b2, space = 10),"\
						"CBV(b3, space = 10),"\
						"CBV(b4, space = 10),"\
						"CBV(b5, space = 10),"\
						"CBV(b6, space = 10),"\
						"CBV(b7, space = 10)"

#endif  // INC_ROOTSIGNATURE_H