#include "Log.hpp"

//#define NDEBUG
#define LOGGING
#define LOGGING_LEVEL 0

#define VE_VERSION_MAJOR 0
#define VE_VERSION_MINOR 1

#define VE_VERSION (VE_VERSION_MAJOR * 100 + VE_VERSION_MINOR)

#define VE_ASSERT(condition, message) \
    if (!(condition))              \
    {                              \
		LOG_ERROR(message); \
    }

#define BIT(x) (1 << x)

// Macros for defining smart pointers
#define VE_REF(x) std::shared_ptr<x>
#define VE_MAKE_REF(x, ...) std::make_shared<x>(__VA_ARGS__)

#define VE_UPTR(x) std::unique_ptr<x>
#define VE_MAKE_UPTR(x, ...) std::make_unique<x>(__VA_ARGS__)