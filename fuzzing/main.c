#include "lexer/lexer.h"


int LLVMFuzzerTestOneInput(const u8* data, size_t size) {
    size = size >= 1024 ? 1023 : size;

    char source[1024] = { 0 };
    memcpy(source, data, size);
    source[size] = '\0';

    TokenArray token_array = lexer_lex(STR("<fuzzer>"), (Str) { size, source });

    if (token_array.tokens != NULL) {
        token_array_free(token_array);
    }

    return 0;  // Values other than 0 and -1 are reserved for future use.
}

