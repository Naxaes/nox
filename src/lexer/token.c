#include "token.h"


const char* token_repr(Token token) {
#define X(upper, lower, body, group) case Token_##upper: return body;
    switch (token) {
        ALL_TOKENS(X)
    }
#undef X
}


TokenGroup token_group(Token token) {
#define X(upper, lower, body, group) case Token_##upper: return group;
    switch (token) {
        ALL_TOKENS(X)
    }
#undef X
}

