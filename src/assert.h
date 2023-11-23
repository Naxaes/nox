#undef assert
#include <signal.h>
#include <stdlib.h>

#define assert(expr) \
    do {                 \
        if (!(expr)) {   \
            fprintf(stderr, "[ASSERT] %s:%d: %s\n", __FILE__, __LINE__, #expr); \
            raise(SIGINT);         \
            exit(-1); \
        } \
    } while (0)
