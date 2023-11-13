#include "token.h"


const char* token_repr(Token token) {
#define X(upper, lower, body, value) case Token_##upper: return body;
    switch (token) {
        ALL_TOKENS(X)
    }
#undef X
}


TokenGroup token_group(Token token) {
#define X(upper, lower, body, value) case Token_##upper: return TokenGroup_##value;
    switch (token) {
        ALL_TOKENS(X)
    }
#undef X
}

