#ifndef FANCY_CORE_PRECOMPILE_H
#define FANCY_CORE_PRECOMPILE_H

#include <algorithm>
#include <cfloat>
#include <malloc.h>
#include <utility>
#include <type_traits>
#include <codecvt>
#include <fstream>
#include <cassert>
#include <climits>
#include <chrono>
#include <cstdlib>
#include <mutex>

#include "Common/EASTLAllocator.h"
#include "EASTL/fixed_vector.h"
#include "EASTL/any.h"
#include "EASTL/fixed_hash_map.h"
#include "EASTL/fixed_hash_set.h"
#include "EASTL/fixed_list.h"
#include "EASTL/fixed_string.h"
#include "EASTL/fixed_substring.h"
#include "EASTL/hash_map.h"
#include "EASTL/hash_set.h"
#include "EASTL/functional.h"
#include "EASTL/optional.h"
#include "EASTL/unique_ptr.h"
#include "EASTL/shared_ptr.h"
#include "EASTL/safe_ptr.h"
#include "EASTL/string.h"
#include "EASTL/fixed_string.h"
#include "EASTL/chrono.h"
#include "EASTL/memory.h"
#include "EASTL/utility.h"
#include "EASTL/span.h"

#include "Common/FancyCoreDefines.h"
#include "Common/MathIncludes.h"
#include "Common/WindowsIncludes.h"
#include "Common/Ptr.h"
#include "Common/Slot.h"
#include "Common/Callback.h"
#include "Common/StringUtil.h"
#include "Common/MathUtil.h"
#include "Common/StaticString.h"
#include "Common/GrowingList.h"
#include "Common/PagedLinearAllocator.h"

#include "Rendering/DataFormat.h"
#include "Rendering/RenderEnums.h"
#include "Rendering/TextureData.h"
#include "Rendering/TempResources.h"
#include "Rendering/TextureProperties.h"
#include "Rendering/GpuBufferProperties.h"

#include "Debug/Log.h"

#endif