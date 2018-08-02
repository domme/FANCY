#ifndef DECLARE_DATA_FORMAT
  #define DECLARE_DATA_FORMAT(...)
#endif

//                   Format,      ByteSize,      NumComponents,   DepthStencil,   SRGB,    Compressed
DECLARE_DATA_FORMAT(NONE,             0,               0,            false,       false,      false )
DECLARE_DATA_FORMAT(SRGB_8_A_8,       4,               4,            false,       true,       false )
DECLARE_DATA_FORMAT(RGBA_8,           4,               4,            false,       false,      false )
DECLARE_DATA_FORMAT(SRGB_8,           3,               3,            false,       true,       false )
DECLARE_DATA_FORMAT(RGB_8,            3,               3,            false,       false,      false )
DECLARE_DATA_FORMAT(RGB_11_11_10F,    4,               3,            false,       false,      false )
DECLARE_DATA_FORMAT(RGBA_16F,         8,               4,            false,       false,      false )
DECLARE_DATA_FORMAT(RGB_16F,          6,               3,            false,       false,      false )
DECLARE_DATA_FORMAT(RG_16F,           4,               2,            false,       false,      false )
DECLARE_DATA_FORMAT(R_16F,            2,               1,            false,       false,      false )
DECLARE_DATA_FORMAT(RGBA_32F,        16,               4,            false,       false,      false )
DECLARE_DATA_FORMAT(RGB_32F,         12,               3,            false,       false,      false )
DECLARE_DATA_FORMAT(RG_32F,           8,               2,            false,       false,      false )
DECLARE_DATA_FORMAT(R_32F,            4,               1,            false,       false,      false )
DECLARE_DATA_FORMAT(RGBA_32UI,       16,               4,            false,       false,      false )
DECLARE_DATA_FORMAT(RGB_32UI,        12,               3,            false,       false,      false )
DECLARE_DATA_FORMAT(RG_32UI,          8,               2,            false,       false,      false )
DECLARE_DATA_FORMAT(R_32UI,           4,               1,            false,       false,      false )
DECLARE_DATA_FORMAT(RGBA_16UI,        8,               4,            false,       false,      false )
DECLARE_DATA_FORMAT(RGB_16UI,         6,               3,            false,       false,      false )
DECLARE_DATA_FORMAT(RG_16UI,          4,               2,            false,       false,      false )
DECLARE_DATA_FORMAT(R_16UI,           2,               1,            false,       false,      false )
DECLARE_DATA_FORMAT(RGBA_8UI,         4,               4,            false,       false,      false )
DECLARE_DATA_FORMAT(RGB_8UI,          3,               3,            false,       false,      false )
DECLARE_DATA_FORMAT(RG_8UI,           2,               2,            false,       false,      false )
DECLARE_DATA_FORMAT(R_8UI,            1,               1,            false,       false,      false )
DECLARE_DATA_FORMAT(RGBA_32I,        16,               4,            false,       false,      false )
DECLARE_DATA_FORMAT(RGB_32I,         12,               3,            false,       false,      false )
DECLARE_DATA_FORMAT(RG_32I,           8,               2,            false,       false,      false )
DECLARE_DATA_FORMAT(R_32I,            4,               1,            false,       false,      false )
DECLARE_DATA_FORMAT(RGBA_16I,         8,               4,            false,       false,      false )
DECLARE_DATA_FORMAT(RGB_16I,          6,               3,            false,       false,      false )
DECLARE_DATA_FORMAT(RG_16I,           4,               2,            false,       false,      false )
DECLARE_DATA_FORMAT(R_16I,            2,               1,            false,       false,      false )
DECLARE_DATA_FORMAT(RGBA_8I,          4,               4,            false,       false,      false )
DECLARE_DATA_FORMAT(RGB_8I,           3,               3,            false,       false,      false )
DECLARE_DATA_FORMAT(RG_8I,            2,               2,            false,       false,      false )
DECLARE_DATA_FORMAT(R_8I,             1,               1,            false,       false,      false )
DECLARE_DATA_FORMAT(D_24UNORM_S_8UI,  4,               2,            true,        false,      false )