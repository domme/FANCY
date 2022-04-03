#ifndef INC_ENCODING
#define INC_ENCODING

uint Encode_Unorm(float aVal01, uint aNumBits, uint aShift)
{
  const uint maxVal = (1u << aNumBits) - 1u;
  return uint(saturate(aVal01) * float(maxVal)) << aShift;
}

uint Encode_Unorm_RGBA(float4 aVec01)
{
  uint enc;
  enc  = Encode_Unorm(aVec01.x, 8, 0);
  enc |= Encode_Unorm(aVec01.y, 8, 8);
  enc |= Encode_Unorm(aVec01.z, 8, 16);
  enc |= Encode_Unorm(aVec01.w, 8, 24);
  return enc;
}

float Decode_Unorm(uint enc, uint aNumBits, uint aShift)
{
	const uint maxVal = ((1u << aNumBits) - 1u);
	float val = (float) ((enc >> aShift) & maxVal);
	return val / (float) maxVal;
}

float4 Decode_Unorm_RGBA(uint enc)
{
	float4 val;
	val.x = Decode_Unorm(enc, 8, 0);
	val.y = Decode_Unorm(enc, 8, 8);
	val.z = Decode_Unorm(enc, 8, 16);
	val.w = Decode_Unorm(enc, 8, 24);
	return val;
}

#endif  // INC_ENCODING