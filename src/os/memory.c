#if defined(_WIN32)
#include "win32_memory.c"
#elif defined(Unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__) && defined(__MACH__)
#include "posix_memory.c"
#else
#error "Unsupported platform"
#endif
