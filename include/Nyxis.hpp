#include "Log.hpp"

#define NDEBUG

#ifndef NDEBUG

#define LOGGING
#define LOGGING_LEVEL 0

#endif
#define NYXIS_VERSION_MAJOR 0
#define NYXIS_VERSION_MINOR 1

#define NYXIS_VERSION (VE_VERSION_MAJOR * 100 + VE_VERSION_MINOR)

#define NYXIS_ASSERT(condition, message) \
    if (!(condition))              \
    {                              \
		LOG_ERROR(message); \
    }

#define BIT(x) (1 << x)

// Macros for defining smart pointers
#define REF(x) std::shared_ptr<x>
#define MAKE_REF(x, ...) std::make_shared<x>(__VA_ARGS__)

#define UPTR(x) std::unique_ptr<x>
#define MAKE_UPTR(x, ...) std::make_unique<x>(__VA_ARGS__)

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;