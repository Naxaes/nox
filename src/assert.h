#undef assert
#include <signal.h>

#define assert(expr) \
    do {                 \
        if (!(expr)) {   \
            fprintf(stderr, "[ASSERT] %s:%d: %s\n", __FILE__, __LINE__, #expr); \
            raise(SIGINT);         \
            abort(); \
        } \
    } while (0)
