#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <cassert>

#define VE_VERSION_MAJOR 0
#define VE_VERSION_MINOR 1

#define VE_VERSION (VE_VERSION_MAJOR * 100 + VE_VERSION_MINOR)
#define VE_ASSERT(x) assert(x)

#define BIT(x) (1 << x)