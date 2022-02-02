#include "fancy_core_precompile.h"

namespace EA
{
	namespace StdC
	{
		EASTL_EASTDC_API int Vsnprintf(char* EA_RESTRICT pDestination, size_t n, const char* EA_RESTRICT pFormat, va_list arguments)
		{
			return vsnprintf(pDestination, n, pFormat, arguments);
		}

		EASTL_EASTDC_API int Vsnprintf(char16_t* EA_RESTRICT pDestination, size_t n, const char16_t* EA_RESTRICT pFormat, va_list arguments)
		{
			static_assert(sizeof(char16_t) == sizeof(wchar_t), "Unexpected wchar_t size");
			return vswprintf( (wchar_t*) pDestination, n, (wchar_t*) pFormat, arguments);
		}

#if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
		EASTL_EASTDC_API int Vsnprintf(wchar_t* EA_RESTRICT pDestination, size_t n, const wchar_t* EA_RESTRICT pFormat, va_list arguments)
		{
			return vswprintf(pDestination, n, pFormat, arguments);
		}
#endif

		// Does Windows have builtin support for printf-functions on 32bit chars?
		// EASTL_EASTDC_API int Vsnprintf(char32_t* EA_RESTRICT pDestination, size_t n, const char32_t* EA_RESTRICT pFormat, va_list arguments);
	}
}
