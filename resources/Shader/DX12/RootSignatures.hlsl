#if !defined(INCLUDE_ROOTSIGNATURES_INCLUDE_)
#define INCLUDE_ROOTSIGNATURES_INCLUDE_

#define RS_FORWARD_COLORPASS \
  "RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT )," \
  "CBV(b0), " \
  "CBV(b1), " \
  "CBV(b3), " \
  "CBV(b4), " \
  "CBV(b5), " \
  "DescriptorTable(SRV(t0, numDescriptors = 3), visibility = SHADER_VISIBILITY_PIXEL),
  "StaticSampler(s0, " \
    "addressU = TEXTURE_ADDRESS_CLAMP, " \
    "addressV = TEXTURE_ADDRESS_CLAMP, " \
    "filter = FILTER_MIN_MAG_MIP_LINEAR )"

#endif  // INCLUDE_ROOTSIGNATURES_INCLUDE_