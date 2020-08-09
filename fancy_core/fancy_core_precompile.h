#ifndef FANCY_CORE_PRECOMPILE_H
#define FANCY_CORE_PRECOMPILE_H

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <array>
#include <list>
#include <iostream>
#include <algorithm>
#include <memory>
#include <functional>
#include <cfloat>
#include <cstdio>
#include <malloc.h>
#include <utility>
#include <type_traits>
#include <codecvt>
#include <fstream>
#include <unordered_map>
#include <cassert>
#include <istream>
#include <ostream>
#include <ios>
#include <climits>
#include <chrono>
#include <cstdlib>
#include <mutex>

#include "EASTLAllocator.h"
#include "EASTL/fixed_vector.h"
#include "EASTL/any.h"
#include "EASTL/fixed_hash_map.h"
#include "EASTL/fixed_hash_set.h"
#include "EASTL/fixed_list.h"
#include "EASTL/fixed_string.h"
#include "EASTL/fixed_substring.h"
#include "EASTL/functional.h"
#include "EASTL/optional.h"
#include "EASTL/unique_ptr.h"
#include "EASTL/shared_ptr.h"
#include "EASTL/safe_ptr.h"
#include "EASTL/string.h"
#include "EASTL/chrono.h"
#include "EASTL/memory.h"
#include "EASTL/utility.h"

#include "FancyCoreDefines.h"
#include "MathIncludes.h"
#include "WindowsIncludes.h"

#include "Log.h"
#include "DynamicArray.h"
#include "FixedArray.h"
#include "StaticArray.h"
#include "Ptr.h"
#include "Slot.h"
#include "Callback.h"
#include "Any.h"
#include "StringUtil.h"
#include "DX12Prerequisites.h"
#include "MathUtil.h"
#include "DataFormat.h"
#include "RenderEnums.h"
#include "StaticString.h"
#include "GrowingList.h"

#include "TextureData.h"
#include "TempResources.h"
#include "PagedLinearAllocator.h"
#include "TextureProperties.h"
#include "GpuBufferProperties.h"
#include "AdapterDX12.h"
#include "CommandAllocatorPoolDX12.h"
#include "DynamicDescriptorHeapDX12.h"

#endif